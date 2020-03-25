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
 * MessageQueue.h
 *
 *  Created on: 2013-4-3
 *      Author: yerungui
 */

#ifndef MESSAGEQUEUE_H_
#define MESSAGEQUEUE_H_

#include <string.h>
#include <string>

#include "boost/function.hpp"
#include "boost/any.hpp"
#include "boost/smart_ptr.hpp"

#if UWP //???andrewu temp compile pass
#include "boost/utility/result_of.hpp"
#endif

#if __cplusplus >= 201103L
#include "boost/static_assert.hpp"
#include "boost/utility/result_of.hpp"
#include "boost/type_traits/is_same.hpp"
#else 
#ifdef WIN32
#include "boost/utility/result_of.hpp"
#endif // DEBUG
#endif 

#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/strutil.h"
namespace MessageQueue {

typedef uint64_t MessageQueue_t;
typedef boost::function<void ()> AsyncInvokeFunction;

const MessageQueue_t KInvalidQueueID = 0;

struct MessageHandler_t {
    MessageHandler_t(): queue(KInvalidQueueID), seq(0) {}
    bool operator == (const MessageHandler_t& _rhs) const {return queue == _rhs.queue && seq == _rhs.seq;}
    bool operator!=(const MessageHandler_t& _rhs) const {return !operator==(_rhs);}
    bool isbroadcast() const {return 0 == seq;}
    MessageQueue_t queue;
    unsigned int seq;
};

struct MessagePost_t {
    MessagePost_t(): seq(0) {}
    bool operator == (const MessagePost_t& _rhs) const {return reg == _rhs.reg && seq == _rhs.seq;}
    bool operator!=(const MessagePost_t& _rhs) const {return !operator==(_rhs);}
    MessageHandler_t reg;
    unsigned int seq;
    
    std::string ToString() const {
        XMessage xmsg;
        xmsg(TSF"(seq:%_, handler:%_, queue:%_)", seq, reg.seq, reg.queue);
        return xmsg.String();
    }
};

struct MessageTitle_t {
    MessageTitle_t(): title(0) {}
    template<typename T> MessageTitle_t(const T& _title): title((uintptr_t)_title) { BOOST_STATIC_ASSERT(sizeof(T) <= sizeof(uintptr_t));}

    bool operator == (const MessageTitle_t& _rhs) const { return title == _rhs.title;}
    bool operator!=(const MessageTitle_t& _rhs) const { return !operator==(_rhs);}
    uintptr_t title;
};


struct Message {
    Message(): title(0), anr_timeout(0), create_time(0), execute_time(0) {}
    Message(const MessageTitle_t& _title, const boost::any& _body1, const boost::any& _body2, const std::string& _name = "")
    : title(_title), body1(_body1), body2(_body2), anr_timeout(10*60*1000), msg_name(_name), create_time(::gettickcount()),
    execute_time(0){}
    
    template <class F>
    Message(const MessageTitle_t& _title, const F& _func, const std::string& _name = "")
    : title(_title), body1(boost::make_shared<AsyncInvokeFunction>()), body2(), anr_timeout(10*60*1000), msg_name(_name), create_time(::gettickcount()), execute_time(0) {
        if (msg_name.empty()){
            const char* funcname = typeid(_func).name();
            if (funcname != nullptr){
                msg_name = funcname;
            }
        }
        *boost::any_cast<boost::shared_ptr<AsyncInvokeFunction> >(body1) = _func;
    }
    
    
    bool operator == (const Message& _rhs) const {return title == _rhs.title;}

    std::string ToString() const {
        XMessage xmsg;
        xmsg(TSF"(msg_name:%_, create_time:%_, execute_time:%_)", msg_name, create_time, execute_time);
        return xmsg.String();
    }
    
    MessageTitle_t  title;
    boost::any      body1;
    boost::any      body2;
    int64_t         anr_timeout;
    
    std::string     msg_name;
    uint64_t        create_time;
    uint64_t        execute_time;
};

    
const char* const TMessageTimingString[] = {
    "kAfter",
    "kPeriod",
    "kImmediately"
};
struct MessageTiming {
    
    enum TMessageTiming {
        kAfter,
        kPeriod,
        kImmediately,
    };
    
