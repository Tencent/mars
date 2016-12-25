/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * TcpServerFSM.h
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#ifndef TcpServerFSM_H_
#define TcpServerFSM_H_

#include "comm/autobuffer.h"
#include "comm/socket/unix_socket.h"

class XLogger;
class SocketSelect;

class TcpServerFSM {
  public:
    enum TSocketStatus {
        kAccept,
        kReadWrite,
        kEnd,
    };

  public:
    TcpServerFSM(SOCKET _socket);
    TcpServerFSM(SOCKET _socket, const sockaddr_in& _addr);
    virtual ~TcpServerFSM();

    TSocketStatus Status() const;
    void Status(TSocketStatus _status);
    bool IsEndStatus() const;

    SOCKET Socket() const;
    const sockaddr_in& Address() const;
    const char* IP() const;
    uint16_t Port() const;
    size_t SendBufLen() {return send_buf_.Length();}
    void Close(bool _notify = true);

    virtual TSocketStatus PreSelect(SocketSelect& _sel, XLogger& _log);
    virtual TSocketStatus AfterSelect(SocketSelect& _sel, XLogger& _log);
    virtual int Timeout() const;

  private:
    TcpServerFSM(const TcpServerFSM&);
    TcpServerFSM& operator=(const TcpServerFSM&);

  protected:
    virtual TSocketStatus PreReadWriteSelect(SocketSelect& _sel, XLogger& _log);
    virtual TSocketStatus AfterReadWriteSelect(const SocketSelect& _sel, XLogger& _log);

    virtual int ReadWriteTimeout() const;
    virtual int ReadWriteAbsTimeout() const;

    virtual void _OnAccept() = 0;
    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) = 0;
    virtual void _OnSend(AutoBuffer& _send_buff, ssize_t _send_len) = 0;
    virtual void _OnClose(TSocketStatus _status, int _error, bool _userclose) = 0;


  protected:
    TSocketStatus status_;
    SOCKET sock_;
    sockaddr_in addr_;
    char ip_[16];

    AutoBuffer send_buf_;
    AutoBuffer recv_buf_;
};

#endif /* TcpServerFSM_H_ */
