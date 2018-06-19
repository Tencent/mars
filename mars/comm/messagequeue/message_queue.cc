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
 * MessageQueue.cpp
 *
 *  Created on: 2013-4-3
 *      Author: yerungui
 */

#include <map>
#include <list>
#include <string>
#include <algorithm>
#ifndef _WIN32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include "boost/bind.hpp"

#include "comm/thread/lock.h"
#include "comm/anr.h"
#include "comm/messagequeue/message_queue.h"
#include "comm/time_utils.h"
#include "comm/bootrun.h"
#include "comm/xlogger/xlogger.h"
#ifdef __APPLE__
#include "comm/debugger/debugger_utils.h"
#endif

#ifdef ANDROID
#include "android/fatal_assert.h"
#endif

#undef min

namespace MessageQueue {

static unsigned int __MakeSeq() {
    static unsigned int s_seq = 0;

    return ++s_seq;
}

struct MessageWrapper {
    MessageWrapper(const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing, unsigned int _seq)
        : message(_message), timing(_timing) {
        postid.reg = _handlerid;
        postid.seq = _seq;
        periodstatus = kImmediately;
        record_time = 0;

        if (kImmediately != _timing.type) {
            periodstatus = kAfter;
            record_time = ::gettickcount();
        }
    }

    ~MessageWrapper() {
        if (wait_end_cond)
            wait_end_cond->notifyAll();
    }

    MessagePost_t postid;
    Message message;

    MessageTiming timing;
    TMessageTiming periodstatus;
    uint64_t record_time;
    boost::shared_ptr<Condition> wait_end_cond;
};

struct HandlerWrapper {
    HandlerWrapper(const MessageHandler& _handler, bool _recvbroadcast, const MessageQueue_t& _messagequeueid, unsigned int _seq)
        : handler(_handler), recvbroadcast(_recvbroadcast) {
        reg.seq = _seq;
        reg.queue = _messagequeueid;
    }

    MessageHandler_t reg;
    MessageHandler handler;
    bool recvbroadcast;
};

struct RunLoopInfo {
    RunLoopInfo():runing_message(NULL) { runing_cond = boost::make_shared<Condition>();}
    
    boost::shared_ptr<Condition> runing_cond;
    MessagePost_t runing_message_id;
    Message* runing_message;
    std::list <MessageHandler_t> runing_handler;
};
    
class Cond : public RunloopCond {
public:
    Cond(){}
    
public:
    const boost::typeindex::type_info& type() const {
        return boost::typeindex::type_id<Cond>().type_info();
    }
    
    virtual void Wait(ScopedLock& _lock, long _millisecond) {
        cond_.wait(_lock, _millisecond);
    }
    virtual void Notify(ScopedLock& _lock) {
        cond_.notifyAll(_lock);
    }
    
private:
    Cond(const Cond&);
    void operator=(const Cond&);
    
private:
    Condition cond_;
};
    
struct MessageQueueContent {
    MessageQueueContent(): breakflag(false) {}

#if defined(ANDROID)
    MessageQueueContent(const MessageQueueContent&): breakflag(false) { /*ASSERT(false);*/ }
#endif

    MessageHandler_t invoke_reg;
    bool breakflag;
    boost::shared_ptr<RunloopCond> breaker;
    std::list<MessageWrapper*> lst_message;
    std::list<HandlerWrapper*> lst_handler;
    
