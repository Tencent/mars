/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#ifndef _SOCKSTSELECT_
#define _SOCKSTSELECT_

#include <winsock2.h>
#include "thread/lock.h"

class SocketSelectBreaker {
  public:
    SocketSelectBreaker();
    ~SocketSelectBreaker();

    bool IsCreateSuc() const;
    bool ReCreate();

    bool IsBreak() const;

    bool Break();
    bool Clear();
    void Close();

    SOCKET BreakerFD() const;

  private:
    SocketSelectBreaker(const SocketSelectBreaker&);
    SocketSelectBreaker& operator=(const SocketSelectBreaker&);

  private:
    int  m_sendinlen;
    SOCKET m_socket_w, m_socket_r;
    struct sockaddr m_sendin;
    bool m_create_success;
    bool m_broken;
    Mutex m_mutex;
};

class SocketSelect {
  public:
    SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear = false);
    ~SocketSelect();

    void PreSelect();
    void Read_FD_SET(SOCKET _socket);
    void Write_FD_SET(SOCKET _socket);
    void Exception_FD_SET(SOCKET _socket);
    int Select();
    int Select(int _msec);
    int Select(int _sec, int _usec);

    int Errno() const;

    int Read_FD_ISSET(SOCKET _socket) const;
    int Write_FD_ISSET(SOCKET _socket) const;
    int Exception_FD_ISSET(SOCKET _socket) const;

    bool IsBreak() const;
    bool IsException() const;

    SocketSelectBreaker& Breaker();

  private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

  private:
    SocketSelectBreaker& breaker_;
    SOCKET maxsocket_;

    fd_set readfd_;
    fd_set writefd_;
    fd_set exceptionfd_;

    int errno_;
    bool autoclear_;
};

#endif
