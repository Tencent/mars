// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * ComplexConnect.cpp
 *
 *  Created on: 2014-2-24
 *      Author: yerungui
 */

#include "complexconnect.h"

#include <algorithm>

#include "comm/xlogger/xlogger.h"
#include "comm/socket/socketselect.h"
#include "comm/socket/tcpclient_fsm.h"
#include "comm/socket/socket_address.h"
#include "comm/time_utils.h"
#include "comm/platform_comm.h"

#ifdef COMPLEX_CONNECT_NAMESPACE
namespace COMPLEX_CONNECT_NAMESPACE {
#endif
    
ComplexConnect::ComplexConnect(unsigned int _timeout, unsigned int _interval)
    : timeout_(_timeout), interval_(_interval), error_interval_(_interval), max_connect_(3), trycount_(0), index_(-1), errcode_(0)
    , index_conn_rtt_(0), index_conn_totalcost_(0), totalcost_(0)
{}

ComplexConnect::ComplexConnect(unsigned int _timeout /*ms*/, unsigned int _interval /*ms*/, unsigned int _error_interval /*ms*/, unsigned int _max_connect)
    : timeout_(_timeout), interval_(_interval), error_interval_(_error_interval), max_connect_(_max_connect),  trycount_(0), index_(-1), errcode_(0)
    , index_conn_rtt_(0), index_conn_totalcost_(0), totalcost_(0)
{}

ComplexConnect::~ComplexConnect()
{}

int ComplexConnect::__ConnectTime(unsigned int _index) const {
    return _index * interval_;
}

int ComplexConnect::__ConnectTimeout(unsigned int _index) const {
    return __ConnectTime(_index) + timeout_;
}

namespace {

class ConnectCheckFSM : public TcpClientFSM {
  public:
    enum TCheckStatus {
        ECheckInit,
        ECheckOK,
        ECheckFail,
    };

    ConnectCheckFSM(const socket_address& _addr, unsigned int _connect_timeout, unsigned int _index, MComplexConnect* _observer)
        : TcpClientFSM(_addr.address()), m_connect_timeout(_connect_timeout), m_index(_index), m_observer(_observer), m_checkfintime(0) {
        m_check_status = (m_observer && m_observer->OnShouldVerify(m_index, addr_)) ? ECheckInit : ECheckOK;
    }

    TCheckStatus CheckStatus() const { return m_check_status;}
    int TotalRtt() const { return int(m_checkfintime - start_connecttime_);}

  protected:
    virtual void _OnCreate() { if (m_observer) m_observer->OnCreated(m_index, addr_, sock_);}
    virtual void _OnConnect() { if (m_observer) m_observer->OnConnect(m_index, addr_, sock_);}
    virtual void _OnConnected(int _rtt) {
        m_checkfintime = ::gettickcount();

        if (!m_observer) return;

        m_observer->OnConnected(m_index, addr_, sock_, 0, _rtt);

        if (ECheckOK == CheckStatus()) {
            return;
        }

        if (!m_observer->OnVerifySend(m_index, addr_, sock_, send_buf_)) {
            m_check_status = ECheckFail;
        }
    }

    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) {
        if (!m_observer) return;

        if (ECheckOK == CheckStatus()) return;

        m_check_status = m_observer->OnVerifyRecv(m_index, addr_, sock_, recv_buf_) ? ECheckOK : ECheckFail;
        m_checkfintime = gettickcount();
    }

    virtual void _OnSend(AutoBuffer& _send_buff, ssize_t _send_len) {}

    virtual void _OnClose(TSocketStatus _status, int _error, bool _userclose) {
        m_checkfintime = gettickcount();

        if (m_observer && !_userclose) {
            if (EConnecting == _status) {
                m_observer->OnConnected(m_index, addr_, sock_, _error, TotalRtt());
            } else if (EReadWrite == _status && SOCKET_ERRNO(ETIMEDOUT) == _error) {
                m_checkfintime = gettickcount();
                m_observer->OnVerifyTimeout((int)(m_checkfintime - end_connecttime_));
            }
        }
    }

    virtual int ConnectTimeout() const {return (int)(start_connecttime_ + ConnectAbsTimeout() - gettickcount());}
    virtual int ReadWriteTimeout() const {return (int)(end_connecttime_ + ReadWriteAbsTimeout() - gettickcount());}

