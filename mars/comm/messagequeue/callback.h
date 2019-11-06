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
 * callback.h
 *
 *  Created on: 2018-12-13
 *      Author: zhouzhijie
 */


#ifndef callback_h
#define callback_h
#include "mars/comm/thread/thread.h"
#include "message_queue.h"
#include "mars/comm/xlogger/xlogger.h"
#include "boost/bind.hpp"

namespace mars {

    using namespace MessageQueue;

template<class T>
class CallBack {
public:
    typedef boost::function<void ()> invoke_function;

    //if MessageQueue::KNullHandler, will callback in worker thread
    CallBack(const T& _func, MessageHandler_t _handler=MessageQueue::KNullHandler):cb_handler_(_handler), cb_func_(_func), valid_(true) {}

    CallBack(const T& _func, MessageTitle_t _title, MessageHandler_t _handler=MessageQueue::KNullHandler):cb_handler_(_handler), title_(_title), cb_func_(_func), valid_(true) {}
    
    CallBack():cb_handler_(MessageQueue::KNullHandler), valid_(false) {}
    
    void set(const T& _func, MessageTitle_t _title = 0, MessageHandler_t _handler=MessageQueue::KNullHandler) {
        ScopedLock lock(mutex_);
        cb_handler_ = _handler;
        cb_func_ = _func;
        valid_ = true;
        title_ = _title;
    }
    
    operator bool() {
        ScopedLock lock(mutex_);
        return valid_;
    }

    void invalidate() {
        //should call in messagequeue of cb_handler_
        xassert2(MessageQueue::CurrentThreadMessageQueue() == MessageQueue::Handler2Queue(cb_handler_));
        ScopedLock lock(mutex_);
        valid_ = false;
    }
    
    template<typename... Args>
    void operator()(const Args&... rest) {
        ScopedLock lock(mutex_);
        if(!valid_) {
            return;
        }
        
        boost::function<void ()> func = boost::bind(cb_func_, rest...);
        if(MessageQueue::KNullHandler == cb_handler_) {
            func();
            return;
        }
        if(title_ != 0) {
            MessageQueue::AsyncInvoke(func, title_, cb_handler_);
        } else {
            MessageQueue::AsyncInvoke(func, cb_handler_);
        }
    }

private:
    MessageHandler_t cb_handler_;
    MessageTitle_t title_;
    T cb_func_;
    Mutex mutex_;
    bool valid_;
};

}


#endif /* callback_h */