    MessageTiming(TMessageTiming _timing, int64_t _after, int64_t _period)
        : type(_timing)
        , after(_after)
        , period(_period)
    {}

    MessageTiming(int64_t _after, int64_t _period)
    : type(TMessageTiming::kPeriod)
        , after(_after)
        , period(_period)
    {}

    MessageTiming(int64_t _after)
    : type(TMessageTiming::kAfter)
        , after(_after)
        , period(0)
    {}

    MessageTiming()
    : type(TMessageTiming::kImmediately)
        , after(0)
        , period(0)
    {}

    std::string ToString() const {
        XMessage xmsg;
        xmsg(TSF"(type:%_, after:%_, period:%_)", TMessageTimingString[type], after, period);
        return xmsg.String();
    }
    
    TMessageTiming type;
    int64_t after;
    int64_t period;
};
    
using TMessageTiming = MessageTiming::TMessageTiming;
const TMessageTiming kAfter = MessageTiming::TMessageTiming::kAfter;
const TMessageTiming kPeriod = MessageTiming::TMessageTiming::kPeriod;
const TMessageTiming kImmediately = MessageTiming::TMessageTiming::kImmediately;


typedef boost::function<void (const MessagePost_t& _id, Message& _message)> MessageHandler;

const MessageTiming     KDefTiming;
const MessageHandler_t  KNullHandler;
const MessagePost_t     KNullPost;
const Message           KNullMessage;

inline const MessageHandler_t& Post2Handler(const MessagePost_t& _postid) { return _postid.reg;}
inline const MessageQueue_t& Handler2Queue(const MessageHandler_t& _handler) { return _handler.queue;}
    
MessageQueue_t CurrentThreadMessageQueue();
MessageQueue_t TID2MessageQueue(thread_tid _tid);
thread_tid     MessageQueue2TID(MessageQueue_t _id);

const Message& RunningMessage();
    
MessagePost_t    RunningMessageID();
MessagePost_t    RunningMessageID(const MessageQueue_t& _id);
MessageQueue_t   GetDefMessageQueue();
MessageQueue_t   GetDefTaskQueue();
MessageHandler_t DefAsyncInvokeHandler(const MessageQueue_t& _messagequeue = CurrentThreadMessageQueue());

void WaitForRunningLockEnd(const MessagePost_t&  _message);
void WaitForRunningLockEnd(const MessageHandler_t&  _handler);
void WaitForRunningLockEnd(const MessageQueue_t&  _messagequeueid);
void BreakMessageQueueRunloop(const MessageQueue_t&  _messagequeueid);

MessageHandler_t InstallMessageHandler(const MessageHandler& _handler, bool _recvbroadcast = false, const MessageQueue_t& _messagequeueid = GetDefMessageQueue());
void UnInstallMessageHandler(const MessageHandler_t& _handlerid);

MessagePost_t PostMessage(const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing = KDefTiming);
MessagePost_t SingletonMessage(bool _replace, const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing = KDefTiming);
MessagePost_t BroadcastMessage(const MessageQueue_t& _messagequeueid,  const Message& _message, const MessageTiming& _timing = KDefTiming);
MessagePost_t FasterMessage(const MessageHandler_t& _handlerid, const Message& _message, const MessageTiming& _timing = KDefTiming);

bool WaitMessage(const MessagePost_t& _message, long _timeoutInMs = -1);
bool FoundMessage(const MessagePost_t& _message);

bool CancelMessage(const MessagePost_t& _postid);
void CancelMessage(const MessageHandler_t& _handlerid);
void CancelMessage(const MessageHandler_t& _handlerid, const MessageTitle_t& _title);

std::string DumpMQ(const MessageQueue_t& _msq_queue_id);
//AsyncInvoke
MessageHandler_t InstallAsyncHandler(const MessageQueue_t& id);

//---with message name
    //no title
template<class F>
    MessagePost_t AsyncInvoke(const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(0, _func, _msg_name));
}
template<class F>
MessagePost_t  AsyncInvokeAfter(int64_t _after, const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(0, _func, _msg_name), MessageTiming(kAfter, _after, 0));
}
template<class F>
MessagePost_t  AsyncInvokePeriod(int64_t _after, int64_t _period, const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(0, _func, _msg_name), MessageTiming(kPeriod, _after, _period));
}
    //~no title
    //title
