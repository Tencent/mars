// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COROUTINE_H_
#define COROUTINE_H_

#include <cstdint>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/coroutine/all.hpp>

#include "comm/messagequeue/message_queue.h"
#include "../thread/thread.h"

namespace coroutine {

class Wrapper: public boost::intrusive_ref_counter<Wrapper> {
    
friend class Coroutine;
friend void Yield();
friend void Resume(const boost::intrusive_ptr<Wrapper>&, int64_t);
    
typedef boost::coroutines::asymmetric_coroutine<void>::pull_type pull_coro_t;
typedef boost::coroutines::asymmetric_coroutine<void>::push_type push_coro_t;
    
public:
    template <typename F>
    Wrapper(const F& _func, const MessageQueue::MessageHandler_t& _handler)
    :handler_(_handler)
    , pull_obj_ptr_(NULL)
    , push_obj_([_func, this](pull_coro_t& sink){
        this->pull_obj_ptr_ = &sink;
        _func();
    })
    {}

private:
    void _Yield() {
        if (pull_obj_ptr_ == NULL || !(*pull_obj_ptr_)) return;
        
        (*pull_obj_ptr_)();
    }

private:
    Wrapper& operator=(const Wrapper& rhs);
    Wrapper(const Wrapper& rhs);

private:
    MessageQueue::MessageHandler_t  handler_;
    pull_coro_t*                    pull_obj_ptr_;
    push_coro_t                     push_obj_;
};
    
/////////static function//////////////
inline boost::intrusive_ptr<Wrapper> RunningCoroutine() {
    const MessageQueue::Message& running_msg = MessageQueue::RuningMessage();
    ASSERT(running_msg.body2.type() == boost::typeindex::type_id<boost::intrusive_ptr<Wrapper> >());
    return (boost::any_cast<boost::intrusive_ptr<Wrapper> >(running_msg.body2));
}
    
inline void Yield() {
    RunningCoroutine()->_Yield();
}

inline void Resume(const boost::intrusive_ptr<Wrapper>& _wrapper, int64_t _after=0) {
    
    MessageQueue::Message message;
    message.body1 = boost::make_shared<MessageQueue::AsyncInvokeFunction>([_wrapper](){ _wrapper->push_obj_(); });
    message.body2 = _wrapper;
    message.title = _wrapper.get();
    
    MessageQueue::PostMessage(_wrapper->handler_, message, _after);
}
    
inline void Wait(int64_t _after = std::numeric_limits<int64_t>::max()) {
    if (0 > _after) { return; }

    Resume(RunningCoroutine(), _after);
    Yield();
}

class Coroutine {
public:
    template <typename F>
    Coroutine(const F& _func, const MessageQueue::MessageHandler_t& _handler):wrapper_(new Wrapper(_func, _handler)) {}
    
    void Start() { Resume(wrapper_); }
    
    void Join() {
        MessageQueue::WaitInvoke([this](){
            MessageQueue::RunLoop([this](){ return !this->wrapper_->push_obj_;}).Run();
            ASSERT(!(this->wrapper_->push_obj_));
            MessageQueue::CancelMessage(this->wrapper_->handler_, this->wrapper_.get());
        }, wrapper_->handler_);
    }

    void Notify() {
        MessageQueue::CancelMessage(wrapper_->handler_, wrapper_.get());
        Resume(wrapper_);
    }
    
    void Wait(int64_t _after = 0) {
        if (0 > _after) { return; }
        if (RunningCoroutine().get() != wrapper_.get()) { return; }

        Resume(wrapper_, _after);
        Yield();
    }
    
private:
    Coroutine& operator=(const Coroutine& rhs);
    Coroutine(const Coroutine& rhs);
    
private:
    boost::intrusive_ptr<Wrapper> wrapper_;
};
    
template <typename F>
void AsyncFunc(const F& _block_func) {
    
    boost::intrusive_ptr<Wrapper> wrapper = RunningCoroutine();
    
    Thread thread([_block_func, wrapper]()
    {
        _block_func();
        Resume(wrapper);
    });
    
    thread.start();
    Yield();
}

}

#endif
