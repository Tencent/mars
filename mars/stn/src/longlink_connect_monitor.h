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
 * longlink_connect_monitor.h
 *
 *  Created on: 2014-2-26
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_CONNECT_MONITOR_H_
#define STN_SRC_LONGLINK_CONNECT_MONITOR_H_

#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/messagequeue/message_queue.h"
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
    uint64_t  __IntervalConnect(int _type);
    uint64_t  __AutoIntervalConnect();

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
    MessageQueue::ScopeRegister     asyncreg_;
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
