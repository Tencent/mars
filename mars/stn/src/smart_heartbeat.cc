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
 * smart_heartbeat.cc
 *
 * description: Manage the heartbeat frequecy by Adaptive Computing for the current active network.
 *              The purpose is to decrease the heartbeat frequecy when our app in inactive state,
 *              And meanwhile keep the TCP alive as far as possible.
 * Created on: 2014-1-22
 * Author: phoenixzuo
 *
 */

#include "smart_heartbeat.h"

#include <time.h>
#include <unistd.h>

#include "boost/filesystem.hpp"

#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/singleton.h"
#include "mars/comm/platform_comm.h"

#include "mars/baseevent/active_logic.h"
#include "mars/app/app.h"

#include "mars/stn/config.h"

#define KV_KEY_SMARTHEART 11249

static const std::string kFileName = "Heartbeat.ini";

// INI key
static const char* const kKeyModifyTime      = "modifyTime";
static const char* const kKeyCurHeart        = "curHeart";
static const char* const kKeyFailHeartCount  = "failHeartCount";
static const char* const kKeyStable          = "stable";
static const char* const kKeyNetType         = "netType";

SmartHeartbeat::SmartHeartbeat(): is_wait_heart_response_(false), xiaomi_style_count_(0), success_heart_count_(0), last_heart_(MinHeartInterval),
    ini_(mars::app::GetAppFilePath() + "/" + kFileName, false) {
    xinfo_function();
    ini_.Parse();
}

SmartHeartbeat::~SmartHeartbeat() {
    xinfo_function();
}

void SmartHeartbeat::OnHeartbeatStart() {
    xverbose_function();

    if (__IsMIUIStyle())
        return;

    ScopedLock lock(_mutex_);
    is_wait_heart_response_ = true;
}

void SmartHeartbeat::OnLongLinkEstablished() {
    xdebug_function();
    __LoadINI();
    ScopedLock lock(_mutex_);
    success_heart_count_ = 0;
}

void SmartHeartbeat::OnLongLinkDisconnect() {
    xinfo_function();

    if (__IsMIUIStyle())
        return;

    if (is_wait_heart_response_)
        OnHeartResult(false, false);

    ScopedLock lock(_mutex_);

    is_wait_heart_response_ = false;

    if (!current_net_heart_info_.is_stable_) {
		xinfo2(TSF"%0 not stable last heart:%1", current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_);
		return;
	}

    current_net_heart_info_.success_curr_heart_count_ = 0;
    success_heart_count_ = 0;
    last_heart_ = MinHeartInterval;
}

#define ONE_DAY_SECONEDS (24 * 60 * 60)