    std::list<RunLoopInfo> lst_runloop_info;
    
private:
    void operator=(const MessageQueueContent&);
    
#if !defined(ANDROID)
    MessageQueueContent(const MessageQueueContent&);
#endif
};

#define sg_messagequeue_map_mutex messagequeue_map_mutex()
static Mutex& messagequeue_map_mutex() {
    static Mutex* mutex = new Mutex;
    return *mutex;
}
#define sg_messagequeue_map messagequeue_map()
static std::map<MessageQueue_t, MessageQueueContent>& messagequeue_map() {
    static std::map<MessageQueue_t, MessageQueueContent>* mq_map = new std::map<MessageQueue_t, MessageQueueContent>;
    return *mq_map;
}

MessageQueue_t CurrentThreadMessageQueue() {
    ScopedLock lock(sg_messagequeue_map_mutex);
    MessageQueue_t id = (MessageQueue_t)ThreadUtil::currentthreadid();

    if (sg_messagequeue_map.end() == sg_messagequeue_map.find(id)) id = KInvalidQueueID;

    return id;
}

MessageQueue_t TID2MessageQueue(thread_tid _tid) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    MessageQueue_t id = (MessageQueue_t)_tid;

    if (sg_messagequeue_map.end() == sg_messagequeue_map.find(id))id = KInvalidQueueID;

    return id;
}
    
thread_tid  MessageQueue2TID(MessageQueue_t _id) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    MessageQueue_t& id = _id;
    
    if (sg_messagequeue_map.end() == sg_messagequeue_map.find(id)) return 0;
    
    return (thread_tid)id;
}

void WaitForRunningLockEnd(const MessagePost_t&  _message) {
    if (Handler2Queue(Post2Handler(_message)) == CurrentThreadMessageQueue()) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = Handler2Queue(Post2Handler(_message));

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return;
    MessageQueueContent& content = pos->second;
    
    if (content.lst_runloop_info.empty()) return;
    
    auto find_it = std::find_if(content.lst_runloop_info.begin(), content.lst_runloop_info.end(),
                                [&_message](const RunLoopInfo& _v){ return _message == _v.runing_message_id; });
    
    if (find_it == content.lst_runloop_info.end()) return;

    boost::shared_ptr<Condition> runing_cond = find_it->runing_cond;
    runing_cond->wait(lock);
}

void WaitForRunningLockEnd(const MessageQueue_t&  _messagequeueid) {
    if (_messagequeueid == CurrentThreadMessageQueue()) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _messagequeueid;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return;
    MessageQueueContent& content = pos->second;

    if (content.lst_runloop_info.empty()) return;
    if (KNullPost == content.lst_runloop_info.front().runing_message_id) return;

    boost::shared_ptr<Condition> runing_cond = content.lst_runloop_info.front().runing_cond;
    runing_cond->wait(lock);
}

void WaitForRunningLockEnd(const MessageHandler_t&  _handler) {
    if (Handler2Queue(_handler) == CurrentThreadMessageQueue()) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = Handler2Queue(_handler);

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) { return; }
    MessageQueueContent& content = pos->second;
    if (content.lst_runloop_info.empty()) return;

    for(auto& i : content.lst_runloop_info) {
        for (auto& x : i.runing_handler) {
            if (_handler==x) {
                boost::shared_ptr<Condition> runing_cond = i.runing_cond;
                runing_cond->wait(lock);
                return;
            }
        }
    }
}

void BreakMessageQueueRunloop(const MessageQueue_t&  _messagequeueid) {
    ASSERT(0 != _messagequeueid);

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _messagequeueid;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        //ASSERT2(false, "%llu", (unsigned long long)id);
        return;
    }

    pos->second.breakflag = true;
    pos->second.breaker->Notify(lock);
}

MessageHandler_t InstallMessageHandler(const MessageHandler& _handler, bool _recvbroadcast, const MessageQueue_t& _messagequeueid) {
    ASSERT(bool(_handler));

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _messagequeueid;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        ASSERT2(false, "%llu", (unsigned long long)id);
        return KNullHandler;
    }

    HandlerWrapper* handler = new HandlerWrapper(_handler, _recvbroadcast, _messagequeueid, __MakeSeq());
    pos->second.lst_handler.push_back(handler);
    return handler->reg;
}