template<class F>
MessagePost_t  AsyncInvoke(const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(_title, _func, _msg_name));
}

template<class F>
MessagePost_t  AsyncInvokeAfter(int64_t _after, const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(_title, _func, _msg_name), MessageTiming(kAfter, _after, 0));
}

template<class F>
MessagePost_t  AsyncInvokePeriod(int64_t _after, int64_t _period, const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name = "default_name") {
    return PostMessage(_handlerid, Message(_title, _func, _msg_name), MessageTiming(kPeriod, _after, _period));
}
    //~title
//---~with message name
/*
template<class F>
MessagePost_t AsyncInvoke(const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(0, _func));
}

template<class F>
MessagePost_t  AsyncInvokeAfter(int64_t _after, const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(0, _func), MessageTiming(kAfter, _after, 0));
}

template<class F>
MessagePost_t  AsyncInvokePeriod(int64_t _after, int64_t _period, const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(0, _func), MessageTiming(kPeriod, _after, _period));
}

template<class F>
MessagePost_t  AsyncInvoke(const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(_title, _func));
}

template<class F>
MessagePost_t  AsyncInvokeAfter(int64_t _after, const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(_title, _func), MessageTiming(kAfter, _after, 0));
}

template<class F>
MessagePost_t  AsyncInvokePeriod(int64_t _after, int64_t _period, const F& _func, const MessageTitle_t& _title, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler()) {
    return PostMessage(_handlerid, Message(_title, _func), MessageTiming(kPeriod, _after, _period));
}
    */
class RunLoop {
 public:
    template<typename B>
    RunLoop(const B& _breaker_func):breaker_func_(_breaker_func) {}
    template<typename B, typename D>
    RunLoop(const B& _breaker_func, const D& _duty_func):breaker_func_(_breaker_func), duty_func_(_duty_func){}
    RunLoop() {}
    
 public:
    void Run();
    
 private:
     RunLoop(const RunLoop&);
     RunLoop& operator=(const RunLoop&);
    
 private:
    boost::function<bool ()> breaker_func_;
    boost::function<void ()> duty_func_;
};
    
class RunloopCond {
public:
    RunloopCond() {};
    virtual ~RunloopCond() {};
    
    static boost::shared_ptr<RunloopCond> CurrentCond();
    
public:
    virtual const boost::typeindex::type_info& type() const = 0;
    virtual void  Wait(ScopedLock& _lock, long _millisecond) = 0;
    virtual void  Notify(ScopedLock& _lock) = 0;
    
private:
    RunloopCond(const RunloopCond&);
    void operator=(const RunloopCond&);
};
    
    
class MessageQueueCreater {
  public:
    MessageQueueCreater(bool _iscreate = false, const char* _msg_queue_name = NULL);
    MessageQueueCreater(boost::shared_ptr<RunloopCond> _breaker, bool _iscreate = false, const char* _msg_queue_name = NULL);
    ~MessageQueueCreater();

    MessageQueue_t GetMessageQueue();
    MessageQueue_t CreateMessageQueue();
    void CancelAndWait();

    static MessageQueue_t CreateNewMessageQueue(const char* _messagequeue_name = NULL);
    static MessageQueue_t CreateNewMessageQueue(boost::shared_ptr<RunloopCond> _breaker, const char* _messagequeue_name = NULL);
    static MessageQueue_t CreateNewMessageQueue(boost::shared_ptr<RunloopCond> _breaker, thread_tid _tid);
    static void ReleaseNewMessageQueue(MessageQueue_t _messagequeue_id); // block api

  private:
    MessageQueueCreater(const MessageQueueCreater&);
    MessageQueueCreater& operator=(const MessageQueueCreater&);

    void __ThreadRunloop();
    static void __ThreadNewRunloop(SpinLock* _sp);

  private:
    Thread                              thread_;
    Mutex                               messagequeue_mutex_;
    MessageQueue_t                      messagequeue_id_;
    boost::shared_ptr<RunloopCond>      breaker_;
};

template <typename R>
class AsyncResult {
  private:
    struct AsyncResultWrapper {
        AsyncResultWrapper(): result_holder(new R), result_valid(false), result(result_holder) {}
        AsyncResultWrapper(R* _result): result_holder(NULL), result_valid(false), result(_result) {}
        ~AsyncResultWrapper() {
            if (!result_valid && callback_function)
                callback_function(*result, false);
            
            if (result_holder) delete result_holder;
        }
        
