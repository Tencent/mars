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
 * UdpServer.h
 *
 *  Created on: 2014-9-9
 *      Author: zhouzhijie
 */

#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include "comm/socket/socketselect.h"
#include "comm/thread/thread.h"
#include "comm/thread/lock.h"
#include "comm/socket/unix_socket.h"

#include <string>
#include <list>

#include "comm/socket/unix_socket.h"
#include "comm/socket/socketselect.h"
#include "comm/thread/thread.h"
#include "comm/thread/mutex.h"
#include "comm/autobuffer.h"

#define IPV4_BROADCAST_IP "255.255.255.255"

struct UdpServerSendData;
class UdpServer;

class IAsyncUdpServerEvent {
  public:
    virtual ~IAsyncUdpServerEvent() {}
    virtual void OnError(UdpServer* _this, int _errno) = 0;
    virtual void OnDataGramRead(UdpServer* _this, struct sockaddr_in* _addr, void* _buf, size_t _len) = 0;
};

class UdpServer {
  public:
    UdpServer(int _port, IAsyncUdpServerEvent* _event);
    ~UdpServer();

    void SendBroadcast(int _port, void* _buf, size_t _len);
    void SendAsync(const std::string& _ip, int _port, void* _buf, size_t _len);
    void SendAsync(struct sockaddr_in* _addr, void* _buf, size_t _len);

  private:
    void __InitSocket(int _port);
    int __DoSelect(bool _bReadSet, bool _bWriteSet, void* _buf, size_t _len, struct sockaddr_in* _addr, int& _errno);
    void __RunLoop();
    bool __SetBroadcastOpt();

  private:
    SOCKET fd_socket_;
    IAsyncUdpServerEvent* event_;

	SocketBreaker breaker_;
    SocketSelect selector_;
    Thread* thread_;

    std::list<UdpServerSendData> list_buffer_;
    Mutex mutex_;
};

#endif /* UDPSERVER_H_ */