void UnInstallMessageHandler(const MessageHandler_t& _handlerid) {
    ASSERT(0 != _handlerid.queue);
    ASSERT(0 != _handlerid.seq);

    if (0 == _handlerid.queue || 0 == _handlerid.seq) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return;

    MessageQueueContent& content = pos->second;

    for (std::list<HandlerWrapper*>::iterator it = content.lst_handler.begin(); it != content.lst_handler.end(); ++it) {
        if (_handlerid == (*it)->reg) {
            delete(*it);
            content.lst_handler.erase(it);
            break;
        }
    }
}

MessagePost_t PostMessage(const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        //ASSERT2(false, "%" PRIu64, id);
        return KNullPost;
    }

    MessageQueueContent& content = pos->second;

    MessageWrapper* messagewrapper = new MessageWrapper(_handlerid, _message, _timing, __MakeSeq());

    content.lst_message.push_back(messagewrapper);
    content.breaker->Notify(lock);
    return messagewrapper->postid;
}

MessagePost_t SingletonMessage(bool _replace, const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return KNullPost;

    MessageQueueContent& content = pos->second;

    MessagePost_t post_id;

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
        if ((*it)->postid.reg == _handlerid && (*it)->message == _message) {
            if (_replace) {
                post_id = (*it)->postid;
                delete(*it);
                content.lst_message.erase(it);
                break;
            } else {
                return (*it)->postid;
            }
        }
    }

    MessageWrapper* messagewrapper = new MessageWrapper(_handlerid, _message, _timing, 0 != post_id.seq ? post_id.seq : __MakeSeq());
    content.lst_message.push_back(messagewrapper);
    content.breaker->Notify(lock);
    return messagewrapper->postid;
}

MessagePost_t BroadcastMessage(const MessageQueue_t& _messagequeueid,  const Message& _message, const MessageTiming& _timing) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _messagequeueid;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        ASSERT2(false, "%" PRIu64, id);
        return KNullPost;
    }

    MessageQueueContent& content = pos->second;

    MessageHandler_t reg;
    reg.queue = _messagequeueid;
    reg.seq = 0;
    MessageWrapper* messagewrapper = new MessageWrapper(reg, _message, _timing, __MakeSeq());

    content.lst_message.push_back(messagewrapper);
    content.breaker->Notify(lock);
    return messagewrapper->postid;
}

static int64_t __ComputerWaitTime(const MessageWrapper& _wrap) {
    int64_t wait_time = 0;

    if (kImmediately == _wrap.timing.type) {
        wait_time = 0;
    } else if (kAfter == _wrap.timing.type) {
        int64_t time_cost = ::gettickspan(_wrap.record_time);
        wait_time =  _wrap.timing.after - time_cost;
    } else if (kPeriod == _wrap.timing.type) {
        int64_t time_cost = ::gettickspan(_wrap.record_time);

        if (kAfter == _wrap.periodstatus) {
            wait_time =  _wrap.timing.after - time_cost;
        } else if (kPeriod == _wrap.periodstatus) {
            wait_time =  _wrap.timing.period - time_cost;
        }
    }

    return 0 < wait_time ? wait_time : 0;
}

MessagePost_t FasterMessage(const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return KNullPost;

    MessageQueueContent& content = pos->second;

    MessageWrapper* messagewrapper = new MessageWrapper(_handlerid, _message, _timing, __MakeSeq());

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
        if ((*it)->postid.reg == _handlerid && (*it)->message == _message) {
            if (__ComputerWaitTime(**it) < __ComputerWaitTime(*messagewrapper)) {
                delete messagewrapper;
                return (*it)->postid;
            }

            messagewrapper->postid = (*it)->postid;
            delete(*it);
            content.lst_message.erase(it);
            break;
        }
    }

    content.lst_message.push_back(messagewrapper);
    content.breaker->Notify(lock);
    return messagewrapper->postid;
}

