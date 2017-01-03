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
 * MessageQueueUtils.h
 *
 *  Created on: 2013-4-8
 *      Author: yerungui
 */

#ifndef MESSAGEQUEUEUTIL_H_
#define MESSAGEQUEUEUTIL_H_


namespace MessageQueue {

struct MessageHandler_t;

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

#define SCOPEUTIL(classname, type, invoke) \
    class classname\
    {\
      public:\
        classname(){}\
        ~classname()\
        {\
            for (std::list<type>::iterator it = m_lst.begin(); it != m_lst.end(); ++it)\
            {\
                invoke(*it);\
            }\
        }\
        \
        void operator+=(const type&  _val) { m_lst.push_back(_val); }\
        void operator-=(const type&  _val) { m_lst.remove(_val); }\
        \
      private:\
        classname(const classname&);\
        classname& operator=(const classname&);\
        \
      private:\
        std::list<type> m_lst;\
    }

}
// SCOPEUTIL(ScopeRegister, MessageRegister_t, UnRegisterMessageHandler);
// SCOPEUTIL(ScopePoster, MessagePost_t, CancelMessage);

#define ASYNC_BLOCK_START MessageQueue::AsyncInvoke([=] () {
#define ASYNC_BLOCK_END }, AYNC_HANDLER);

#define SYNC2ASYNC_FUNC(func) \
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
    { MessageQueue::AsyncInvoke(func, AYNC_HANDLER); return; } \

#define RETURN_SYNC2ASYNC_FUNC(func, ret) \
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
    { MessageQueue::AsyncInvoke(func, AYNC_HANDLER); return ret; } \

#define RETURN_SYNC2ASYNC_FUNC_TITLE(func, title, ret) \
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
    { MessageQueue::AsyncInvoke(func, title, AYNC_HANDLER); return ret; } \

#define RETURN_WAIT_SYNC2ASYNC_FUNC(func, ret) \
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
    { MessageQueue::MessagePost_t postId = MessageQueue::AsyncInvoke(func, AYNC_HANDLER);MessageQueue::WaitMessage(postId); return ret; } \

#define WAIT_SYNC2ASYNC_FUNC(func) \
    \
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(AYNC_HANDLER)) \
    {\
        return MessageQueue::WaitInvoke(func, AYNC_HANDLER);\
    }

// define AYNC_HANDLER in source file
//#define AYNC_HANDLER handler

#endif
