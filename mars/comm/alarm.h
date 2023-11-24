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
 * alarm.h
 *
 *  Created on: 2012-8-27
 *      Author: yerungui
 */

#ifndef COMM_ALARM_H_
#define COMM_ALARM_H_

#include <boost/bind.hpp>

#include "comm/xlogger/xlogger.h"
#include "messagequeue/message_queue.h"

#ifdef ANDROID
#include "android/wakeuplock.h"

const static char* const kKeyAlarmStartWakeupLook = "AlarmStart";
const static unsigned int kAlarmStartWakeupLook = 1000;

const static char* const kKeyAlarmOnWakeupLook = "AlarmOn";
const static unsigned int kAlarmOnWakeupLook = 500;

static int wakelock_on_alarm_ = kAlarmOnWakeupLook;
static int wakelock_start_alarm_ = kAlarmStartWakeupLook;
#endif

namespace mars {
namespace comm {

class Alarm {
 public:
    enum {
        kInit,
        kStart,
        kCancel,
        kOnAlarm,
    };

 public:
    template <class T>
    explicit Alarm(const T& _op, bool _inthread = true)
    : target_(detail::transform(_op))
    , reg_async_(MessageQueue::InstallAsyncHandler(MessageQueue::GetDefMessageQueue()))
    , broadcast_msg_id_(MessageQueue::KNullPost)
    , runthread_(boost::bind(&Alarm::__Run, this), "alarm")
    , inthread_(_inthread)
    , seq_(0)
    , status_(kInit)
    , after_(0)
    , starttime_(0)
    , endtime_(0)
    , reg_(MessageQueue::InstallMessageHandler(boost::bind(&Alarm::OnAlarm, this, _1, _2), true))
#ifdef ANDROID
    , wakelock_(NULL)
    , type_(-1)
#endif
    {
        xinfo2(TSF "handler:(%_,%_)", reg_async_.Get().queue, reg_async_.Get().seq);
    }

    template <class T>
    explicit Alarm(const T& _op, const MessageQueue::MessageQueue_t& _id)
    : target_(detail::transform(_op))
    , reg_async_(MessageQueue::InstallAsyncHandler(_id))
    , broadcast_msg_id_(MessageQueue::KNullPost)
    , runthread_(boost::bind(&Alarm::__Run, this), "alarm")
    , inthread_(false)
    , seq_(0)
    , status_(kInit)
    , after_(0)
    , starttime_(0)
    , endtime_(0)
    , reg_(MessageQueue::InstallMessageHandler(boost::bind(&Alarm::OnAlarm, this, _1, _2), true))
#ifdef ANDROID
    , wakelock_(NULL)
    , type_(-1)
#endif
    {
        xinfo2(TSF "handler:(%_,%_)", reg_async_.Get().queue, reg_async_.Get().seq);
    }

    virtual ~Alarm() {
        Cancel();
        reg_.CancelAndWait();
        reg_async_.CancelAndWait();
        runthread_.join();
        delete target_;
#ifdef ANDROID
        delete wakelock_;
#endif
    }

#ifdef ANDROID
    static void onAlarmImpl(int64_t _id);
    void SetType(int _type) {
        type_ = _type;
    }
    static void __StartWakeLock();
    static void SetStartAlarmWakeLock(int time);
    static void SetOnAlarmWakeLock(int time);
#endif

    bool Start(int _after, bool _needWake = true);  // ms
    bool Cancel();

    bool IsWaiting() const;
    int Status() const;
    int After() const;
    int64_t ElapseTime() const;

    const Thread& RunThread() const;

 private:
    Alarm(const Alarm&);
    Alarm& operator=(const Alarm&);

    void OnAlarm(const MessageQueue::MessagePost_t& _id, MessageQueue::Message& _message);
    virtual void __Run();

 private:
    Runnable* target_;
    MessageQueue::ScopeRegister reg_async_;
    MessageQueue::MessagePost_t broadcast_msg_id_;
    Thread runthread_;
    bool inthread_;

    int64_t seq_;
    int status_;

    int after_;
    uint64_t starttime_;
    uint64_t endtime_;

    MessageQueue::ScopeRegister reg_;
#ifdef ANDROID
    WakeUpLock* wakelock_;
    int type_;
#endif
};

}  // namespace comm
}  // namespace mars
#endif /* COMM_ALARM_H_ */
