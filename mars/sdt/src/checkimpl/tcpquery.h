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
 * TcpQuery.h
 *
 *  Created on: 2014年6月30日
 *      Author: wutianqiang
 */

#ifndef SDT_SRC_CHECKIMPL_TCPQUERY_H_
#define SDT_SRC_CHECKIMPL_TCPQUERY_H_

#include "mars/sdt/sdt.h"

#include "mars/comm/thread/mutex.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/socket/socketselect.h"

class NetCheckTrafficMonitor;

namespace mars {
namespace sdt {

class TcpQuery {

    enum TcpStatus {
        kTcpInit = 0,
        kTcpInitErr,
        kTcpConnectErr,
        kTcpConnected,
        kTcpDisConnected,
    };

  public:
    TcpQuery(const char* _ip, uint16_t _port, unsigned int connTimeout, NetCheckTrafficMonitor* trafficMonitor = NULL);
    virtual ~TcpQuery();

  public:
    TcpErrCode tcp_send(const unsigned char* buff, unsigned int unSize, int timeoutMs);
    TcpErrCode tcp_receive(AutoBuffer& recvBuf, unsigned int unSize, int timeoutMs);

    void send_break();
    std::string getStatus();

  private:
    char* ip_;
    uint16_t port_;
    SOCKET sock_;
    SocketBreaker pipe_;
    SocketSelect select_;
    TcpStatus status_;
    int errcode_;
    unsigned int conn_timeout_;
};

}}

#endif /* SDT_SRC_CHECKIMPL_TCPQUERY_H_ */
