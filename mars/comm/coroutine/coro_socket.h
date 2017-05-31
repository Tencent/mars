// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//

#ifndef MMNET_ASYNC_SOCKET_H_H
#define MMNET_ASYNC_SOCKET_H_H

#include "mars/comm/tickcount.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/socket/socketpoll.h"
#include "mars/comm/messagequeue/message_queue.h"

class socket_address;
class AutoBuffer;

namespace coroutine {

SOCKET  block_socket_connect(const socket_address& _address, SocketBreaker& _breaker, int& _errcode, int32_t _timeout=-1/*ms*/);
int     block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketBreaker& _breaker, int &_errcode, int _timeout=-1);
int     block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketBreaker& _breaker, int &_errcode, int _timeout=-1, bool _wait_full_size=false);
    
class SocketSelect : public ::SocketSelect {
public:
    SocketSelect(SocketBreaker& _breaker)
    : ::SocketSelect(_breaker, false) {}
    virtual ~SocketSelect() {}
    
    virtual int Select(int _msec);
    virtual int Select() { return Select(-1);}
    
private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);
};
    
class SocketPoll : public ::SocketPoll {
public:
    SocketPoll(SocketBreaker& _breaker)
    : ::SocketPoll(_breaker, false) {}
    virtual ~SocketPoll() {}
    
    virtual int Poll(int _msec);
    virtual int Poll() { return Poll(-1);}
    
private:
    SocketPoll(const SocketPoll&);
    SocketPoll& operator=(const SocketPoll&);
};
    
class Multiplexing;
struct TaskInfo;
class RunloopCond : public mq::RunloopCond {
public:
    RunloopCond();
    virtual ~RunloopCond();
    
public:
    void Add(TaskInfo& _task);
    
public:
    virtual const boost::typeindex::type_info& type() const;
    virtual void  Wait(ScopedLock& _lock, long _millisecond);
    virtual void  Notify(ScopedLock& _lock);
    
private:
    RunloopCond(const RunloopCond&);
    void operator=(const RunloopCond&);
    
private:
    Multiplexing* multiplexing_;
};
    
}

#define COMPLEX_CONNECT_NAMESPACE coroutine
#include "mars/comm/socket/complexconnect.h"
#undef COMPLEX_CONNECT_NAMESPACE

#include "./dns.h"

#endif //MMNET_ASYNC_SOCKET_H_H
