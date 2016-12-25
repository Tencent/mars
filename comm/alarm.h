/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * alarm.h
 *
 *  Created on: 2012-8-27
 *      Author: yerungui
 */

#ifndef COMM_ALARM_H_
#define COMM_ALARM_H_

#include <boost/bind.hpp>
#include "messagequeue/message_queue.h"
#include "messagequeue/message_queue_utils.h"

#ifdef ANDROID
#include "android/wakeuplock.h"
#endif

class Alarm {
  public:
    enum {
        kInit,
        kStart,
        kCancel,
        kOnAlarm,
    };

  public:
    template<class T>
    explicit Alarm(const T& _op, bool _inthread = true)
        : target_(detail::transform(_op))
        , reg_async_(MessageQueue::InstallAsyncHandler(MessageQueue::GetDefMessageQueue()))
        , runthread_(boost::bind(&Alarm::__Run, this), "alarm")
        , inthread_(_inthread)
        , seq_(0), status_(kInit)
        , after_(0) , starttime_(0) , endtime_(0)
        , reg_(MessageQueue::InstallMessageHandler(boost::bind(&Alarm::OnAlarm, this, _1, _2), true))
#ifdef ANDROID
        , wakelock_(NULL)
#endif
    {}

    template<class T>
    explicit Alarm(const T& _op, const MessageQueue::MessageQueue_t& _id)
        : target_(detail::transform(_op))
        , reg_async_(MessageQueue::InstallAsyncHandler(_id))
        , runthread_(boost::bind(&Alarm::__Run, this), "alarm")
        , inthread_(false)
        , seq_(0), status_(kInit)
        , after_(0) , starttime_(0) , endtime_(0)
        , reg_(MessageQueue::InstallMessageHandler(boost::bind(&Alarm::OnAlarm, this, _1, _2), true))
#ifdef ANDROID
        , wakelock_(NULL)
#endif
    {}

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

    bool Start(int _after);  // ms
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
    virtual void    __Run();

  private:
    Runnable*                   target_;
    MessageQueue::ScopeRegister reg_async_;
    Thread                      runthread_;
    bool                        inthread_;

    int64_t                  	seq_;
    int                         status_;

    int                         after_;
    uint64_t          			starttime_;
    uint64_t          			endtime_;

    MessageQueue::ScopeRegister reg_;
#ifdef ANDROID
    WakeUpLock*                 wakelock_;
#endif
};

#endif /* COMM_ALARM_H_ */
