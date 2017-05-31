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
 * tcpquery.cc
 *
 *  Created on: 2014年6月30日
 *      Author: wutianqiang
 */

#include "tcpquery.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/socket/socketselect.h"

#include "sdt/src/tools/netchecker_socketutils.hpp"
#include "sdt/src/tools/netchecker_trafficmonitor.h"

using namespace mars::sdt;

TcpQuery::TcpQuery(const char* _ip, uint16_t _port, unsigned int _conn_timeout, NetCheckTrafficMonitor* _traffic_monitor)
    : ip_(strdup(_ip))
    , port_(_port)
    , select_(pipe_)
    , status_(kTcpInit)
    , errcode_(-1)
    , conn_timeout_(_conn_timeout) {
    if (!pipe_.IsCreateSuc()) {
        xassert2(false, "TcpQuery create breaker error.");
        status_ = kTcpInitErr;
    }

    sock_ = NetCheckerSocketUtils::makeNonBlockSocket(select_, ip_, port_, conn_timeout_, errcode_);

    if (sock_ < 0) {
        xerror2(TSF"make socket connect error. ret: %0", sock_);
        status_ = kTcpConnectErr;
    } else {
        xinfo2(TSF"make socket success.");
        status_ = kTcpConnected;
    }
}

TcpQuery::~TcpQuery() {
    free(ip_);
    xinfo2(TSF"close fd in tcpquery,m_sock=%0", sock_);

    if (sock_ >= 0)
        ::socket_close(sock_);
}

TcpErrCode TcpQuery::tcp_send(const unsigned char* _buff, unsigned int _size, int _timeout) {
    if (kTcpConnected == status_) {
        return NetCheckerSocketUtils::writenWithNonBlock(sock_, select_, _timeout, _buff, _size, errcode_);
    }

    return kConnectErr;
}

TcpErrCode TcpQuery::tcp_receive(AutoBuffer& _recvbuf, unsigned int _size, int _timeout) {
    if (kTcpConnected == status_) {
        TcpErrCode ret = NetCheckerSocketUtils::readnWithNonBlock(sock_, select_, _timeout, _recvbuf, _size, errcode_);

        if (kTimeoutErr == ret && _recvbuf.Length() > 0) {
            xinfo2(TSF"receive timeout, success.");
            ret = kTcpSucc;
        }

        return ret;
    }

    return kConnectErr;
}

void TcpQuery::send_break() {
    pipe_.Break();
}
std::string TcpQuery::getStatus() {
    std::string str_status;

    switch (status_) {
    case kTcpInit:
        str_status.append("Tcp init.");
        break;

    case kTcpInitErr:
        str_status.append("Tcp init error.");
        break;

    case kTcpConnectErr:
        str_status.append("Tcp connect error.");
        break;

    case kTcpConnected:
        str_status.append("Tcp connect success.");
        break;

    case kTcpDisConnected:
        str_status.append("Disconnect tcp Connection.");
        break;

    default:
        str_status.append("get status failed!");
    }

    return str_status;
}