bool WaitMessage(const MessagePost_t& _message) {
    bool is_in_mq = Handler2Queue(Post2Handler(_message)) == CurrentThreadMessageQueue();

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = Handler2Queue(Post2Handler(_message));
    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return false;
    MessageQueueContent& content = pos->second;

    auto find_it = std::find_if(content.lst_message.begin(), content.lst_message.end(),
                                [&_message](const MessageWrapper * const &_v) {
                                    return _message == _v->postid;
                                });
    
    if (find_it == content.lst_message.end()) {
        auto find_it = std::find_if(content.lst_runloop_info.begin(), content.lst_runloop_info.end(),
                     [&_message](const RunLoopInfo& _v){ return _message == _v.runing_message_id; });
        
        if (find_it != content.lst_runloop_info.end()) {
            if (is_in_mq) return false;
            
            boost::shared_ptr<Condition> runing_cond = find_it->runing_cond;
            runing_cond->wait(lock);
        }
    } else {
        
        if (is_in_mq) {
            lock.unlock();
            RunLoop( [&_message](){
                        MessageQueueContent& content = sg_messagequeue_map[Handler2Queue(Post2Handler(_message))];
                        return content.lst_message.end() == std::find_if(content.lst_message.begin(), content.lst_message.end(),
                                                                [&_message](const MessageWrapper *  const &_v) {
                                                                    return _message == _v->postid;
                                                                });
            }).Run();
            
        } else {
            if (!((*find_it)->wait_end_cond))(*find_it)->wait_end_cond = boost::make_shared<Condition>();

            boost::shared_ptr<Condition> wait_end_cond = (*find_it)->wait_end_cond;
            wait_end_cond->wait(lock);
        }
    }

    return true;
}

bool FoundMessage(const MessagePost_t& _message) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = Handler2Queue(Post2Handler(_message));

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return false;
    MessageQueueContent& content = pos->second;
    if (content.lst_runloop_info.empty()) return false;

    auto find_it = std::find_if(content.lst_runloop_info.begin(), content.lst_runloop_info.end(),
                                [&_message](const RunLoopInfo& _v){ return _message == _v.runing_message_id; });
    
    if (find_it != content.lst_runloop_info.end())  { return true; }

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
        if (_message == (*it)->postid) { return true;}
    }

    return false;
}

bool CancelMessage(const MessagePost_t& _postid) {
    ASSERT(0 != _postid.reg.queue);
    ASSERT(0 != _postid.seq);

    // 0==_postid.reg.seq for BroadcastMessage
    if (0 == _postid.reg.queue || 0 == _postid.seq) return false;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _postid.reg.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        ASSERT2(false, "%" PRIu64, id);
        return false;
    }

    MessageQueueContent& content = pos->second;

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
        if (_postid == (*it)->postid) {
            delete(*it);
            content.lst_message.erase(it);
            return true;
        }
    }

    return false;
}

void CancelMessage(const MessageHandler_t& _handlerid) {
    ASSERT(0 != _handlerid.queue);

    // 0==_handlerid.seq for BroadcastMessage
    if (0 == _handlerid.queue) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        //        ASSERT2(false, "%lu", id);
        return;
    }

    MessageQueueContent& content = pos->second;

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end();) {
        if (_handlerid == (*it)->postid.reg) {
            delete(*it);
            it = content.lst_message.erase(it);
        } else {
            ++it;
        }
    }
}

void CancelMessage(const MessageHandler_t& _handlerid, const MessageTitle_t& _title) {
    ASSERT(0 != _handlerid.queue);

    // 0==_handlerid.seq for BroadcastMessage
    if (0 == _handlerid.queue) return;

    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _handlerid.queue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        ASSERT2(false, "%" PRIu64, id);
        return;
    }

    MessageQueueContent& content = pos->second;

    for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end();) {
        if (_handlerid == (*it)->postid.reg && _title == (*it)->message.title) {
            delete(*it);
            it = content.lst_message.erase(it);
        } else {
            ++it;
        }
    }
}
    
