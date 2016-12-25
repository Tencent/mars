/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
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
    bool have_read_date_;
    bool will_disconnect_;
    int writedbufid_;
    std::list<AutoBuffer*> lst_buffer_;

    Thread thread_;
    mutable Mutex write_mutex_;
    mutable Mutex read_disconnect_mutex_;
    Mutex connect_mutex_;

    SocketSelectBreaker pipe_;

    int timeout_;
    volatile TTcpStatus status_;
};

#endif
