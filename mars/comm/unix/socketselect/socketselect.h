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
 * SocketSelect.h
 *
 *  Created on: 2013-7-23
 *      Author: jiahongxu
 */


#ifndef _SOCKSTSELECT_
#define _SOCKSTSELECT_

#include <sys/select.h>
#include <poll.h>
#include <vector>

#if __APPLE__
#import <TargetConditionals.h>
#if TARGET_OS_MAC
    #include <sys/event.h>
#endif
#endif

#include "mars/comm/thread/lock.h"
#include "mars/comm/socket/unix_socket.h"

class SocketSelectBreaker {
  public:
    SocketSelectBreaker();
    ~SocketSelectBreaker();

    bool IsCreateSuc() const;
    bool ReCreate();
    void Close();

    bool Break();
    bool Clear();

    bool IsBreak() const;
    int BreakerFD() const;

  private:
    SocketSelectBreaker(const SocketSelectBreaker&);
    SocketSelectBreaker& operator=(const SocketSelectBreaker&);

  private:
    int pipes_[2];
    bool create_success_;
    bool broken_;
    Mutex mutex_;
};

#if 0/*TARGET_OS_MAC*/
class SocketSelect {
  public:
    SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear = false);
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

    SocketSelectBreaker& Breaker();

  private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

  private:
    SocketSelectBreaker& breaker_;

    int kq_;
    struct kevent *events_;
    int trigered_events_;

    int errno_;
    bool autoclear_;
};

#else
class SocketSelect {
  public:
    SocketSelect(SocketSelectBreaker& _breaker, bool _autoclear = false);
    ~SocketSelect();

    void PreSelect();
    void Consign(SocketSelect& _consignor);
    void Read_FD_SET(int _socket);
    void Write_FD_SET(int _socket);
    void Exception_FD_SET(int _socket);
    
    virtual int Select();
    virtual int Select(int _msec);

    int  Errno() const;

    bool Report(SocketSelect& _consignor, int64_t _timeout);
    int  Read_FD_ISSET(int _socket) const;
    int  Write_FD_ISSET(int _socket) const;
    int  Exception_FD_ISSET(int _socket) const;

    bool IsBreak() const;
    bool IsException() const;

    SocketSelectBreaker& Breaker();

  private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

  protected:
    SocketSelectBreaker&       breaker_;
    std::vector<struct pollfd> vfds_;

    int         ret_;
    int         errno_;
    const bool  autoclear_;
};

#endif
#endif