const Message& RunningMessage() {
    MessageQueue_t id = (MessageQueue_t)ThreadUtil::currentthreadid();
    ScopedLock lock(sg_messagequeue_map_mutex);
    
    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) {
        return KNullMessage;
    }
    
    Message* runing_message = pos->second.lst_runloop_info.back().runing_message;
    return runing_message? *runing_message: KNullMessage;
}
    
MessagePost_t RunningMessageID() {
    MessageQueue_t id = (MessageQueue_t)ThreadUtil::currentthreadid();
    return RunningMessageID(id);
}

MessagePost_t RunningMessageID(const MessageQueue_t& _id) {
    ScopedLock lock(sg_messagequeue_map_mutex);

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(_id);
    if (sg_messagequeue_map.end() == pos) {
        return KNullPost;
    }

    MessageQueueContent& content = pos->second;
    return content.lst_runloop_info.back().runing_message_id;
}

static void __AsyncInvokeHandler(const MessagePost_t& _id, Message& _message) {
    (*boost::any_cast<boost::shared_ptr<AsyncInvokeFunction> >(_message.body1))();
}

MessageHandler_t InstallAsyncHandler(const MessageQueue_t& id) {
    ASSERT(0 != id);
    return InstallMessageHandler(__AsyncInvokeHandler, false, id);
}
    

static MessageQueue_t __CreateMessageQueueInfo(boost::shared_ptr<RunloopCond>& _breaker, thread_tid _tid) {
    ScopedLock lock(sg_messagequeue_map_mutex);

    MessageQueue_t id = (MessageQueue_t)_tid;

    if (sg_messagequeue_map.end() == sg_messagequeue_map.find(id)) {
        MessageQueueContent& content = sg_messagequeue_map[id];
        HandlerWrapper* handler = new HandlerWrapper(&__AsyncInvokeHandler, false, id, __MakeSeq());
        content.lst_handler.push_back(handler);
        content.invoke_reg = handler->reg;
        if (_breaker)
            content.breaker = _breaker;
        else
            content.breaker = boost::make_shared<Cond>();
    }

    return id;
}
    
static void __ReleaseMessageQueueInfo() {

    MessageQueue_t id = (MessageQueue_t)ThreadUtil::currentthreadid();

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() != pos) {
        MessageQueueContent& content = pos->second;

        for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
            delete(*it);
        }

        for (std::list<HandlerWrapper*>::iterator it = content.lst_handler.begin(); it != content.lst_handler.end(); ++it) {
            delete(*it);
        }

        sg_messagequeue_map.erase(id);
    }
}

    
const static int kMQCallANRId = 110;
const static long kWaitANRTimeout = 5 * 1000;
static void __ANRAssert(bool _iOS_style, const mars::comm::check_content& _content, MessageHandler_t _mq_id) {
    if(MessageQueue2TID(_mq_id.queue) == 0) {
        xwarn2(TSF"messagequeue already destroy, handler:(%_,%_)", _mq_id.queue, _mq_id.seq);
        return;
    }
    
    __ASSERT2(_content.file.c_str(), _content.line, _content.func.c_str(), "anr dead lock", "timeout:%d, tid:%" PRIu64 ", runing time:%" PRIu64 ", real time:%" PRIu64 ", used_cpu_time:%" PRIu64 ", iOS_style:%d",
              _content.timeout, _content.tid, clock_app_monotonic() - _content.start_time, gettickcount() - _content.start_tickcount, _content.used_cpu_time, _iOS_style);
#ifdef ANDROID
    __FATAL_ASSERT2(_content.file.c_str(), _content.line, _content.func.c_str(), "anr dead lock", "timeout:%d, tid:%" PRIu64 ", runing time:%" PRIu64 ", real time:%" PRIu64 ", used_cpu_time:%" PRIu64 ", iOS_style:%s",
                    _content.timeout, _content.tid, clock_app_monotonic() - _content.start_time, gettickcount() - _content.start_tickcount, _content.used_cpu_time, _iOS_style?"true":"false");
#endif
}
    

