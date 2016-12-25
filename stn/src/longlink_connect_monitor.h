/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * longlink_connect_monitor.h
 *
 *  Created on: 2014-2-26
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_CONNECT_MONITOR_H_
#define STN_SRC_LONGLINK_CONNECT_MONITOR_H_

#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/alarm.h"

#include "longlink.h"

class ActiveLogic;

namespace mars {
    namespace stn {
        
class LongLinkConnectMonitor {
  public:
    LongLinkConnectMonitor(ActiveLogic& _activelogic, LongLink& _longlinkk, MessageQueue::MessageQueue_t _id);
    ~LongLinkConnectMonitor();

  public:
    bool MakeSureConnected();
    bool NetworkChange();

  public:
    boost::function<void ()> fun_longlink_reset_;

  private:
    unsigned long  __IntervalConnect(int _type);
    unsigned long  __AutoIntervalConnect();

  private:
    void __OnSignalForeground(bool _isforeground);
    void __OnSignalActive(bool _isactive);
    void __OnLongLinkStatuChanged(LongLink::TLongLinkStatus _status);
    void __OnAlarm();

    void __Run();
#ifdef __APPLE__
    bool __StartTimer();
    bool __StopTimer();
#endif
    void __ReConnect();
    
  private:
    LongLinkConnectMonitor(const LongLinkConnectMonitor&);
    LongLinkConnectMonitor& operator=(const LongLinkConnectMonitor&);

  private:
    ActiveLogic& activelogic_;
    LongLink& longlink_;
    Alarm         alarm_;
    Mutex         mutex_;

    LongLink::TLongLinkStatus status_;
    uint64_t last_connect_time_;
    int last_connect_net_type_;

    Thread thread_;
    Mutex testmutex_;

    int conti_suc_count_;
    bool isstart_;
};
        
} }

#endif // STN_SRC_LONGLINK_CONNECT_MONITOR_H_
