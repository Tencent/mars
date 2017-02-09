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
 * longlink_speed_test.cc
 *
 *  Created on: 2013-5-13
 *      Author: yanguoyue
 */

#include "longlink_speed_test.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/platform_comm.h"
#include "mars/stn/stn.h"
#include "mars/stn/proto/longlink_packer.h"

using namespace mars::stn;

static const unsigned int kCmdIdOutOfBand = 72;
static const int kTimeout = 10*1000;  // s

LongLinkSpeedTestItem::LongLinkSpeedTestItem(const std::string& _ip, uint16_t _port)
    : ip_(_ip)
    , port_(_port)
    , socket_(-1)
    , state_(kLongLinkSpeedTestConnecting)
    , before_connect_time_(0)
    , after_connect_time_(0) {
        
    AutoBuffer body;
    AutoBuffer extension;
    longlink_noop_req_body(body, extension);

    longlink_pack(longlink_noop_cmdid(), Task::kNoopTaskID, body, extension, req_ab_, NULL);
    req_ab_.Seek(0, AutoBuffer::ESeekStart);

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_ == INVALID_SOCKET) {
        xerror2(TSF"socket create error, errno:%0", strerror(errno));
        return;
    }

    // set the socket to unblocked model
#ifdef _WIN32 
    if (0 != socket_ipv6only(socket_, 0)){ xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno)); }
#endif
        
    int ret = socket_set_nobio(socket_);

    if (ret != 0) {
        xerror2(TSF"nobio error");
        ::socket_close(socket_);
        socket_ = -1;
        socket_ = -1;
        return;
    }

    if (::getNetInfo() == kWifi && socket_fix_tcp_mss(socket_) < 0) {
#ifdef ANDROID
        xinfo2(TSF"wifi set tcp mss error:%0", strerror(socket_errno));
#endif
    }

    struct sockaddr_in _addr;

    bzero(&_addr, sizeof(_addr));
    _addr = *(struct sockaddr_in*)(&socket_address(ip_.c_str(), port_).address());

    before_connect_time_ = gettickcount();

    ::connect(socket_, (sockaddr*)&_addr, sizeof(_addr));
}

LongLinkSpeedTestItem::~LongLinkSpeedTestItem() {
    CloseSocket();
}

void LongLinkSpeedTestItem::HandleFDISSet(SocketSelect& _sel) {
    xverbose_function();

    if (kLongLinkSpeedTestFail == state_ || kLongLinkSpeedTestSuc == state_) {
        return;
    }

    if (_sel.Exception_FD_ISSET(socket_)) {
        xerror2(TSF"the socket is error, error:%0", strerror(errno));
        state_ = kLongLinkSpeedTestFail;
    } else if (_sel.Write_FD_ISSET(socket_)) {
        if (kLongLinkSpeedTestConnecting == state_) {
            after_connect_time_ = gettickcount();
        }

        state_ = __HandleSpeedTestReq();
    } else if (_sel.Read_FD_ISSET(socket_)) {
        state_ = __HandleSpeedTestResp();
    } else {
        // do nothing
    }
}

void LongLinkSpeedTestItem::HandleSetFD(SocketSelect& _sel) {
    switch (state_) {
    case kLongLinkSpeedTestConnecting:
    case kLongLinkSpeedTestOOB:
    case kLongLinkSpeedTestReq:
        _sel.Write_FD_SET(socket_);
        _sel.Read_FD_SET(socket_);
        _sel.Exception_FD_SET(socket_);
        break;

    case kLongLinkSpeedTestResp:
        _sel.Read_FD_SET(socket_);
        _sel.Exception_FD_SET(socket_);
        break;

    default:
        xassert2(false);
        break;
    }
}

int LongLinkSpeedTestItem::GetSocket() {
    return socket_;
}

std::string LongLinkSpeedTestItem::GetIP() {
    return ip_;
}

unsigned int LongLinkSpeedTestItem::GetPort() {
    return port_;
}

unsigned long LongLinkSpeedTestItem::GetConnectTime() {
    return after_connect_time_ - before_connect_time_;
}