static void __ANRCheckCallback(bool _iOS_style, const mars::comm::check_content& _content) {
    if (kMQCallANRId != _content.call_id) {
        return;
    }
    
    MessageHandler_t mq_id = *((MessageHandler_t*)_content.extra_info);
    xinfo2(TSF"anr check content:%_, handler:(%_,%_)", _content.call_id, mq_id.queue, mq_id.seq);
    
    boost::shared_ptr<Thread> thread(new Thread(boost::bind(__ANRAssert, _iOS_style, _content, mq_id)));
    thread->start_after(kWaitANRTimeout);
    
    MessageQueue::AsyncInvoke([=]() {
        if (thread->isruning()) {
            xinfo2(TSF"misjudge anr, timeout:%_, tid:%_, runing time:%_, real time:%_, used_cpu_time:%_, handler:(%_,%_)", _content.timeout,
                   _content.tid, clock_app_monotonic() - _content.start_time, gettickcount() - _content.start_tickcount, _content.used_cpu_time, mq_id.queue, mq_id.seq);
            thread->cancel_after();
        }
    }, MessageQueue::DefAsyncInvokeHandler(mq_id.queue));
}
#ifndef ANR_CHECK_DISABLE

static void __RgisterANRCheckCallback() {
    GetSignalCheckHit().connect(5, boost::bind(&__ANRCheckCallback, _1, _2));
}
static void __UnregisterANRCheckCallback() {
    GetSignalCheckHit().disconnect(5);
}
    
BOOT_RUN_STARTUP(__RgisterANRCheckCallback);
BOOT_RUN_EXIT(__UnregisterANRCheckCallback);
#endif
void RunLoop::Run() {
    MessageQueue_t id = CurrentThreadMessageQueue();
    ASSERT(0 != id);
    {
        ScopedLock lock(sg_messagequeue_map_mutex);
        sg_messagequeue_map[id].lst_runloop_info.push_back(RunLoopInfo());
    }
    
    xinfo_function(TSF"messagequeue id:%_", id);

    while (true) {
        ScopedLock lock(sg_messagequeue_map_mutex);
        MessageQueueContent& content = sg_messagequeue_map[id];
        content.lst_runloop_info.back().runing_message_id = KNullPost;
        content.lst_runloop_info.back().runing_message = NULL;
        content.lst_runloop_info.back().runing_handler.clear();
        content.lst_runloop_info.back().runing_cond->notifyAll(lock);
        
        if (duty_func_) duty_func_();

        if ((content.breakflag || (breaker_func_ && breaker_func_()))) {
            content.lst_runloop_info.pop_back();
            if (content.lst_runloop_info.empty())
                __ReleaseMessageQueueInfo();
            break;
        }

        int64_t wait_time = 10 * 60 * 1000;
        MessageWrapper* messagewrapper = NULL;
        bool delmessage = true;

        for (std::list<MessageWrapper*>::iterator it = content.lst_message.begin(); it != content.lst_message.end(); ++it) {
            if (kImmediately == (*it)->timing.type) {
                messagewrapper = *it;
                content.lst_message.erase(it);
                break;
            } else if (kAfter == (*it)->timing.type) {
                int64_t time_cost = ::gettickspan((*it)->record_time);

                if ((*it)->timing.after <= time_cost) {
                    messagewrapper = *it;
                    content.lst_message.erase(it);
                    break;
                } else {
                    wait_time = std::min(wait_time, (*it)->timing.after - time_cost);
                }
            } else if (kPeriod == (*it)->timing.type) {
                if (kAfter == (*it)->periodstatus) {
                    int64_t time_cost = ::gettickspan((*it)->record_time);

                    if ((*it)->timing.after <= time_cost) {
                        messagewrapper = *it;
                        (*it)->record_time = ::gettickcount();
                        (*it)->periodstatus = kPeriod;
                        delmessage = false;
                        break;
                    } else {
                        wait_time = std::min(wait_time, (*it)->timing.after - time_cost);
                    }
                } else if (kPeriod == (*it)->periodstatus) {
                    int64_t time_cost = ::gettickspan((*it)->record_time);

                    if ((*it)->timing.period <= time_cost) {
                        messagewrapper = *it;
                        (*it)->record_time = ::gettickcount();
                        delmessage = false;
                        break;
                    } else {
                        wait_time = std::min(wait_time, (*it)->timing.period - time_cost);
                    }
                } else {
                    ASSERT(false);
                }
            } else {
                ASSERT(false);
            }
        }

        if (NULL == messagewrapper) {
            content.breaker->Wait(lock, (long)wait_time);
            continue;
        }

        std::list<HandlerWrapper> fit_handler;

        for (std::list<HandlerWrapper*>::iterator it = content.lst_handler.begin(); it != content.lst_handler.end(); ++it) {
            if (messagewrapper->postid.reg == (*it)->reg || ((*it)->recvbroadcast && messagewrapper->postid.reg.isbroadcast())) {
                fit_handler.push_back(**it);
                content.lst_runloop_info.back().runing_handler.push_back((*it)->reg);
            }
        }

        content.lst_runloop_info.back().runing_message_id = messagewrapper->postid;
        content.lst_runloop_info.back().runing_message = &messagewrapper->message;
        int64_t anr_timeout = messagewrapper->message.anr_timeout;
        lock.unlock();

        for (std::list<HandlerWrapper>::iterator it = fit_handler.begin(); it != fit_handler.end(); ++it) {
            SCOPE_ANR_AUTO((int)anr_timeout, kMQCallANRId, &(*it).reg);
            uint64_t timestart = ::clock_app_monotonic();
            (*it).handler(messagewrapper->postid, messagewrapper->message);
            uint64_t timeend = ::clock_app_monotonic();
#if defined(DEBUG) && defined(__APPLE__)

            if (!isDebuggerPerforming())
#endif
                ASSERT2(0 >= anr_timeout || anr_timeout >= (int64_t)(timeend - timestart), "anr_timeout:%" PRId64 " < cost:%" PRIu64", timestart:%" PRIu64", timeend:%" PRIu64, anr_timeout, timeend - timestart, timestart, timeend);
        }

        if (delmessage) {
            delete messagewrapper;
        }
    }
}

