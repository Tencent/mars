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

#include "coro_socket.h"

#include <list>
#include <climits>

#include "coroutine.h"
#include "comm/socket/socket_address.h"
#include "comm/xlogger/xlogger.h"
#include "comm/thread/lock.h"
#include "comm/time_utils.h"
#include "comm/autobuffer.h"
#include "comm/platform_comm.h"

namespace coroutine {

/* SocketSelect Start */
    
struct TaskInfo {
public:
    TaskInfo(boost::intrusive_ptr<Wrapper> _wrapper, ::SocketPoll& _poll)
    :wrapper_(_wrapper), poll_(_poll) { }

public:
    boost::intrusive_ptr<Wrapper> wrapper_;
    ::SocketPoll& poll_;
    tickcount_t abs_timeout_;
};

class Multiplexing {
public:
    Multiplexing()
    : poll_(breaker_, true)
    {}
    
public:
    void Add(TaskInfo& _task) {
        ScopedLock lock(mutex_);
        tasks_.push_back(&_task);
        poll_.Breaker().Break();
    }
    
    ::SocketBreaker& Breaeker() {
        return poll_.Breaker();
    }
    
    void Run(int32_t _timeout) {
        
        int32_t timeout = _timeout;
        
        __Before(timeout);
        poll_.Poll(timeout);
        __After();
    }
    
private:
    Multiplexing(const Multiplexing&);
    void operator=(const Multiplexing&);
    
private:
    
    void __Before(int32_t &_timeout) {
        
        poll_.ClearEvent();
        ScopedLock lock(mutex_);
        
        for (auto& i: tasks_) {
            TaskInfo& task = *i;
            xverbose2(TSF"task in, id:%_", &task);
            poll_.Consign(task.poll_);
            
            if (!task.abs_timeout_.isValid()) continue;
            
            tickcountdiff_t diff = -task.abs_timeout_.gettickspan();
            if (diff <= 0) {
                _timeout = 0;
                continue;
            }
            
            if (_timeout < 0) {
                _timeout = (int32_t)diff;
                continue;
            }
            
            _timeout = std::min(_timeout, (int32_t)diff);
        }
    }
    
    void __After() {
        
        ScopedLock lock(mutex_);
        std::list<TaskInfo*>::iterator iter = tasks_.begin();
        while (iter != tasks_.end()) {
            TaskInfo& task = **iter;
            
            if(poll_.ConsignReport(task.poll_, task.abs_timeout_.isValid()? -task.abs_timeout_.gettickspan():1)) {
                iter = tasks_.erase(iter);
                coroutine::Resume(task.wrapper_);
            } else {
                ++iter;
            }
        }
    }
    
private:
    Mutex                                   mutex_;
    ::SocketBreaker                         breaker_;
    ::SocketPoll                            poll_;
    std::list<TaskInfo*>                    tasks_;
};

static void __Coro_Poll(int _msec, ::SocketPoll& _socket_poll) {
    
    ::SocketBreaker breaker;
    ::SocketPoll selector(breaker);
    selector.Consign(_socket_poll);
    int ret = selector.Poll(0);
    
    if (0 != ret) {
        selector.ConsignReport(_socket_poll, 1);
        return;
    }
    
    boost::shared_ptr<mq::RunloopCond> cond = mq::RunloopCond::CurrentCond();
    TaskInfo task(coroutine::RunningCoroutine(), _socket_poll);
    if (0 <= _msec) { task.abs_timeout_.gettickcount() += _msec;};
    
    //messagequeue for coro select
    if (cond && cond->type() == boost::typeindex::type_id<coroutine::RunloopCond>()) {
        static_cast<coroutine::RunloopCond*>(cond.get())->Add(task);
        coroutine::Yield();
        return;
    }
    
    //new thread for coro select
    static Multiplexing s_multiplexing;
    static Thread s_thread([&](){
        while (true) {
            s_multiplexing.Run(10*60*1000);
        }
    }, XLOGGER_TAG"::coro_multiplexing");
    s_thread.start();
    s_multiplexing.Add(task);
    coroutine::Yield();
    return;
}
    
int SocketSelect::Select(int _msec) {
    __Coro_Poll(_msec, Poll());
    return Ret();
}
    
int SocketPoll::Poll(int _msec) {
    __Coro_Poll(_msec, *this);
    return Ret();
}

/* SocketSelect End */

RunloopCond::RunloopCond():multiplexing_(new Multiplexing) {}
RunloopCond::~RunloopCond() { delete multiplexing_;}
    
void RunloopCond::Add(TaskInfo& _task) {
    multiplexing_->Add(_task);
}
    
const boost::typeindex::type_info& RunloopCond::type() const {
    return boost::typeindex::type_id<coroutine::RunloopCond>().type_info();
}
    
void RunloopCond::Wait(ScopedLock& _lock, long _millisecond) {
    ASSERT(_lock.islocked());
    multiplexing_->Breaeker().Clear();
    _lock.unlock();
    multiplexing_->Run(int32_t(_millisecond));
    _lock.lock();
}
    
void RunloopCond::Notify(ScopedLock& _lock) {
    ASSERT(_lock.islocked());
    multiplexing_->Breaeker().Break();
}
    
#define SocketSelect coroutine::SocketSelect
#include "comm/socket/block_socket.cc"
#undef SocketSelect
}

#include "comm/socket/tcpclient_fsm.h"
#define SocketSelect coroutine::SocketSelect
#define COMPLEX_CONNECT_NAMESPACE coroutine
#include "comm/socket/complexconnect.cc"
#undef SocketSelect

