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
 * TcpClient.c
 *
 *  Created on: 2013-5-7
 *      Author: yerungui && jiahong
 */



#ifndef __TcpClient__
#define __TcpClient__

#include "comm/socket/socketselect.h"
#include <list>
#include "comm/thread/thread.h"
#include "comm/thread/lock.h"
#include "comm/socket/unix_socket.h"

class AutoBuffer;

class MTcpEvent {
  public:
    virtual ~MTcpEvent() {}

    virtual void OnConnect() = 0;
    virtual void OnDisConnect(bool _isremote) = 0;
    virtual void OnError(int _status, int _errcode) = 0;

    virtual void OnWrote(int _id, unsigned int _len) = 0;
    virtual void OnAllWrote() = 0;

    virtual void OnRead() = 0;
};

class TcpClient {
  public:
    enum TTcpStatus {
        kTcpInit = 0,
        kTcpInitErr,
        kSocketThreadStart,
        kSocketThreadStartErr,
        kTcpConnecting,
        kTcpConnectIpErr,
        kTcpConnectingErr,
        kTcpConnectTimeoutErr,
        kTcpConnected,
        kTcpIOErr,
        kTcpDisConnectedbyRemote,
        kTcpDisConnected,
    };

  public:
    TcpClient(const char* _ip, uint16_t _port, MTcpEvent& _event, int _timeout = 6 * 1000);
    ~TcpClient();

  public:
    bool Connect();
    void Disconnect();
    void DisconnectAndWait();

    bool HaveDataRead() const;
    ssize_t Read(void* _buf, unsigned int _len);

    bool HaveDataWrite() const;
    int Write(const void* _buf, unsigned int _len);
    int WritePostData(void* _buf, unsigned int _len);

    const char* GetIP() const {return ip_;}
    uint16_t GetPort() const {return port_;}

    TTcpStatus GetTcpStatus() const {return status_;}

  private:
    void __Run();
    void __RunThread();
    void __SendBreak();

  private:
    char* ip_;
    uint16_t port_;
    MTcpEvent& event_;

    SOCKET socket_;
    bool have_read_data_;
    bool will_disconnect_;
    int writedbufid_;
    std::list<AutoBuffer*> lst_buffer_;

    Thread thread_;
    mutable Mutex write_mutex_;
    mutable Mutex read_disconnect_mutex_;
    Mutex connect_mutex_;

    SocketBreaker pipe_;

    int timeout_;
    volatile TTcpStatus status_;
};

#endif