int LongLinkSpeedTestItem::GetState() {
    return state_;
}

void LongLinkSpeedTestItem::CloseSocket() {
    if (socket_ > 0) {
        ::socket_close(socket_);
        socket_ = -1;
        socket_ = -1;
    }
}

int LongLinkSpeedTestItem::__HandleSpeedTestReq() {
    ssize_t nwrite =::send(socket_, req_ab_.PosPtr(), req_ab_.Length() - req_ab_.Pos(), 0);

    if (nwrite <= 0) {
        xerror2(TSF"writen send <= 0, errno:%0, nwrite:%1", strerror(errno), nwrite);
        return kLongLinkSpeedTestFail;
    } else {
        xdebug2(TSF"send length:%0", nwrite);
        req_ab_.Seek(nwrite, AutoBuffer::ESeekCur);

        if (req_ab_.Length() - req_ab_.Pos() <= 0) {
            return  kLongLinkSpeedTestResp;
        } else {
            return kLongLinkSpeedTestReq;
        }
    }
}

int LongLinkSpeedTestItem::__HandleSpeedTestResp() {
    if (resp_ab_.Capacity() - resp_ab_.Pos() <= 0) {
        resp_ab_.AddCapacity(resp_ab_.Capacity() == 0 ? 1024 : resp_ab_.Capacity());
    }

    ssize_t nrecv = recv(socket_, resp_ab_.PosPtr(), resp_ab_.Capacity() - resp_ab_.Pos(), 0);

    if (nrecv <= 0) {
        xerror2(TSF"recv nrecv <= 0, errno:%0, resp_ab_.Capacity():%1,resp_ab_.Pos():%2", strerror(errno), resp_ab_.Capacity(), resp_ab_.Pos());
        return kLongLinkSpeedTestFail;
    } else {
        xdebug2(TSF"recv length:%0", nrecv);
        resp_ab_.Length(nrecv + resp_ab_.Pos(), resp_ab_.Length() + nrecv);

        size_t pacLength = 0;
        uint32_t anSeq = 0;
        uint32_t anCmdID = 0;
        AutoBuffer body;
        AutoBuffer extension;
        
        int nRet  = longlink_unpack(resp_ab_, anCmdID, anSeq, pacLength, body, extension, NULL);

        if (LONGLINK_UNPACK_FALSE == nRet) {
            xerror2(TSF"longlink_unpack false");
            return kLongLinkSpeedTestFail;
        } else if (LONGLINK_UNPACK_CONTINUE == nRet) {
            xdebug2(TSF"not recv an package,continue recv, resp_ab_.Lenght():%0", resp_ab_.Length());
            return kLongLinkSpeedTestResp;
        } else if (kCmdIdOutOfBand == anCmdID) {
            uint32_t nType = ((uint32_t*)body.Ptr(16))[0];
            uint32_t nTime = ((uint32_t*)body.Ptr(16))[1];
            nType = ntohl(nType);
            nTime = ntohl(nTime);
            xwarn2(TSF"out of band,nType:%0, nTime:%1", nType, nTime);

            resp_ab_.Reset();
            return kLongLinkSpeedTestOOB;
        } else if (longlink_noop_isresp(Task::kNoopTaskID, anCmdID, anSeq, body, extension)) {
            return kLongLinkSpeedTestSuc;
        } else {
            xassert2(false);
            return kLongLinkSpeedTestFail;
        }
    }
}

////////////////////////////////////////////////////////////////

LongLinkSpeedTest::LongLinkSpeedTest(const boost::shared_ptr<NetSource>& _netsource): netsource_(_netsource)
    , selector_(breaker_) {
    if (!breaker_.IsCreateSuc()) {
        xassert2(false, "pipe error");
        return;
    }
}

LongLinkSpeedTest::~LongLinkSpeedTest() {
}