        R*  result_holder;

        boost::function<R()> invoke_function;
        boost::function<void (const R&, bool)> callback_function;

        bool result_valid;
        R* result;
    };

  public:
    template<typename T>
    AsyncResult(const T& _func)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R>::value);
#endif
        wrapper_->invoke_function = _func;
    }

    template<typename T>
    AsyncResult(const T& _func, R* _result_holder)
        : wrapper_(new AsyncResultWrapper(_result_holder)) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R>::value);
#endif
        wrapper_->invoke_function = _func;
    }

    template<typename T, typename C>
    AsyncResult(const T& _func, const C& _callback)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R>::value);
#endif
        wrapper_->invoke_function = _func;
        wrapper_->callback_function = _callback;
    }

    template<typename T, typename C>
    AsyncResult(const T& _func, const C& _callback, R* _result_holder)
        : wrapper_(new AsyncResultWrapper(_result_holder)) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R>::value);
#endif
        wrapper_->invoke_function = _func;
        wrapper_->callback_function = _callback;
    }

    void operator()() const { Invoke(wrapper_->invoke_function()); }
    R& Result() const { return *(wrapper_->result);}
    operator bool() const { return wrapper_->result_valid;}

  private:
    void Invoke(const R& _result) const {
        if (wrapper_->result) *(wrapper_->result) = _result;
        wrapper_->result_valid = true;
        
        if (wrapper_->callback_function) wrapper_->callback_function(_result, true);
    }
    
  private:
    AsyncResult& operator=(const AsyncResult&);
    // AsyncResult(const AsyncResult& _ref);

  private:
    boost::shared_ptr<AsyncResultWrapper> wrapper_;
};

template <>
class AsyncResult<void> {
  private:
    struct AsyncResultWrapper {
        AsyncResultWrapper(): result_valid(false) {}
        ~AsyncResultWrapper() {
            if (!result_valid && callback_function)
                callback_function(false);
        }

        boost::function<void ()> invoke_function;
        boost::function<void (bool)> callback_function;
        bool result_valid;
    };

  public:
    template<typename T>
    AsyncResult(const T& _func, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, void>::value);
#endif
        wrapper_->invoke_function = _func;
    }

    template<typename T, typename C>
    AsyncResult(const T& _func, const C& _callback, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, void>::value);
#endif
        wrapper_->invoke_function = _func;
        wrapper_->callback_function = _callback;
    }

    void operator()() const {
        wrapper_->invoke_function();
        wrapper_->result_valid = true;

        if (wrapper_->callback_function)
            wrapper_->callback_function(true);
    }

    void Result() const {}
    operator bool() const { return wrapper_->result_valid;}

  private:
    AsyncResult& operator=(const AsyncResult&);
    // AsyncResult(const AsyncResult& _ref);

  private:
    boost::shared_ptr<AsyncResultWrapper> wrapper_;
};

template <typename R>
class AsyncResult <R&> {
  private:
    struct AsyncResultWrapper {
        AsyncResultWrapper(): result_valid(false), result(NULL) {}
        ~AsyncResultWrapper() {
            if (!result_valid && callback_function)
                callback_function(*result, false);
        }

        boost::function<R& ()> invoke_function;
        boost::function<void (R&, bool)> callback_function;

        bool result_valid;
        R* result;
    };

  public:
    template<typename T>
    AsyncResult(const T& _func, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R&>::value);
#endif
        wrapper_->invoke_function = _func;
    }

    template<typename T, typename C>
    AsyncResult(const T& _func, const C& _callback, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, R&>::value);
#endif
        wrapper_->invoke_function = _func;
        wrapper_->callback_function = _callback;
    }

    void operator()() const {
        wrapper_->result = & (wrapper_->invoke_function());
        wrapper_->result_valid = true;

        if (wrapper_->callback_function)
            wrapper_->callback_function(Result(), true);
    }

    R& Result() const { return *(wrapper_->result);}
    operator bool() const { return wrapper_->result_valid;}

  private:
    AsyncResult& operator=(const AsyncResult&);
    // AsyncResult(const AsyncResult& _ref);

  private:
    boost::shared_ptr<AsyncResultWrapper> wrapper_;
};

