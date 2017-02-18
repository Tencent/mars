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

#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include <algorithm>

#include "comm/xlogger/xlogger.h"
#include "comm/socket/socketselect.h"
#include "comm/socket/tcpclient_fsm.h"
#include "comm/socket/socket_address.h"
#include "comm/http.h"
#include "comm/comm_data.h"
#include "comm/time_utils.h"
#include "comm/crypt/ibase64.h"
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
        
        EProxySelectSucc,
        EProxyConnectReqSend,
        EProxyConnectSucc,
        
        ECheckOK,
        ECheckFail,
    };

    ConnectCheckFSM(const socket_address& _addr, unsigned int _connect_timeout, unsigned int _index, MComplexConnect* _observer)
        : TcpClientFSM(_addr.address()), connect_timeout_(_connect_timeout), index_(_index), observer_(_observer), checkfintime_(0) {
        check_status_ = (_observer && _observer->OnShouldVerify(_index, addr_)) ? ECheckInit : ECheckOK;
    }

    TCheckStatus CheckStatus() const { return check_status_;}
    int TotalRtt() const { return int(checkfintime_ - start_connecttime_);}

  protected:
    virtual void _OnCreate() { if (observer_) observer_->OnCreated(index_, addr_, sock_);}
    virtual void _OnConnect() { if (observer_) observer_->OnConnect(index_, addr_, sock_);}
    virtual void _OnConnected(int _rtt) {
        checkfintime_ = ::gettickcount();

        if (!observer_) return;

        observer_->OnConnected(index_, addr_, sock_, 0, _rtt);

        if (ECheckOK == CheckStatus()) {
            return;
        }

        if (!observer_->OnVerifySend(index_, addr_, sock_, send_buf_)) {
            check_status_ = ECheckFail;
        }
    }

    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) {
        if (!observer_) return;

        if (ECheckOK == CheckStatus()) return;

        check_status_ = observer_->OnVerifyRecv(index_, addr_, sock_, recv_buf_) ? ECheckOK : ECheckFail;
        checkfintime_ = gettickcount();
    }

    virtual void _OnSend(AutoBuffer& _send_buff, ssize_t _send_len) {}

    virtual void _OnClose(TSocketStatus _status, int _error, bool _userclose) {
        checkfintime_ = gettickcount();

        if (observer_ && !_userclose) {
            if (EConnecting == _status) {
                observer_->OnConnected(index_, addr_, sock_, _error, TotalRtt());
            } else if (EReadWrite == _status && SOCKET_ERRNO(ETIMEDOUT) == _error) {
                checkfintime_ = gettickcount();
                observer_->OnVerifyTimeout((int)(checkfintime_ - end_connecttime_));
            }
        }
    }

    virtual int ConnectTimeout() const {return (int)(start_connecttime_ + ConnectAbsTimeout() - gettickcount());}
    virtual int ReadWriteTimeout() const {return (int)(end_connecttime_ + ReadWriteAbsTimeout() - gettickcount());}

    virtual int ConnectAbsTimeout() const { return connect_timeout_; }
    virtual int ReadWriteAbsTimeout() const { return std::max(1000, std::min(6 * Rtt(), ConnectAbsTimeout() - Rtt()));}

  protected:
    const unsigned int connect_timeout_;
    const unsigned int index_;
    MComplexConnect* observer_;
    TCheckStatus check_status_;
    uint64_t checkfintime_;
};
    
    
class ConnectHttpTunelCheckFSM : public ConnectCheckFSM {
public:
    ConnectHttpTunelCheckFSM(const socket_address& _destaddr, const socket_address* _proxy_addr, const std::string& _proxy_username,
                             const std::string& _proxy_pwd, unsigned int _connect_timeout, unsigned int _index, MComplexConnect* _observer)
    : ConnectCheckFSM(*_proxy_addr, _connect_timeout,_index,_observer), destaddr_(_destaddr), username_(_proxy_username), password_(_proxy_pwd){
        check_status_ = ECheckInit;
        xinfo2(TSF"proxy info:%_:%_ username:%_", _proxy_addr->ip(), _proxy_addr->port(), username_);
    }
    
protected:
    virtual void _OnConnected(int _rtt) {
        checkfintime_ = ::gettickcount();
        
        if (!observer_) return;
        
        observer_->OnConnected(index_, addr_, sock_, 0, _rtt);
        
        if (ECheckOK == CheckStatus()) {
            return;
        }
        request_send_ = true;
        check_status_ = EProxySelectSucc;
    }
    
    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) {
        if (!observer_) return;
        if (ECheckOK == CheckStatus()) return;
        if (check_status_ == EProxyConnectReqSend) {

            http::Parser parser;
            http::Parser::TRecvStatus parse_status = parser.Recv(_recv_buff.Ptr(), _recv_buff.Length());

            if (parse_status != http::Parser::kEnd) {
                xinfo2(TSF"proxy response continue:%_", _recv_buff.Length());
                return;
            }
            
            if (200 == parser.Status().StatusCode()) {
                check_status_ = (observer_ && observer_->OnShouldVerify(index_, addr_)) ? EProxyConnectSucc : ECheckOK;
                request_send_ = true;
                _recv_buff.Reset();
            } else {
                xwarn2(TSF"proxy status code:%_, proxy info:%_:%_", parser.Status().StatusCode(), addr_.ip(), addr_.port());
                check_status_ = ECheckFail;
            }
            
        } else if (check_status_ == EProxyConnectSucc) {
            check_status_ = observer_->OnVerifyRecv(index_, destaddr_, sock_, _recv_buff) ? ECheckOK : ECheckFail;
            checkfintime_ = gettickcount();
        } else {
            xassert2(false, "status:%d", check_status_);
        }
        
    }
    
    virtual void _OnRequestSend(AutoBuffer& _send_buff){
        if (check_status_ == EProxySelectSucc) {
            
            char ip_port[64] = {0};
            snprintf(ip_port, sizeof(ip_port), "%s:%" PRIu16, destaddr_.ip(), destaddr_.port());
            http::Builder req_builder(http::kRequest);
            req_builder.Request().Method(http::RequestLine::kConnect);
            req_builder.Request().Url(ip_port);
            req_builder.Request().Version(http::kVersion_1_1);
            req_builder.Fields().HeaderFiled(http::HeaderFields::KStringHost, ip_port);
            req_builder.Fields().HeaderFiled(http::HeaderFields::kStringProxyConnection, "keep-alive");
            req_builder.Fields().HeaderFiled(http::HeaderFields::KStringUserAgent, http::HeaderFields::KStringMicroMessenger);
            
            if (!username_.empty() && !password_.empty()) {
                std::string account_info = username_ + ":" + password_;
                size_t dstlen = modp_b64_encode_len(account_info.length());
                
                char* dstbuf = (char*)malloc(dstlen);
                memset(dstbuf, 0, dstlen);
                
                int retsize = Comm::EncodeBase64((unsigned char*)account_info.c_str(), (unsigned char*)dstbuf, (int)account_info.length());
                dstbuf[retsize] = '\0';
                
                char auth_info[1024] = {0};
                snprintf(auth_info, sizeof(auth_info), "Basic %s", dstbuf);

                req_builder.Fields().HeaderFiled(http::HeaderFields::kStringProxyAuthorization, auth_info);
            }
            
            req_builder.HeaderToBuffer(_send_buff);

            check_status_ = EProxyConnectReqSend;
        } else if (check_status_ == EProxyConnectSucc) {
            _send_buff.Reset();
            bool verifySucc = observer_->OnVerifySend(index_, destaddr_, sock_, _send_buff);
            if (!verifySucc) {
                check_status_ = ECheckFail;
            }
        } else {
            xassert2(false, "status:%d", check_status_);
        }
    }
    
protected:
    const socket_address& destaddr_;
    std::string username_;
    std::string password_;

};

static bool __isconnecting(const ConnectCheckFSM* _ref) { return NULL != _ref && INVALID_SOCKET != _ref->Socket(); }
}

SOCKET ComplexConnect::ConnectImpatient(const std::vector<socket_address>& _vecaddr, SocketSelectBreaker& _breaker, MComplexConnect* _observer,
                                            mars::comm::ProxyType _proxy_type, const socket_address* _proxy_addr,
                                            const std::string& _proxy_username, const std::string& _proxy_pwd) {
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

        ConnectCheckFSM* ic = NULL;
        if (mars::comm::kProxyHttp == _proxy_type) {
            ic = new ConnectHttpTunelCheckFSM(_vecaddr[i], _proxy_addr, _proxy_username, _proxy_pwd, timeout_, i, _observer);
        } else {
            ic = new ConnectCheckFSM(_vecaddr[i], timeout_, i, _observer);
        }

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