void SmartHeartbeat::OnHeartResult(bool _sucess, bool _fail_of_timeout) {
    xdebug2(TSF"heart result:%0, %1", _sucess, _fail_of_timeout);

    if (__IsMIUIStyle())
        return;

    ScopedLock lock(_mutex_);
    xassert2(!current_net_heart_info_.net_detail_.empty(), "something wrong,net_detail_ shoudn't be NULL");
    
    if (current_net_heart_info_.net_detail_.empty()) return;

    is_wait_heart_response_ = false;
    
    if (_sucess) {
        success_heart_count_++;
        if (current_net_heart_info_.is_stable_) {
            if (success_heart_count_ >= NetStableTestCount) {
                
                // has reach the Max value, no need to try bigger.
                if (current_net_heart_info_.cur_heart_ >= MaxHeartInterval - SuccessStep) {
                    return;
                }
                
                time_t cur_time = time(NULL);
                struct tm cur_TM = *localtime(&cur_time);
                
                // heart info changed recently,Don't need probe
                // probe bigger heart on Wednesday
                if ((cur_time - current_net_heart_info_.last_modify_time_) >= ONE_DAY_SECONEDS && cur_TM.tm_wday == 2) {
                    xinfo2(TSF"__TryProbeBiggerHeart. curHeart=%0  ", current_net_heart_info_.cur_heart_);
                    current_net_heart_info_.cur_heart_ += SuccessStep;
                    current_net_heart_info_.success_curr_heart_count_ = 0;
                    current_net_heart_info_.is_stable_ = false;
                    current_net_heart_info_.fail_heart_count_ = 0;
                    __SaveINI();
                }
            }
            return;
        }
    }


    if (last_heart_ != current_net_heart_info_.cur_heart_) {
        xinfo2(TSF"dynamic heart stop by some reason");
        return;
    }

    if (success_heart_count_ < NetStableTestCount) return;
    
    if (_sucess) {
        current_net_heart_info_.success_curr_heart_count_++;
        
        if (current_net_heart_info_.cur_heart_ >= MaxHeartInterval) {
            current_net_heart_info_.cur_heart_ = MaxHeartInterval - SuccessStep;
            current_net_heart_info_.success_curr_heart_count_ = 0;
            current_net_heart_info_.is_stable_ = true;
            xinfo2(TSF"%0 find the smart heart interval = %1", current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_);
        } else {
            if (current_net_heart_info_.success_curr_heart_count_ >= BaseSuccCount) {
                unsigned int old_heart = current_net_heart_info_.cur_heart_;
                current_net_heart_info_.cur_heart_ += HeartStep;
                
                if (current_net_heart_info_.cur_heart_ > MaxHeartInterval) {
                    current_net_heart_info_.cur_heart_ = MaxHeartInterval;
                }
                
                current_net_heart_info_.success_curr_heart_count_ = 0;
                xinfo2(TSF"Increace curHeart from %0 to %1", old_heart, current_net_heart_info_.cur_heart_);
            } else
                xdebug2(TSF"current succ count<3, curr:%0", current_net_heart_info_.success_curr_heart_count_);
        }
        current_net_heart_info_.fail_heart_count_ = 0;
        
    } else {
        if (last_heart_ == MinHeartInterval) return;
        
        current_net_heart_info_.fail_heart_count_++;
        
        if (current_net_heart_info_.fail_heart_count_ >= MaxHeartFailCount) {
            if (current_net_heart_info_.is_stable_) {
                current_net_heart_info_.cur_heart_ = MinHeartInterval;
                current_net_heart_info_.success_curr_heart_count_ = 0;
                current_net_heart_info_.is_stable_ = false;
                current_net_heart_info_.fail_heart_count_  = 0;
                xinfo2(TSF"in stable sate,can't use old value to Keep TCP alive, restart __AdaptiveComputing");
            } else {
                if (current_net_heart_info_.cur_heart_ - HeartStep - SuccessStep > MinHeartInterval) {
                    current_net_heart_info_.cur_heart_ = current_net_heart_info_.cur_heart_ - HeartStep - SuccessStep;
                } else {
                    current_net_heart_info_.cur_heart_ = MinHeartInterval;
                }
                
                current_net_heart_info_.success_curr_heart_count_ = 0;
                current_net_heart_info_.fail_heart_count_ = 0;
                
                current_net_heart_info_.is_stable_ = true;
                
                xinfo2(TSF"finsh AdaptiveComputing choose the proper value %0", current_net_heart_info_.cur_heart_);
            }
        }
    }
    
    __DumpHeartInfo();
    __SaveINI();
}


#define MAX_JUDGE_TIMES (10)

