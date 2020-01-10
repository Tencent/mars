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

#include <string>

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
    struct LongLinkProgressRecord {
      uint64_t longlink_start_connect_time;
      uint64_t longlink_connected_time;
      uint64_t longlink_disconnect_time;
      uint64_t next_rebuild_time_interval;
      uint64_t next_heartbeat_interval;
      uint64_t longlink_rebuild_start_alarm;
      uint64_t networkchange_time;
      uint64_t noop_alarm_set_time;
      uint64_t noop_alarm_interval;
      int      alarm_after_time;
      bool foreground;
      bool process_active;
      bool rebuild_from_alarm;
      bool rebuild_from_netwrokchange;
      bool noop_timeout;
      std::string error_msg;
      std::string alarm_reason;

      void Reset() {
        longlink_start_connect_time = 0;
        longlink_connected_time = 0;
        longlink_disconnect_time = 0;
        next_rebuild_time_interval = 0;
        next_heartbeat_interval = 0;
        longlink_rebuild_start_alarm = 0;
        networkchange_time  = 0;
        noop_alarm_set_time = 0;
        noop_alarm_interval = 0;
        alarm_after_time = 0;
        rebuild_from_alarm = false;
        rebuild_from_netwrokchange = false;
        noop_timeout  = false;
        error_msg = "";
        alarm_reason = "";
      }
    };

  public:
    LongLinkConnectMonitor(ActiveLogic& _activelogic, LongLink& _longlinkk, MessageQueue::MessageQueue_t _id);
    ~LongLinkConnectMonitor();

  public:
    bool MakeSureConnected();
    bool NetworkChange();
    void OnHeartbeatAlarmSet(uint64_t _interval);
    void OnHeartbeatAlarmReceived(bool _is_noop_timeout);


  public:
    boost::function<void ()> fun_longlink_reset_;
    boost::function<std::string ()> fun_get_disconnect_error_msg_;

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

    void __PrintRecordToOSLogger();
    
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
    LongLinkProgressRecord progress_record_;
};
        
} }

#endif // STN_SRC_LONGLINK_CONNECT_MONITOR_H_
