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
    TaskInfo(boost::intrusive_ptr<Wrapper> _wrapper, SocketSelect& _sel)
    :wrapper_(_wrapper), sel_(_sel) {
    }

public:
    boost::intrusive_ptr<Wrapper> wrapper_;
    SocketSelect& sel_;
};

class CoroutineSelect {
public:
    CoroutineSelect()
    : selector_(breaker_, true)
    {}
    
public:
    void Select(TaskInfo& _task) {
        ScopedLock lock(mutex_);
        tasks_.push_back(&_task);
        selector_.Breaker().Break();
    }
    
    SocketSelectBreaker& Breaeker() {
        return selector_.Breaker();
    }
    
    void Run(int32_t _timeout) {
        
        int32_t timeout = _timeout;
        
        __Before(timeout);
        if (timeout != INT_MAX) {
            selector_.Select(timeout);
        } else {
            selector_.Select();
        }
        __After();
    }
    
private:
    CoroutineSelect(const CoroutineSelect&);
    void operator=(const CoroutineSelect&);
    
private:
    
    void __Before(int32_t &_timeout) {
        
        selector_.PreSelect();
        ScopedLock lock(mutex_);
        
        for (auto& i: tasks_) {
            TaskInfo& task = *i;
            xverbose2(TSF"task in, id:%_", &task);
            selector_.Consign(task.sel_);
            
            if (!task.sel_.abs_timeout_.isValid()) continue;
            
            tickcountdiff_t diff = -task.sel_.abs_timeout_.gettickspan();
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
            
            if (selector_.Report(task.sel_, task.sel_.abs_timeout_.isValid()? -task.sel_.abs_timeout_.gettickspan():1)) {
                iter = tasks_.erase(iter);
                coroutine::Resume(task.wrapper_);
            } else {
                ++iter;
            }
            
        }
    }
    
private:
    Mutex                                   mutex_;
    SocketSelectBreaker                     breaker_;
    ::SocketSelect                          selector_;
    std::list<TaskInfo*>                    tasks_;
};

SocketSelect::SocketSelect(SocketSelectBreaker& _breaker)
    : ::SocketSelect(_breaker, false) {}
    
int SocketSelect::Select(int _msec) {
    
    if (0 < _msec) {
        abs_timeout_.gettickcount();
        abs_timeout_ += _msec;
    }
    
    SocketSelectBreaker breaker;
    ::SocketSelect selector(breaker);
    selector.PreSelect();
    selector.Consign(*this);
    int ret = selector.Select(0);
    
    if (0 != ret) {
        selector.Report(*this, 1);
        return ret_;
    }
    
    boost::shared_ptr<mq::RunloopCond> cond = mq::RunloopCond::CurrentCond();
    TaskInfo task(coroutine::RunningCoroutine(), *this);
    
    //messagequeue for coro select
    if (cond && cond->type() == boost::typeindex::type_id<RunloopCond>()) {
        static_cast<RunloopCond*>(cond.get())->Select(task);
        coroutine::Yield();
        return ret_;
    }
    
    //new thread for coro select
    static CoroutineSelect s_corotine_select;
    static Thread s_thread([&](){
        while (true) {
            s_corotine_select.Run(10*60*1000);
        }
    }, XLOGGER_TAG"::coro_select");
    s_thread.start();
    s_corotine_select.Select(task);
    coroutine::Yield();
    return ret_;
}

/* SocketSelect End */

RunloopCond::RunloopCond():coroutine_select(new CoroutineSelect) {}
RunloopCond::~RunloopCond() { delete coroutine_select;}
    
void RunloopCond::Select(TaskInfo& _task) {
    coroutine_select->Select(_task);
}
    
const boost::typeindex::type_info& RunloopCond::type() const {
        return boost::typeindex::type_id<RunloopCond>().type_info();
}
    
void RunloopCond::Wait(ScopedLock& _lock, long _millisecond) {
    ASSERT(_lock.islocked());
    coroutine_select->Breaeker().Clear();
    _lock.unlock();
    coroutine_select->Run(int32_t(_millisecond));
    _lock.lock();
}
    
void RunloopCond::Notify(ScopedLock& _lock) {
    ASSERT(_lock.islocked());
    coroutine_select->Breaeker().Break();
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

