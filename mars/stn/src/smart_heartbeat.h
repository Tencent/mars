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
 * smart_heartbeat.h
 *
 *  Created on: 2014-1-22
 *      Author: phoenixzuo
 */

#ifndef STN_SRC_SMART_HEARTBEAT_H_
#define STN_SRC_SMART_HEARTBEAT_H_

#include <string>

#include "mars/comm/singleton.h"
#include "mars/comm/tickcount.h"
#include "mars/stn/config.h"

#include "special_ini.h"

enum HeartbeatReportType {
    kReportTypeCompute            = 1,        // report info of compute smart heartbeat
    kReportTypeSuccRate           = 2,    // report succuss rate when smart heartbeat is stabled
};
enum TSmartHeartBeatType {
	kNoSmartHeartBeat = 0,
	kSmartHeartBeat,
	kDozeModeHeart,
};

enum TSmartHeartBeatAction {
    kActionCalcEnd = 0,
    kActionReCalc = 1,
    kActionDisconnect = 2,
    kActionBadNetwork = 3
};

class SmartHeartbeat;

class NetHeartbeatInfo {
  public:
    NetHeartbeatInfo();
    void Clear();

  public:
//    NetHeartbeatInfo(const NetHeartbeatInfo&);
//    NetHeartbeatInfo& operator=(const NetHeartbeatInfo&);
    
  public:
    std::string net_detail_;
    int net_type_;

    unsigned int cur_heart_;
    TSmartHeartBeatType heart_type_;
    bool is_stable_;
    time_t last_modify_time_;

    unsigned int fail_heart_count_;  // accumulated failed counts on curHeart
    unsigned int succ_heart_count_;
    unsigned int min_heart_fail_count_;

    friend class SmartHeartbeat;
};

class SmartHeartbeat {
  public:
    boost::function<void (TSmartHeartBeatAction _action, const NetHeartbeatInfo& _heart_info, bool _fail_timeout)> report_smart_heart_;
    
	SmartHeartbeat();
	~SmartHeartbeat();
    void OnHeartbeatStart();

    void OnLongLinkEstablished();
    void OnLongLinkDisconnect();
    void OnHeartResult(bool _sucess, bool _fail_of_timeout);
    unsigned int GetNextHeartbeatInterval();   // bIsUseSmartBeat is add by andrewu for stat

    // MIUI align alarm response at Times of five minutes, We should  handle this case specailly.
    void JudgeDozeStyle();

  private:
    void __DumpHeartInfo();

    bool __IsDozeStyle();

    void __LimitINISize();
    void __LoadINI();
    void __SaveINI();

  private:
    bool is_wait_heart_response_;
    
    unsigned int success_heart_count_;  // the total success heartbeat based on single alive TCP, And heartbeat interval can be different.
    unsigned int last_heart_;
    unsigned int pre_heart_;
    unsigned int cur_heart_;
    NetHeartbeatInfo current_net_heart_info_;

    SpecialINI ini_;
    
    int doze_mode_count_;
    int normal_mode_count_;
    tickcount_t noop_start_tick_;
    
};

#endif // STN_SRC_SMART_HEARTBEAT_H_