    virtual int ConnectAbsTimeout() const { return m_connect_timeout; }
    virtual int ReadWriteAbsTimeout() const { return std::max(1000, std::min(6 * Rtt(), ConnectAbsTimeout() - Rtt()));}

  protected:
    const unsigned int m_connect_timeout;
    const unsigned int m_index;
    MComplexConnect* m_observer;
    TCheckStatus m_check_status;
    uint64_t m_checkfintime;
};

static bool __isconnecting(const ConnectCheckFSM* _ref) { return NULL != _ref && INVALID_SOCKET != _ref->Socket(); }
}

SOCKET ComplexConnect::ConnectImpatient(const std::vector<socket_address>& _vecaddr, SocketSelectBreaker& _breaker, MComplexConnect* _observer) {
    trycount_ = 0;
    index_ = -1;
    errcode_ = 0;
    index_conn_rtt_ = 0;
    index_conn_totalcost_ = 0;
    totalcost_ = 0;

    if (_vecaddr.empty()) {
        xwarn2(TSF"_vecaddr size:%_, m_timeout:%_, m_interval:%_, m_error_interval:%_, m_max_connect:%_, @%_", _vecaddr.size(), timeout_, interval_, error_interval_, max_connect_, this);
        return INVALID_SOCKET;
    }

    xinfo2(TSF"_vecaddr size:%_, m_timeout:%_, m_interval:%_, m_error_interval:%_, m_max_connect:%_, @%_", _vecaddr.size(), timeout_, interval_, error_interval_, max_connect_, this);
    
    uint64_t  starttime = gettickcount();
    std::vector<ConnectCheckFSM*> vecsocketfsm;

    for (unsigned int i = 0; i < _vecaddr.size(); ++i) {
        xinfo2(TSF"complex.conn %_", _vecaddr[i].url());

        ConnectCheckFSM* ic = new ConnectCheckFSM(_vecaddr[i], timeout_, i, _observer);
        vecsocketfsm.push_back(ic);
    }

    uint64_t  curtime = gettickcount();
    uint64_t  laststart_connecttime = curtime - std::max(interval_, error_interval_);

    xdebug2(TSF"curtime:%_, laststart_connecttime:%_, @%_", curtime, laststart_connecttime, this);

    int lasterror = 0;
    unsigned int index = 0;
    SOCKET retsocket = INVALID_SOCKET;

    do {
        curtime = gettickcount();
        // timeout and connect
        SocketSelect sel(_breaker);
        sel.PreSelect();

        int next_connect_timeout = int(((0 == lasterror) ? interval_ : error_interval_) - (curtime - laststart_connecttime));

        int timeout = (int)timeout_;
        unsigned int runing_count = (unsigned int)std::count_if(vecsocketfsm.begin(), vecsocketfsm.end(), &__isconnecting);

        if (index < vecsocketfsm.size()
                && 0 < next_connect_timeout
                && runing_count < max_connect_) {
            timeout = std::min(timeout, next_connect_timeout);
        }

        // connect
        if (index < vecsocketfsm.size()
                && 0 >= next_connect_timeout
                && runing_count < max_connect_) {
            if (runing_count + 1 < max_connect_) timeout = std::min(timeout, (int)interval_);

            laststart_connecttime = gettickcount();
            lasterror = 0;

            trycount_ = (unsigned int)(index + 1);
            ++index;
        }

        for (unsigned int i = 0; i < index; ++i) {
            if (NULL == vecsocketfsm[i]) continue;

            xgroup2_define(group);
            vecsocketfsm[i]->PreSelect(sel, group);
            xgroup2_if(!group.Empty(), TSF"index:%_, @%_, ", i, this) << group;
            timeout = std::min(timeout, vecsocketfsm[i]->Timeout());
        }

        xdebug2(TSF"timeout:%_, @%_", timeout, this);
        int ret = 0;

        if (INT_MAX == timeout) {
            ret = sel.Select();
        } else {
            timeout = std::max(0, timeout);
            ret = sel.Select(timeout);
        }

        // select error
        if (ret < 0) {
            xerror2(TSF"sel ret:(%_, %_, %_), @%_", ret, sel.Errno(), socket_strerror(sel.Errno()), this);
            break;
        }

        // user break
        if (sel.IsException()) {
            xerror2(TSF"sel exception @%_", this);
            break;
        }

        if (sel.IsBreak()) {
            xinfo2(TSF"sel breaker @%_", this);
            break;
        }

        // socket
        for (unsigned int i = 0; i < index; ++i) {
            if (NULL == vecsocketfsm[i]) continue;

            xgroup2_define(group);
            vecsocketfsm[i]->AfterSelect(sel, group);
            xgroup2_if(!group.Empty(), TSF"index:%_, @%_, ", i, this) << group;

            if (TcpClientFSM::EEnd == vecsocketfsm[i]->Status()) {
                if (_observer) _observer->OnFinished(i, socket_address(&vecsocketfsm[i]->Address()), vecsocketfsm[i]->Socket(), vecsocketfsm[i]->Error(),
                                                         vecsocketfsm[i]->Rtt(), vecsocketfsm[i]->TotalRtt(), (int)(gettickcount() - starttime));

                vecsocketfsm[i]->Close();
                delete vecsocketfsm[i];
                vecsocketfsm[i] = NULL;
                lasterror = -1;
                continue;
            }

            if (TcpClientFSM::EReadWrite == vecsocketfsm[i]->Status() && ConnectCheckFSM::ECheckFail == vecsocketfsm[i]->CheckStatus()) {
                if (_observer) _observer->OnFinished(i, socket_address(&vecsocketfsm[i]->Address()), vecsocketfsm[i]->Socket(), vecsocketfsm[i]->Error(),
                                                         vecsocketfsm[i]->Rtt(), vecsocketfsm[i]->TotalRtt(), (int)(gettickcount() - starttime));

                vecsocketfsm[i]->Close();
                delete vecsocketfsm[i];
                vecsocketfsm[i] = NULL;
                lasterror = -1;
                continue;
            }

            if (TcpClientFSM::EReadWrite == vecsocketfsm[i]->Status() && ConnectCheckFSM::ECheckOK == vecsocketfsm[i]->CheckStatus()) {
                if (_observer) _observer->OnFinished(i, socket_address(&vecsocketfsm[i]->Address()), vecsocketfsm[i]->Socket(), vecsocketfsm[i]->Error(),
                                                         vecsocketfsm[i]->Rtt(), vecsocketfsm[i]->TotalRtt(), (int)(gettickcount() - starttime));

                xinfo2(TSF"index:%_, sock:%_, suc ConnectImpatient:%_:%_, RTT:(%_, %_), @%_", i, vecsocketfsm[i]->Socket(),
                       vecsocketfsm[i]->IP(), vecsocketfsm[i]->Port(), vecsocketfsm[i]->Rtt(), vecsocketfsm[i]->TotalRtt(), this);
                retsocket = vecsocketfsm[i]->Socket();
                index_ = i;
                index_conn_rtt_ = vecsocketfsm[i]->Rtt();
                index_conn_totalcost_ = vecsocketfsm[i]->TotalRtt();
                vecsocketfsm[i]->Socket(INVALID_SOCKET);
                delete vecsocketfsm[i];
                vecsocketfsm[i] = NULL;
                break;
            }
        }

        // end of loop
        bool all_invalid = true;

        for (unsigned int i = 0; i < vecsocketfsm.size(); ++i) {
            if (NULL != vecsocketfsm[i]) {
                all_invalid = false;
                break;
            }
        }

        if (all_invalid || INVALID_SOCKET != retsocket) break;
    } while (true);

    for (unsigned int i = 0; i < vecsocketfsm.size(); ++i) {
        if (NULL != vecsocketfsm[i]) {
            vecsocketfsm[i]->Close();
            delete vecsocketfsm[i];
            vecsocketfsm[i] = NULL;
        }
    }

    vecsocketfsm.clear();

    totalcost_ = (int)(::gettickcount() - starttime);
    xinfo2(TSF"retsocket:%_, connrtt:%_, conntotalrtt:%_, totalcost:%_, @%_", retsocket, index_conn_rtt_, index_conn_totalcost_, totalcost_, this);

    return retsocket;
}

#ifdef COMPLEX_CONNECT_NAMESPACE
}
#endif