template <typename R>
class AsyncResult <const R&> {
  private:
    struct AsyncResultWrapper {
        AsyncResultWrapper(): result_valid(false), result(NULL) {}
        ~AsyncResultWrapper() {
            if (!result_valid && callback_function)
                callback_function(*result, false);
        }

        boost::function<const R& ()> invoke_function;
        boost::function<void (const R&, bool)> callback_function;

        bool result_valid;
        const R* result;
    };

  public:
    template<typename T>
    AsyncResult(const T& _func, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, const R&>::value);
#endif
        wrapper_->invoke_function = _func;
    }

    template<typename T, typename C>
    AsyncResult(const T& _func, const C& _callback, const void* _place_holder = NULL)
        : wrapper_(new AsyncResultWrapper()) {
#if __cplusplus >= 201103L
        BOOST_STATIC_ASSERT(boost::is_same<typename boost::result_of<T()>::type, const R&>::value);
#endif
        wrapper_->invoke_function = _func;
        wrapper_->callback_function = _callback;
    }

    void operator()() const {
        wrapper_->result = & (wrapper_->invoke_function());
        wrapper_->result_valid = true;

        if (wrapper_->callback_function)
            wrapper_->callback_function(Result(), true);
    }

    const R& Result() const { return *(wrapper_->result);}
    operator bool() const { return wrapper_->result_valid;}

  private:
    AsyncResult& operator=(const AsyncResult&);
    // AsyncResult(const AsyncResult& _ref);

  private:
    boost::shared_ptr<AsyncResultWrapper> wrapper_;
};

template <typename R>
    R& WaitInvoke(const AsyncResult<R>& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name="default_name") {
    
    if (CurrentThreadMessageQueue() == Handler2Queue(_handlerid)) {
        _func();
        return _func.Result();
    } else {
        WaitMessage(AsyncInvoke(_func, _handlerid, _msg_name));
        return _func.Result();
    }
}
    
template <typename F>
typename boost::result_of< F()>::type  WaitInvoke(const F& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name="default_name") {
    
    if (CurrentThreadMessageQueue() == Handler2Queue(_handlerid)) {
        return _func();
    } else {
        
        typedef typename boost::result_of<F()>::type R;
        MessageQueue::AsyncResult<R> result(_func);
        
        WaitMessage(AsyncInvoke(result, _handlerid, _msg_name));
        return result.Result();
    }
}
    
template <typename F, typename R>
    R  WaitInvoke(const F& _func, R _ret, long _timeout = -1, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name="default_name") {
    
    if (CurrentThreadMessageQueue() == Handler2Queue(_handlerid)) {
        return _func();
    } else {
        // typedef typename boost::result_of<F()>::type R;
        MessageQueue::AsyncResult<R> result(_func);
        
        bool hasRun = WaitMessage(AsyncInvoke(result, _handlerid, _msg_name), _timeout);
        if(hasRun)
            return result.Result();
        else
            return _ret;
    }
}

template <typename R>
MessagePost_t  AsyncInvoke(const AsyncResult<R>& _func, const MessageHandler_t& _handlerid = DefAsyncInvokeHandler(), const std::string& _msg_name="default_name") {
    return PostMessage(_handlerid, Message(0, _func, _msg_name));
}
    
class ScopeRegister {
public:
    ScopeRegister(const MessageHandler_t& _reg);
    ~ScopeRegister();
    
    const MessageHandler_t& Get() const;
    void Cancel() const;
    void CancelAndWait() const;
    
private:
    ScopeRegister(const ScopeRegister&);
    ScopeRegister& operator=(const ScopeRegister&);
    
private:
    MessageHandler_t* m_reg;
};

//------
    
#define MESSAGE_NAME(file, function) (strutil::GetFileNameFromPath(file)+":"+function)
#define ASYNC_BLOCK_END_MSGNAME(msg_name)  }, AYNC_HANDLER, msg_name);

#define ASYNC_BLOCK_START MessageQueue::AsyncInvoke([=] () {
#define ASYNC_BLOCK_END ASYNC_BLOCK_END_MSGNAME(MESSAGE_NAME(__FILE__, __FUNCTION__))


