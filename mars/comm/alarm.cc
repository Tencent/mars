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

#include "comm/platform_comm.h"

static Mutex sg_lock;
static int64_t sg_seq = 1;
const MessageQueue::MessageTitle_t KALARM_MESSAGETITLE(0x1F1FF);
const MessageQueue::MessageTitle_t KALARM_SYSTEMTITLE(0x1F1F1E);

#define MAX_LOCK_TIME (5000)
#define INVAILD_SEQ (0)

bool Alarm::Start(int _after, bool _needWake) {
    ScopedLock lock(sg_lock);

    if (INVAILD_SEQ != seq_) return false;

    if (INVAILD_SEQ == sg_seq) sg_seq = 1;
    xinfo2(TSF"alarm sg_seq is %_", sg_seq);

    int64_t seq = sg_seq++;
    uint64_t starttime = gettickcount();
    broadcast_msg_id_ = MessageQueue::BroadcastMessage(MessageQueue::GetDefMessageQueue(), MessageQueue::Message(KALARM_MESSAGETITLE, (int64_t)seq, MessageQueue::GetDefMessageQueue(), "Alarm.broadcast"), MessageQueue::MessageTiming(_after));

    if (MessageQueue::KNullPost == broadcast_msg_id_) {
        xerror2(TSF"mq alarm return null post, id:%0, after:%1, seq:%2", (uintptr_t)this, _after, seq);
        return false;
    }

#ifdef ANDROID

    if (_needWake && !::startAlarm(type_, (int64_t) seq, _after)) {
        xerror2(TSF"startAlarm error, id:%0, after:%1, seq:%2", (uintptr_t)this, _after, seq);
        MessageQueue::CancelMessage(broadcast_msg_id_);
        broadcast_msg_id_ = MessageQueue::KNullPost;
        return false;
    }

#endif

    status_ = kStart;
    starttime_ = starttime;
    endtime_ = 0;
    after_ = _after;
    seq_ = seq;
    xinfo2(TSF"alarm id:%_, after:%_, seq:%_, po.reg.q:%_,po.reg.s:%_,po.s:%_, MQ:%_", (uintptr_t)this, _after, seq, broadcast_msg_id_.reg.queue, broadcast_msg_id_.reg.seq, broadcast_msg_id_.seq, MessageQueue::GetDefMessageQueue());

    return true;
}

bool Alarm::Cancel() {
    ScopedLock lock(sg_lock);
    if (broadcast_msg_id_!=MessageQueue::KNullPost) {
        MessageQueue::CancelMessage(broadcast_msg_id_);
        broadcast_msg_id_=MessageQueue::KNullPost;
    }
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
    if (KALARM_MESSAGETITLE != _message.title && KALARM_SYSTEMTITLE != _message.title) return;

    ScopedLock lock(sg_lock);

    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(reg_async_.Get())) {
        MessageQueue::AsyncInvoke(boost::bind(&Alarm::OnAlarm, this, _id, _message), (MessageQueue::MessageTitle_t)this, reg_async_.Get(), "Alarm::OnAlarm");
        return;
    }

    bool isSystemAlarm = (KALARM_SYSTEMTITLE == _message.title);
    MessageQueue::MessageQueue_t fromMQ = boost::any_cast<MessageQueue::MessageQueue_t>(_message.body2);

    if (seq_ != boost::any_cast<int64_t>(_message.body1) || fromMQ != MessageQueue::GetDefMessageQueue()) {
        if(fromMQ != MessageQueue::GetDefMessageQueue()) {
            xinfo2(TSF"not match:(%_, %_), (%_, %_)", seq_, boost::any_cast<int64_t>(_message.body1), MessageQueue::GetDefMessageQueue(), fromMQ);
        }
        return;
    }

    uint64_t  curtime = gettickcount();
    int64_t   elapseTime = curtime - starttime_;
    int64_t   missTime = after_ - elapseTime;
    xgroup2_define(group);
    xinfo2(TSF"OnAlarm id:%_, seq:%_, elapsed:%_, after:%_, miss:%_, android alarm:%_, MQ:%_", (uintptr_t)this, seq_, elapseTime, after_, -missTime, isSystemAlarm, MessageQueue::GetDefMessageQueue()) >> group;

#ifdef ANDROID

    if (missTime > 0) {
        if (missTime <= MAX_LOCK_TIME) {
            if (NULL == wakelock_) wakelock_ = new WakeUpLock();

            wakelock_->Lock(missTime + 500);     // add 00ms
            xinfo2(TSF"wakelock") >> group;
            return;
        }

        ::stopAlarm(seq_);

        if (::startAlarm(type_, (int64_t) seq_, missTime)) return;

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
        MessageQueue::AsyncInvoke(boost::bind(&Alarm::__Run, this), (MessageQueue::MessageTitle_t)this, reg_async_.Get(), "Alarm::__Run");
}

void Alarm::__Run() {
    target_->run();
}

const Thread& Alarm::RunThread() const {
    return runthread_;
}

static void StartWakeLock() {
#ifdef ANDROID
    static WakeUpLock wakelock; 
    wakelock.Lock(1000);    
    xinfo2(TSF"StartWakeLock");
#endif
}

#ifdef ANDROID
void Alarm::onAlarmImpl(int64_t _id) {
    xinfo2(TSF"onAlarm id:%_, MQ:%_", _id, MessageQueue::GetDefMessageQueue());
    StartWakeLock(); //wakelock need be acquired in onalarm thread, or will fail if try to acquire in other threads.
    MessageQueue::BroadcastMessage(MessageQueue::GetDefMessageQueue(), MessageQueue::Message(KALARM_SYSTEMTITLE, _id, MessageQueue::GetDefMessageQueue(), "KALARM_SYSTEMTITLE.id"));
}
#endif
