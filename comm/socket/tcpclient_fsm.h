/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * TcpClientFSM.h
 *
 *  Created on: 2014-8-19
 *      Author: yerungui
 */

#ifndef TCPCLIENTFSM_H_
#define TCPCLIENTFSM_H_

#include "autobuffer.h"
#include "socket/unix_socket.h"
#include "socket/socket_address.h"

class XLogger;
class SocketSelect;

class TcpClientFSM {
  public:
    enum TSocketStatus {
        EStart,
        EConnecting,
        EReadWrite,
        EEnd,
    };

  public:
    TcpClientFSM(const sockaddr& _addr);
    virtual ~TcpClientFSM();

    void RequestSend();

    TSocketStatus Status() const;
    TSocketStatus LastStatus() const;
    void Status(TSocketStatus _status);
    bool IsEndStatus() const;
    int Error() const;

    SOCKET Socket() const;
    void Socket(SOCKET _sock);
    const sockaddr& Address() const;
    const char* IP() const;
    uint16_t Port() const;

    void Close(bool _notify = true);
    bool RemoteClose() const;

    virtual void PreSelect(SocketSelect& _sel, XLogger& _log);
    virtual void AfterSelect(SocketSelect& _sel, XLogger& _log);
    virtual int Timeout() const;
    int Rtt() const;

  private:
    TcpClientFSM(const TcpClientFSM&);
    TcpClientFSM& operator=(const TcpClientFSM&);

  protected:
    virtual void PreConnectSelect(SocketSelect& _sel, XLogger& _log);
    virtual void AfterConnectSelect(const SocketSelect& _sel, XLogger& _log);
    virtual void PreReadWriteSelect(SocketSelect& _sel, XLogger& _log);
    virtual void AfterReadWriteSelect(const SocketSelect& _sel, XLogger& _log);

    virtual int ConnectTimeout() const;
    virtual int ReadWriteTimeout() const;

    virtual int ConnectAbsTimeout() const;
    virtual int ReadWriteAbsTimeout() const;

    virtual void _OnCreate() = 0;
    virtual void _OnConnect() = 0;
    virtual void _OnConnected(int _rtt) = 0;
    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) = 0;
    virtual void _OnRequestSend(AutoBuffer& _send_buff) {}
    virtual void _OnSend(AutoBuffer& _send_buff, ssize_t _send_len) = 0;
    virtual void _OnClose(TSocketStatus _status, int _error, bool _userclose) = 0;


  protected:
    TSocketStatus status_;
    TSocketStatus last_status_;
    int           error_;
    bool           remote_close_;
    bool           request_send_;

    socket_address addr_;
    SOCKET sock_;

    uint64_t start_connecttime_;
    uint64_t end_connecttime_;

    AutoBuffer send_buf_;
    AutoBuffer recv_buf_;
};

#endif /* TCPCLIENTFSM_H_ */