bool LongLinkSpeedTest::GetFastestSocket(int& _fdSocket, std::string& _strIp, unsigned int& _port, IPSourceType& _type, unsigned long& _connectMillSec) {
    xdebug_function();

    std::vector<IPPortItem> ipItemVec;

    if (!netsource_->GetLongLinkSpeedTestIPs(ipItemVec)) {
        xerror2(TSF"ipItemVec is empty");
        return false;
    }

    std::vector<LongLinkSpeedTestItem*> speedTestItemVec;

    for (std::vector<IPPortItem>::iterator iter = ipItemVec.begin(); iter != ipItemVec.end(); ++iter) {
        LongLinkSpeedTestItem* item = new LongLinkSpeedTestItem((*iter).str_ip, (*iter).port);
        speedTestItemVec.push_back(item);
    }

    int tryCount = 0;
    bool loopShouldBeStop = false;

    while (!loopShouldBeStop) {
        selector_.PreSelect();

        for (std::vector<LongLinkSpeedTestItem*>::iterator iter = speedTestItemVec.begin(); iter != speedTestItemVec.end(); ++iter) {
            (*iter)->HandleSetFD(selector_);
        }

        int selectRet = selector_.Select(kTimeout);

        if (selectRet == 0) {
            xerror2(TSF"time out");
            break;
        }

        if (selectRet < 0) {
            xerror2(TSF"select errror, ret:%0, strerror(errno):%1", selectRet, strerror(errno));

            if (EINTR == errno && tryCount < 3) {
                ++  tryCount;
                continue;
            } else {
                break;
            }
        }

        if (selector_.IsException()) {
            xerror2(TSF"pipe exception");
            break;
        }

        if (selector_.IsBreak()) {
            xwarn2(TSF"FD_ISSET(pipe_[0], &readfd)");
            break;
        }

        size_t count = 0;

        for (std::vector<LongLinkSpeedTestItem*>::iterator iter = speedTestItemVec.begin(); iter != speedTestItemVec.end(); ++iter) {
            (*iter)->HandleFDISSet(selector_);

            if (kLongLinkSpeedTestSuc == (*iter)->GetState()) {
                loopShouldBeStop = true;
                break;
            } else if (kLongLinkSpeedTestFail == (*iter)->GetState()) {
                ++count;
            } else {
                // do nothing
            }
        }

        if (count == speedTestItemVec.size()) {
            xwarn2(TSF"all speed tese fail");
            loopShouldBeStop = true;
        }
    }


    for (std::vector<LongLinkSpeedTestItem*>::iterator iter = speedTestItemVec.begin(); iter != speedTestItemVec.end(); ++iter) {
        for (std::vector<IPPortItem>::iterator ipItemIter = ipItemVec.begin(); ipItemIter != ipItemVec.end(); ++ipItemIter) {
            std::string ip = (*iter)->GetIP();

            if (ip != (*ipItemIter).str_ip || (*iter)->GetPort() != (*ipItemIter).port) {
                continue;
            }

            if (kLongLinkSpeedTestSuc == (*iter)->GetState()) {
                // (*ipItemIter).eState = ETestOK;
                _type = (*ipItemIter).source_type;
                _strIp = (*ipItemIter).str_ip;
                _port = (*iter)->GetPort();
            } else if (kLongLinkSpeedTestFail == (*iter)->GetState()) {
                // (*ipItemIter).eState = ETestFail;
            } else {
                // (*ipItemIter).eState = ETestNone;
            }

            break;
        }
    }

    // report the result of speed test
    netsource_->ReportLongLinkSpeedTestResult(ipItemVec);

    bool bRet = false;

    for (std::vector<LongLinkSpeedTestItem*>::iterator iter = speedTestItemVec.begin(); iter != speedTestItemVec.end(); ++iter) {
        if (kLongLinkSpeedTestSuc == (*iter)->GetState() && !bRet) {
            bRet = true;
            _fdSocket = (*iter)->GetSocket();
            _connectMillSec = (*iter)->GetConnectTime();
            xdebug2(TSF"speed test success, socket:%0, use time:%1", _fdSocket, _connectMillSec);
        } else {
            (*iter)->CloseSocket();
        }

        delete *iter;
    }

    speedTestItemVec.clear();

    return bRet;
}

boost::shared_ptr<NetSource> LongLinkSpeedTest::GetNetSource() {
    return netsource_;
}