boost::shared_ptr<RunloopCond> RunloopCond::CurrentCond() {
    ScopedLock lock(sg_messagequeue_map_mutex);
    MessageQueue_t id = (MessageQueue_t)ThreadUtil::currentthreadid();

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() != pos) {
        MessageQueueContent& content = pos->second;
        return content.breaker;
    } else {
        return boost::shared_ptr<RunloopCond>();
    }
}

MessageQueueCreater::MessageQueueCreater(bool _iscreate, const char* _msg_queue_name)
    : MessageQueueCreater(boost::shared_ptr<RunloopCond>(), _iscreate, _msg_queue_name)
{}
    
MessageQueueCreater::MessageQueueCreater(boost::shared_ptr<RunloopCond> _breaker, bool _iscreate, const char* _msg_queue_name)
    : thread_(boost::bind(&MessageQueueCreater::__ThreadRunloop, this), _msg_queue_name)
	, messagequeue_id_(KInvalidQueueID), breaker_(_breaker) {
	if (_iscreate)
		CreateMessageQueue();
}

MessageQueueCreater::~MessageQueueCreater() {
    CancelAndWait();
}

void MessageQueueCreater::__ThreadRunloop() {
    ScopedLock lock(messagequeue_mutex_);
    lock.unlock();
    
    RunLoop().Run();
    
}

MessageQueue_t MessageQueueCreater::GetMessageQueue() {
    return messagequeue_id_;
}