//------
#define SYNC2ASYNC_FUNC_MSGNAME(func, msg_name) \
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{ MessageQueue::AsyncInvoke(func, AYNC_HANDLER, msg_name); return; } \

#define SYNC2ASYNC_FUNC(func) SYNC2ASYNC_FUNC_MSGNAME(func, MESSAGE_NAME(__FILE__, __FUNCTION__))

//------
#define RETURN_SYNC2ASYNC_FUNC_MSGNAME(func, ret, msg_name) \
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{ MessageQueue::AsyncInvoke(func, AYNC_HANDLER, msg_name); return ret; } \

    /*
     * sync to async, will not wait, but immediately return ret
     */
#define RETURN_SYNC2ASYNC_FUNC(func, ret) RETURN_SYNC2ASYNC_FUNC_MSGNAME(func, ret, MESSAGE_NAME(__FILE__, __FUNCTION__))
   
    
//------
#define RETURN_SYNC2ASYNC_FUNC_TITLE_MSGNAME(func, title, ret, msg_name) \
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{ MessageQueue::AsyncInvoke(func, title, AYNC_HANDLER, msg_name); return ret; } \

#define RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, ret) RETURN_SYNC2ASYNC_FUNC_TITLE_MSGNAME(func, title, ret, MESSAGE_NAME(__FILE__, __FUNCTION__))

//------
#define RETURN_WAIT_SYNC2ASYNC_FUNC_MSGNAME(func, ret, msg_name) \
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{ MessageQueue::MessagePost_t postId = MessageQueue::AsyncInvoke(func, AYNC_HANDLER, msg_name);MessageQueue::WaitMessage(postId); return ret; } \

    /*
     * sync to async, wait utils async function end
     * when wait ends, will return ret
     */
#define RETURN_WAIT_SYNC2ASYNC_FUNC(func, ret) RETURN_WAIT_SYNC2ASYNC_FUNC_MSGNAME(func, ret, MESSAGE_NAME(__FILE__, __FUNCTION__))
    
//------
#define WAIT_SYNC2ASYNC_FUNC_MSGNAME(func, msg_name) \
\
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{\
return MessageQueue::WaitInvoke(func, AYNC_HANDLER, msg_name);\
}

    /*
     * sync to async, wait utils async function end
     */
#define WAIT_SYNC2ASYNC_FUNC(func) WAIT_SYNC2ASYNC_FUNC_MSGNAME(func, MESSAGE_NAME(__FILE__, __FUNCTION__))
    
//-------

#define WAIT_SYNC2ASYNC_FUNC_MSGNAME_WITH_HOOK(func, pre_lambda, post_lambda, msg_name, ret, timeout) \
\
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{\
pre_lambda();\
auto r = MessageQueue::WaitInvoke(func, ret, timeout, AYNC_HANDLER, msg_name);\
return post_lambda(r);\
}
    
    /*
     * sync to async, wait utils async function end
     * pre_lambda: function will be called before transform to async, will only be called in caller's thread
     */
#define WAIT_SYNC2ASYNC_FUNC_WITH_HOOK(func, pre_lambda, post_lambda, ret, timeout) WAIT_SYNC2ASYNC_FUNC_MSGNAME_WITH_HOOK(func, pre_lambda, post_lambda, MESSAGE_NAME(__FILE__, __FUNCTION__), ret, timeout)
    
    
#define WAIT_SYNC2ASYNC_FUNC_MSGNAME_WITH_PREHOOK(func, pre_lambda, msg_name) \
\
if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
{\
pre_lambda();\
return MessageQueue::WaitInvoke(func, AYNC_HANDLER, msg_name);\
}

    /*
     * sync to async, wait utils async function end
     * pre_lambda: function will be called before transform to async, will only be called in caller's thread
     */
#define WAIT_SYNC2ASYNC_FUNC_WITH_PREHOOK(func, pre_lambda) WAIT_SYNC2ASYNC_FUNC_MSGNAME_WITH_PREHOOK(func, pre_lambda, MESSAGE_NAME(__FILE__, __FUNCTION__))


// define AYNC_HANDLER in source file
//#define AYNC_HANDLER handler
    
} namespace mq = MessageQueue; //namespace MessageQueue

#endif /* MESSAGEQUEUE_H_ */
