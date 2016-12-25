/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * TcpServer.h
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#ifndef TcpServer_H_
#define TcpServer_H_

#include "comm/socket/unix_socket.h"
#include "comm/socket/socketselect.h"
#include "comm/thread/mutex.h"
#include "comm/thread/condition.h"
#include "comm/thread/thread.h"

class XLogger;
class SocketSelect;
class TcpServer;

class MTcpServer {
  public:
    virtual ~MTcpServer() {}
    virtual void OnCreate(TcpServer* _server) = 0;
    virtual void OnAccept(TcpServer* _server, SOCKET _sock, const sockaddr_in& _addr) = 0;
    virtual void OnError(TcpServer* _server, int _error) = 0;
};

class TcpServer {
  public:
    TcpServer(const char* _ip, uint16_t _port, MTcpServer& _observer, int _backlog = 256);
    TcpServer(uint16_t _port, MTcpServer& _observer, int _backlog = 256);
    TcpServer(const sockaddr_in& _bindaddr, MTcpServer& _observer, int _backlog = 256);
    ~TcpServer();

    SOCKET Socket() const;
    const sockaddr_in& Address() const;

    bool StartAndWait(bool* _newone = NULL);
    void StopAndWait();

  private:
    TcpServer(const TcpServer&);
    TcpServer& operator=(const TcpServer&);

  private:
    void __ListenThread();

  protected:
    MTcpServer&         observer_;
    Thread              thread_;
    Mutex                  mutex_;
    Condition            cond_;

    SOCKET                 listen_sock_;
    sockaddr_in         bind_addr_;
    const int             backlog_;
    SocketSelectBreaker breaker_;
};

#endif /* TcpServer_H_ */