void SmartHeartbeat::JudgeMIUIStyle() {
    static int test_total_count = 0;
    static uint64_t last_alarm_tick = 0;

    if (test_total_count >= MAX_JUDGE_TIMES) {
        return;
    }

    if (last_alarm_tick == 0) {
        last_alarm_tick = gettickcount();
        return;
    }

    uint64_t span = gettickspan(last_alarm_tick);
    last_alarm_tick = ::gettickcount();

    if (span < 10000)    // for case the same alarm
        return;

    if ((span % 300000) <= 10000 || (300000 - (span % 300000)) <= 10000) {  // judge if curTime is times of five minutes, 10 seconds as the max offset
        xiaomi_style_count_++;
        xinfo2(TSF"m_xiaomiStyleCount++ %0", xiaomi_style_count_);

        if (!current_net_heart_info_.is_stable_ && xiaomi_style_count_ >= 3) {
            xinfo2(TSF"judgeMIUIStyle: is MIUIStyle. xiaomiCount = %0 ", xiaomi_style_count_);
            current_net_heart_info_.is_stable_ = true;
            __SaveINI();
        }
    } else {
        xiaomi_style_count_ = 0;
    }

    test_total_count++;
}


bool SmartHeartbeat::__IsMIUIStyle() {
    return xiaomi_style_count_ >= 3;
}

unsigned int SmartHeartbeat::GetNextHeartbeatInterval(bool& _use_smart_heartbeat) {  // bIsUseSmartBeat is add by andrewu for stat
    // xinfo_function();
    _use_smart_heartbeat = false;
    ScopedLock lock(_mutex_);

    if (SINGLETON_STRONG(ActiveLogic)->IsActive() || success_heart_count_ < NetStableTestCount || current_net_heart_info_.net_detail_.empty()
            || __IsMIUIStyle()) {
        //        xdebug2(TSF"getNextHeartbeatInterval use MinHeartInterval. success_heart_count_=%0",success_heart_count_);
        last_heart_ = MinHeartInterval;
        return MinHeartInterval;
    }

    _use_smart_heartbeat = true;

    unsigned int heart = current_net_heart_info_.cur_heart_;

    if (heart < MinHeartInterval || heart > MaxHeartInterval) {
        xassert2(false, "shouldn't be here,  smaller than min heart:%d", heart);
        current_net_heart_info_.cur_heart_ = MinHeartInterval;
        current_net_heart_info_.success_curr_heart_count_ = 0;

        current_net_heart_info_.is_stable_ = false;
        __SaveINI();
    }

    last_heart_ = current_net_heart_info_.cur_heart_;
    return last_heart_;
}

void SmartHeartbeat::__LoadINI() {
    xinfo_function();
    std::string net_info;
    int net_type = getCurrNetLabel(net_info);
    
    if (net_info.empty()) {
        current_net_heart_info_.Clear();
        xerror2("net_info NULL");
        return;
    }
    if (net_info == current_net_heart_info_.net_detail_) return;
    
    current_net_heart_info_.Clear();
    current_net_heart_info_.net_detail_ = net_info;
    current_net_heart_info_.net_type_ = net_type;

    if (ini_.Select(net_info)) {
        
        current_net_heart_info_.last_modify_time_ = ini_.Get(kKeyModifyTime, current_net_heart_info_.last_modify_time_);
        current_net_heart_info_.cur_heart_ = ini_.Get(kKeyCurHeart, current_net_heart_info_.cur_heart_);
        current_net_heart_info_.fail_heart_count_ = ini_.Get(kKeyFailHeartCount, current_net_heart_info_.fail_heart_count_);
        current_net_heart_info_.is_stable_ = ini_.Get(kKeyStable, current_net_heart_info_.is_stable_);
        current_net_heart_info_.net_type_ = ini_.Get(kKeyNetType, current_net_heart_info_.net_type_);
        
        xassert2(net_type == current_net_heart_info_.net_type_, "cur:%d, INI:%d", net_type, current_net_heart_info_.net_type_);
        
        if (current_net_heart_info_.cur_heart_ < MinHeartInterval) {
            xerror2(TSF"current_net_heart_info_.cur_heart_:%_ < MinHeartInterval:%_", current_net_heart_info_.cur_heart_, MinHeartInterval);
            current_net_heart_info_.cur_heart_ = MinHeartInterval;
        }
        
        if (current_net_heart_info_.cur_heart_ > MaxHeartInterval) {
            xerror2(TSF"current_net_heart_info_.cur_heart_:%_ > MaxHeartInterval:%_", current_net_heart_info_.cur_heart_, MaxHeartInterval);
            current_net_heart_info_.cur_heart_ = MaxHeartInterval - SuccessStep;
        }
        
        time_t cur_time = time(NULL);
        
        if (current_net_heart_info_.last_modify_time_ > cur_time) {
            xerror2(TSF"current_net_heart_info_.last_modify_time_:%_ > cur_time:%_", current_net_heart_info_.last_modify_time_, cur_time);
            current_net_heart_info_.last_modify_time_ = cur_time;
        }
    } else {
        __LimitINISize();
        bool ret = ini_.Create(net_info);
        xassert2(ret);
        __SaveINI();
    }
}

