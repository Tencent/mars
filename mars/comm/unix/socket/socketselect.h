/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * SocketSelect.h
 *
 *  Created on: 2013-7-23
 *      Author: jiahongxu
 */


#ifndef _SOCKSTSELECT_
#define _SOCKSTSELECT_

#include "comm/socket/socketpoll.h"

#if 0/*TARGET_OS_MAC*/
#if __APPLE__
#import <TargetConditionals.h>
#if TARGET_OS_MAC
    #include <sys/event.h>
#endif
#endif
class SocketSelect {
  public:
    SocketSelect(SocketBreaker& _breaker, bool _autoclear = false);
    ~SocketSelect();

    void PreSelect();
    void Read_FD_SET(int _socket);
    void Write_FD_SET(int _socket);
    void Exception_FD_SET(int _socket);
    int Select();
    int Select(int _msec);
    int Select(int _sec, int _usec);

    int Errno() const;

    int Read_FD_ISSET(int _socket) const;
    int Write_FD_ISSET(int _socket) const;
    int Exception_FD_ISSET(int _socket) const;

    bool IsBreak() const;
    bool IsException() const;

    SocketBreaker& Breaker();

  private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

  private:
    SocketBreaker& breaker_;

    int kq_;
    struct kevent *events_;
    int trigered_events_;

    int errno_;
    bool autoclear_;
};

#else

class SocketSelect {
  public:
    SocketSelect(SocketBreaker& _breaker, bool _autoclear = false);
    virtual ~SocketSelect();

    void PreSelect();
    void Read_FD_SET(SOCKET _socket);
    void Write_FD_SET(SOCKET _socket);
    void Exception_FD_SET(SOCKET _socket);
    
    virtual int Select();
    virtual int Select(int _msec);

    int  Ret() const;
    int  Errno() const;

    int  Read_FD_ISSET(SOCKET _socket) const;
    int  Write_FD_ISSET(SOCKET _socket) const;
    int  Exception_FD_ISSET(SOCKET _socket) const;

    bool IsBreak() const;
    bool IsException() const;

    SocketBreaker& Breaker();

    SocketPoll&  Poll();
    
  private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

  protected:
    SocketPoll  socket_poll_;
};

#endif
#endif