MessageQueue_t MessageQueueCreater::CreateMessageQueue() {
    ScopedLock lock(messagequeue_mutex_);

    if (thread_.isruning()) return messagequeue_id_;

    if (0 != thread_.start()) { return KInvalidQueueID;}
    messagequeue_id_ = __CreateMessageQueueInfo(breaker_, thread_.tid());
    xinfo2(TSF"create messageqeue id:%_", messagequeue_id_);
    
    return messagequeue_id_;
}

void MessageQueueCreater::CancelAndWait() {
    ScopedLock lock(messagequeue_mutex_);

    if (KInvalidQueueID == messagequeue_id_) return;
    
    BreakMessageQueueRunloop(messagequeue_id_);
    messagequeue_id_ = KInvalidQueueID;
    lock.unlock();
    if(ThreadUtil::currentthreadid() != thread_.tid()) {
        thread_.join();
    }
}

MessageQueue_t MessageQueueCreater::CreateNewMessageQueue(boost::shared_ptr<RunloopCond> _breaker, thread_tid _tid) {
    return(__CreateMessageQueueInfo(_breaker, _tid));
}

MessageQueue_t MessageQueueCreater::CreateNewMessageQueue(boost::shared_ptr<RunloopCond> _breaker, const char* _messagequeue_name) {
    
    SpinLock* sp = new SpinLock;
    Thread thread(boost::bind(&__ThreadNewRunloop, sp), _messagequeue_name, true);
//    thread.outside_join();
    ScopedSpinLock lock(*sp);

    if (0 != thread.start()) {
        delete sp;
        return KInvalidQueueID;
    }

    MessageQueue_t id = __CreateMessageQueueInfo(_breaker, thread.tid());
    return id;
}
    
MessageQueue_t MessageQueueCreater::CreateNewMessageQueue(const char* _messagequeue_name) {
    return CreateNewMessageQueue(boost::shared_ptr<RunloopCond>(), _messagequeue_name);
}

void MessageQueueCreater::ReleaseNewMessageQueue(MessageQueue_t _messagequeue_id) {

	if (KInvalidQueueID == _messagequeue_id) return;

	BreakMessageQueueRunloop(_messagequeue_id);
	WaitForRunningLockEnd(_messagequeue_id);
	ThreadUtil::join((thread_tid)_messagequeue_id);
}

void MessageQueueCreater::__ThreadNewRunloop(SpinLock* _sp) {
    ScopedSpinLock lock(*_sp);
    lock.unlock();
    delete _sp;

    RunLoop().Run();
}

MessageQueue_t GetDefMessageQueue() {
    static MessageQueueCreater* s_defmessagequeue = new MessageQueueCreater;
    return s_defmessagequeue->CreateMessageQueue();
}

MessageQueue_t GetDefTaskQueue() {
    static MessageQueueCreater* s_deftaskqueue = new MessageQueueCreater;
    return s_deftaskqueue->CreateMessageQueue();
}

MessageHandler_t DefAsyncInvokeHandler(const MessageQueue_t& _messagequeue) {
    ScopedLock lock(sg_messagequeue_map_mutex);
    const MessageQueue_t& id = _messagequeue;

    std::map<MessageQueue_t, MessageQueueContent>::iterator pos = sg_messagequeue_map.find(id);
    if (sg_messagequeue_map.end() == pos) return KNullHandler;

    MessageQueueContent& content = pos->second;
    return content.invoke_reg;
}

ScopeRegister::ScopeRegister(const MessageHandler_t& _reg)
: m_reg(new MessageHandler_t(_reg)) {}

ScopeRegister::~ScopeRegister() {
    Cancel();
    delete m_reg;
}

const MessageHandler_t& ScopeRegister::Get() const
{return *m_reg;}

void ScopeRegister::Cancel() const {
    UnInstallMessageHandler(*m_reg);
    CancelMessage(*m_reg);
}
void ScopeRegister::CancelAndWait() const {
    Cancel();
    WaitForRunningLockEnd(*m_reg);
}
}  // namespace MessageQueue
