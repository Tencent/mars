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
 * alarm.cpp
 *
 *  Created on: 2012-8-27
 *      Author: yerungui
 */

#include "comm/alarm.h"

#include "comm/assert/__assert.h"
#include "comm/thread/lock.h"
#include "comm/time_utils.h"
#include "comm/xlogger/xlogger.h"

#include "comm/platform_comm.h"

static Mutex sg_lock;
static int64_t sg_seq = 1;
static const MessageQueue::MessageTitle_t KALARM_MESSAGETITLE(0x1F1FF);

#define MAX_LOCK_TIME (5000)
#define INVAILD_SEQ (0)

bool Alarm::Start(int _after) {
    ScopedLock lock(sg_lock);

    if (INVAILD_SEQ != seq_) return false;

    if (INVAILD_SEQ == sg_seq) sg_seq = 1;

    int64_t seq = sg_seq++;
    uint64_t starttime = gettickcount();
    MessageQueue::MessagePost_t mqId = MessageQueue::BroadcastMessage(MessageQueue::GetDefMessageQueue(), MessageQueue::Message(KALARM_MESSAGETITLE, (int64_t)seq, 1), MessageQueue::MessageTiming(_after));

    if (MessageQueue::KNullPost == mqId) {
        xerror2(TSF"mq alarm return null post, id:%0, after:%1, seq:%2", (uintptr_t)this, _after, seq);
        return false;
    }

#ifdef ANDROID

    if (!::startAlarm((int64_t) seq, _after)) {
        xerror2(TSF"startAlarm error, id:%0, after:%1, seq:%2", (uintptr_t)this, _after, seq);
        MessageQueue::CancelMessage(mqId);
        return false;
    }

#endif

    status_ = kStart;
    starttime_ = starttime;
    endtime_ = 0;
    after_ = _after;
    seq_ = seq;
    xinfo2(TSF"alarm id:%0, after:%1, seq:%2, po.reg.q:%3,po.reg.s:%4,po.s:%5", (uintptr_t)this, _after, seq, mqId.reg.queue, mqId.reg.seq, mqId.seq);

    return true;
}

bool Alarm::Cancel() {
    ScopedLock lock(sg_lock);
    MessageQueue::CancelMessage(reg_async_.Get());

    if (INVAILD_SEQ == seq_) return true;

#ifdef ANDROID

    if (!::stopAlarm((int64_t)seq_)) {
        xwarn2(TSF"stopAlarm error, id:%0, seq:%1", (uintptr_t)this, seq_);
        status_ = kCancel;
        endtime_ = gettickcount();
        seq_ = INVAILD_SEQ;
        return false;
    }

#endif

    xinfo2(TSF"alarm cancel id:%0, seq:%1, after:%2", (uintptr_t)this, seq_, after_);
    status_ = kCancel;
    endtime_ = gettickcount();
    seq_ = INVAILD_SEQ;
    return true;
}

bool Alarm::IsWaiting() const {
    return kStart == status_;
}

int Alarm::Status() const {
    return status_;
}

int Alarm::After() const {
    return after_;
}

int64_t Alarm::ElapseTime() const {
    if (endtime_ < starttime_)
        return gettickspan(starttime_);

    return endtime_ -  starttime_;
}

void Alarm::OnAlarm(const MessageQueue::MessagePost_t& _id, MessageQueue::Message& _message) {
    if (KALARM_MESSAGETITLE != _message.title) return;

    ScopedLock lock(sg_lock);

    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(reg_async_.Get())) {
        MessageQueue::AsyncInvoke(boost::bind(&Alarm::OnAlarm, this, _id, _message), (MessageQueue::MessageTitle_t)this, reg_async_.Get());
        return;
    }

    if (seq_ != boost::any_cast<int64_t>(_message.body1)) return;

    uint64_t  curtime = gettickcount();
    int64_t   elapseTime = curtime - starttime_;
    int64_t   missTime = after_ - elapseTime;
    xgroup2_define(group);
    xinfo2(TSF"OnAlarm id:%_, seq:%_, elapsed:%_, after:%_, miss:%_, android alarm:%_, ", (uintptr_t)this, seq_, elapseTime, after_, -missTime, !bool(boost::any_cast<int>(_message.body2))) >> group;

#ifdef ANDROID

    if (missTime > 0) {
        if (missTime <= MAX_LOCK_TIME) {
            if (NULL == wakelock_) wakelock_ = new WakeUpLock();

            wakelock_->Lock(missTime + 500);     // add 00ms
            xinfo2(TSF"wakelock") >> group;
            return;
        }

        ::stopAlarm(seq_);

        if (::startAlarm((int64_t) seq_, missTime)) return;

        xerror2(TSF"startAlarm err, continue") >> group;
    }

#endif

    xinfo2(TSF"runing") >> group;
    status_ = kOnAlarm;
    seq_ = INVAILD_SEQ;
    endtime_ = curtime;

    if (inthread_)
        runthread_.start();
    else
        MessageQueue::AsyncInvoke(boost::bind(&Alarm::__Run, this), (MessageQueue::MessageTitle_t)this, reg_async_.Get());
}

void Alarm::__Run() {
    target_->run();
}

const Thread& Alarm::RunThread() const {
    return runthread_;
}

#ifdef ANDROID
#include "jni/OnAlarm.inl"
#endif
