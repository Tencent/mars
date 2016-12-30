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

#include "comm/tickcount.h"
#include "comm/socket/unix_socket.h"
#include "comm/socket/socketselect.h"
#include "comm/messagequeue/message_queue.h"

class socket_address;
class AutoBuffer;

namespace coroutine {

SOCKET  block_socket_connect(const socket_address& _address, SocketSelectBreaker& _breaker, int& _errcode, int32_t _timeout=-1/*ms*/);
int     block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1);
int     block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1, bool _wait_full_size=false);

    
class SocketSelect : public ::SocketSelect {
public:
    SocketSelect(SocketSelectBreaker& _breaker);
    ~SocketSelect() {}
    
    virtual int Select(int _msec);
    virtual int Select() { return Select(-1);}
    
private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);
    
public:
    tickcount_t  abs_timeout_;
};
    
class CoroutineSelect;
class TaskInfo;
class RunloopCond : public mq::RunloopCond {
public:
    RunloopCond();
    virtual ~RunloopCond();
    
public:
    void Select(TaskInfo& _task);
    
public:
    virtual const boost::typeindex::type_info& type() const;
    virtual void  Wait(ScopedLock& _lock, long _millisecond);
    virtual void  Notify(ScopedLock& _lock);
    
private:
    RunloopCond(const RunloopCond&);
    void operator=(const RunloopCond&);
    
private:
    CoroutineSelect* coroutine_select;
};
    
}

#define COMPLEX_CONNECT_NAMESPACE coroutine
#include "comm/socket/complexconnect.h"

#endif //MMNET_ASYNC_SOCKET_H_H