#define MAX_INI_SECTIONS (20)

void SmartHeartbeat::__LimitINISize() {
    xinfo_function();
    SpecialINI::sections_t& sections =  ini_.Sections();

    if (ini_.Sections().size() <= MAX_INI_SECTIONS)
        return;

    xwarn2(TSF"sections.size=%0 > MAX_INI_SECTIONS=%1", sections.size(), MAX_INI_SECTIONS);

    time_t cur_time = time(NULL);

    time_t min_time = 0;
    SpecialINI::sections_t::iterator min_iter = sections.end();
    std::stringstream stream;

    for (SpecialINI::sections_t::iterator iter = sections.begin(); iter != sections.end();) {
        SpecialINI::keys_t::iterator time_iter = iter->second.find(kKeyModifyTime);

        if (time_iter == iter->second.end()) {
            // remove dirty value
            sections.erase(iter++);
            xinfo2(TSF"remove dirty value because miss KEY_ModifyTime");
            continue;
        }

        time_t time_value = 0;
        stream << time_iter->second;
        stream >> time_value;
        stream.clear();
        stream.str("");

        if (time_value > cur_time) {
            // remove dirty value
            sections.erase(iter++);
            xinfo2(TSF"remove dirty value because Wrong ModifyTime ");
            continue;
        }

        if (0 == min_time || time_value < min_time) {
            min_iter = iter;
            min_time = time_value;
        }

        ++iter;
    }

    if (min_iter != sections.end()) sections.erase(min_iter);
}

void SmartHeartbeat::__SaveINI() {
    xdebug_function();
    current_net_heart_info_.last_modify_time_ = time(NULL);

    ini_.Set<time_t>(kKeyModifyTime, current_net_heart_info_.last_modify_time_);
    ini_.Set(kKeyCurHeart, current_net_heart_info_.cur_heart_);
    ini_.Set(kKeyFailHeartCount, current_net_heart_info_.fail_heart_count_);
    ini_.Set(kKeyStable, current_net_heart_info_.is_stable_);
    ini_.Set(kKeyNetType, current_net_heart_info_.net_type_);
    ini_.Save();
}

void SmartHeartbeat::__DumpHeartInfo() {
    xinfo2(TSF"SmartHeartbeat Info last_heart_:%0,successHeartCount:%1, currSuccCount:%2", last_heart_, success_heart_count_, current_net_heart_info_.success_curr_heart_count_);

    if (!current_net_heart_info_.net_detail_.empty()) {
        xinfo2(TSF"currentNetHeartInfo detail:%0,curHeart:%1,isStable:%2,failcount:%3,modifyTime:%4",
               current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_, current_net_heart_info_.is_stable_,
               current_net_heart_info_.fail_heart_count_, current_net_heart_info_.last_modify_time_);
    }
}

// #pragma endregion

// #pragma region NetHeartbeatInfo

NetHeartbeatInfo::NetHeartbeatInfo() {
    Clear();
}

void NetHeartbeatInfo::Clear() {
    net_detail_ = "";
    net_type_ = kNoNet;
    last_modify_time_ = 0;
    cur_heart_ = MinHeartInterval;
    fail_heart_count_ = 0;
    is_stable_ = false;
    
    success_curr_heart_count_ = 0;
}
