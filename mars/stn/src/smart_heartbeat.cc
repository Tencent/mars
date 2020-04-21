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
#include <algorithm>

#define KV_KEY_SMARTHEART 11249

static const std::string kFileName = "Heartbeat.ini";

// INI key
static const char* const kKeyModifyTime      = "modifyTime";
static const char* const kKeyCurHeart        = "curHeart";
static const char* const kKeyFailHeartCount  = "failHeartCount";
static const char* const kKeyStable          = "stable";
static const char* const kKeyNetType         = "netType";
static const char* const kKeyHeartType       = "hearttype";
static const char* const kKeyMinHeartFail    = "minheartfail";

SmartHeartbeat::SmartHeartbeat(): report_smart_heart_(NULL), is_wait_heart_response_(false), success_heart_count_(0), last_heart_(MinHeartInterval),
    pre_heart_(MinHeartInterval), cur_heart_(MinHeartInterval),
    ini_(mars::app::GetAppFilePath() + "/" + kFileName, false)
    , doze_mode_count_(0), normal_mode_count_(0), noop_start_tick_(false) {
    xinfo_function();
    ini_.Parse();
}

SmartHeartbeat::~SmartHeartbeat() {
    xinfo_function();
    __SaveINI();
}

void SmartHeartbeat::OnHeartbeatStart() {
    xverbose_function();

    noop_start_tick_.gettickcount();
    is_wait_heart_response_ = true;
}

void SmartHeartbeat::OnLongLinkEstablished() {
    xdebug_function();
    __LoadINI();
    success_heart_count_ = 0;
    pre_heart_ = cur_heart_ = MinHeartInterval;
}

void SmartHeartbeat::OnLongLinkDisconnect() {
    xinfo_function();

    OnHeartResult(false, false);
    current_net_heart_info_.succ_heart_count_ = 0;

    if (!current_net_heart_info_.is_stable_) {
		xinfo2(TSF"%0 not stable last heart:%1", current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_);
		return;
	}

    last_heart_ = MinHeartInterval;
}

#define ONE_DAY_SECONEDS (24 * 60 * 60)

void SmartHeartbeat::OnHeartResult(bool _sucess, bool _fail_of_timeout) {
    if(!is_wait_heart_response_)
        return;
    
    if(report_smart_heart_&& !_sucess && success_heart_count_ >=NetStableTestCount && current_net_heart_info_.is_stable_) {
        report_smart_heart_(kActionDisconnect, current_net_heart_info_, _fail_of_timeout);
    }
    
    xinfo2(TSF"heart result:%0, timeout:%1", _sucess, _fail_of_timeout);
    pre_heart_ = cur_heart_;
    cur_heart_ = last_heart_;
    is_wait_heart_response_ = false;

    xassert2(!current_net_heart_info_.net_detail_.empty(), "something wrong,net_detail_ shoudn't be NULL");
    if (current_net_heart_info_.net_detail_.empty()) return;
    if(_sucess) success_heart_count_ += 1;
    if (success_heart_count_ <= NetStableTestCount) {
        current_net_heart_info_.min_heart_fail_count_ = _sucess ? 0 : (current_net_heart_info_.min_heart_fail_count_ + 1);
        if(report_smart_heart_ && current_net_heart_info_.min_heart_fail_count_ >= 6 && ::isNetworkConnected()) {
            report_smart_heart_(kActionBadNetwork, current_net_heart_info_, false);
            current_net_heart_info_.min_heart_fail_count_ = 0;
        }
        return;
    }

    if (last_heart_ != current_net_heart_info_.cur_heart_) {
        xdebug2(TSF"last heart & cur_heart not match, ignore");
        return;
    }
    
    if(_sucess) {
        if (last_heart_ == pre_heart_) {
            current_net_heart_info_.succ_heart_count_ += 1;
            current_net_heart_info_.fail_heart_count_ = 0;
        }
    }
    else {
        if(_fail_of_timeout) {
            current_net_heart_info_.succ_heart_count_ = 0;
        }
        current_net_heart_info_.fail_heart_count_ += 1;
    }
    
    if (_sucess && current_net_heart_info_.is_stable_) {
        // has reach the Max value, no need to try bigger.
        if (current_net_heart_info_.cur_heart_ >= MaxHeartInterval - SuccessStep)   return;
        
        time_t cur_time = time(NULL);
        // heart info changed recently,Don't need probe
        // probe bigger heart on Wednesday
        if ((cur_time - current_net_heart_info_.last_modify_time_) >= 7*ONE_DAY_SECONEDS && current_net_heart_info_.cur_heart_ < (MaxHeartInterval - SuccessStep)) {
            xinfo2(TSF"tryProbeBiggerHeart. curHeart=%_, last modify:%_", current_net_heart_info_.cur_heart_, current_net_heart_info_.last_modify_time_);
            current_net_heart_info_.cur_heart_ += SuccessStep;
            current_net_heart_info_.succ_heart_count_ = 0;
            current_net_heart_info_.is_stable_ = false;
            current_net_heart_info_.fail_heart_count_ = 0;
            if(report_smart_heart_)
                report_smart_heart_(kActionReCalc, current_net_heart_info_, false);
            __SaveINI();
        }
        return;
    }
    
    if (_sucess) {
        if(current_net_heart_info_.succ_heart_count_ >= BaseSuccCount) {
            if (current_net_heart_info_.cur_heart_ >= (MaxHeartInterval - SuccessStep)) {
                //already max, make stable
                current_net_heart_info_.cur_heart_ = MaxHeartInterval - SuccessStep;
                current_net_heart_info_.succ_heart_count_ = 0;
                current_net_heart_info_.is_stable_ = true;
                current_net_heart_info_.heart_type_ = __IsDozeStyle() ? kDozeModeHeart : kSmartHeartBeat;
                xinfo2(TSF"%0 find the smart heart interval = %1", current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_);
                if(report_smart_heart_)
                    report_smart_heart_(kActionCalcEnd, current_net_heart_info_, false);
            } else {
                current_net_heart_info_.succ_heart_count_ = 0;
                
                unsigned int old_heart = current_net_heart_info_.cur_heart_;
                if(__IsDozeStyle()) {
                    current_net_heart_info_.cur_heart_ = MaxHeartInterval - SuccessStep;
                } else {
                    current_net_heart_info_.cur_heart_ += HeartStep;
                    current_net_heart_info_.cur_heart_ = std::min((unsigned int)(MaxHeartInterval - SuccessStep), current_net_heart_info_.cur_heart_);
                }
                
                xinfo2(TSF"increace curHeart from %_ to %_", old_heart, current_net_heart_info_.cur_heart_);
            }
        }
    } else {
        if (last_heart_ == MinHeartInterval) return;
        
        if (current_net_heart_info_.fail_heart_count_ >= MaxHeartFailCount) {
            if (current_net_heart_info_.is_stable_) {
                current_net_heart_info_.cur_heart_ = MinHeartInterval;
                current_net_heart_info_.succ_heart_count_ = 0;
                current_net_heart_info_.is_stable_ = false;
                if(report_smart_heart_)
                    report_smart_heart_(kActionReCalc, current_net_heart_info_, true);
                //first report, then set fail count
                current_net_heart_info_.fail_heart_count_  = 0;
                xinfo2(TSF"in stable sate,can't use old value to Keep TCP alive");
            } else {
                if(__IsDozeStyle()) {
                    current_net_heart_info_.cur_heart_ = MinHeartInterval;
                } else if ((current_net_heart_info_.cur_heart_ - HeartStep - SuccessStep) > MinHeartInterval) {
                    current_net_heart_info_.cur_heart_ = current_net_heart_info_.cur_heart_ - HeartStep - SuccessStep;
                } else {
                    current_net_heart_info_.cur_heart_ = MinHeartInterval;
                }
                
                current_net_heart_info_.succ_heart_count_ = 0;
                current_net_heart_info_.fail_heart_count_ = 0;
                current_net_heart_info_.is_stable_ = true;
                current_net_heart_info_.heart_type_ = __IsDozeStyle() ? kDozeModeHeart : kSmartHeartBeat;
                xinfo2(TSF"finish choose the proper value %0", current_net_heart_info_.cur_heart_);
                if(report_smart_heart_)
                    report_smart_heart_(kActionCalcEnd, current_net_heart_info_, false);
            }
        }
    }
    
    __DumpHeartInfo();
    __SaveINI();
}


#define MAX_JUDGE_TIMES (10)

void SmartHeartbeat::JudgeDozeStyle() {
    
    if(ActiveLogic::Instance()->IsActive())  return;
    if(!noop_start_tick_.isValid()) return;
    if(kMobile != ::getNetInfo())   return;
    
    if(std::abs(noop_start_tick_.gettickspan() - last_heart_) >= 20*1000) {
        doze_mode_count_++;
        normal_mode_count_ = std::max(normal_mode_count_-1, 0);
    } else {
        normal_mode_count_++;
        doze_mode_count_ = std::max(doze_mode_count_-1, 0);
    }
    noop_start_tick_ = tickcount_t(false);
}


bool SmartHeartbeat::__IsDozeStyle() {
    return doze_mode_count_ >= 2 && doze_mode_count_ > (2*normal_mode_count_);
}

unsigned int SmartHeartbeat::GetNextHeartbeatInterval() {  //
    // xinfo_function();
    
    if(ActiveLogic::Instance()->IsActive()) {
        last_heart_ = MinHeartInterval;
        return MinHeartInterval;
    }

    if (success_heart_count_ < NetStableTestCount || current_net_heart_info_.net_detail_.empty()) {
        //        xdebug2(TSF"getNextHeartbeatInterval use MinHeartInterval. success_heart_count_=%0",success_heart_count_);
        last_heart_ = MinHeartInterval;
        return MinHeartInterval;
    }

    last_heart_ = current_net_heart_info_.cur_heart_;
    xassert2((last_heart_ < MaxHeartInterval && last_heart_ >= MinHeartInterval), "heart value invalid");

    if(__IsDozeStyle() && current_net_heart_info_.heart_type_ != kDozeModeHeart && last_heart_ != (MaxHeartInterval - SuccessStep)) {
        current_net_heart_info_.cur_heart_ = last_heart_ = MinHeartInterval;
    }
    if(last_heart_ >= MaxHeartInterval || last_heart_ < MinHeartInterval) {
        current_net_heart_info_.cur_heart_ = last_heart_ = MinHeartInterval;
    }
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
        current_net_heart_info_.heart_type_ = (TSmartHeartBeatType)ini_.Get(kKeyHeartType, 0);
        current_net_heart_info_.min_heart_fail_count_ = ini_.Get(kKeyMinHeartFail, 0);
        
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
    __DumpHeartInfo();
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

    for (SpecialINI::sections_t::iterator iter = sections.begin(); iter != sections.end();) {
        SpecialINI::keys_t::iterator time_iter = iter->second.find(kKeyModifyTime);

        if (time_iter == iter->second.end()) {
            // remove dirty value
            sections.erase(iter++);
            xinfo2(TSF"remove dirty value because miss KEY_ModifyTime");
            continue;
        }

        time_t time_value = number_cast<time_t>(time_iter->second.c_str());
        
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
    if(current_net_heart_info_.net_detail_.empty())return;
    
    current_net_heart_info_.last_modify_time_ = time(NULL);

    ini_.Set<time_t>(kKeyModifyTime, current_net_heart_info_.last_modify_time_);
    ini_.Set(kKeyCurHeart, current_net_heart_info_.cur_heart_);
    ini_.Set(kKeyFailHeartCount, current_net_heart_info_.fail_heart_count_);
    ini_.Set(kKeyStable, current_net_heart_info_.is_stable_);
    ini_.Set(kKeyNetType, current_net_heart_info_.net_type_);
    ini_.Set(kKeyHeartType, current_net_heart_info_.heart_type_);
    ini_.Set(kKeyMinHeartFail, current_net_heart_info_.min_heart_fail_count_);
    ini_.Save();
}

void SmartHeartbeat::__DumpHeartInfo() {
    xinfo2(TSF"SmartHeartbeat Info last_heart_:%0,successHeartCount:%1, currSuccCount:%2", last_heart_, success_heart_count_, current_net_heart_info_.succ_heart_count_);

    if (!current_net_heart_info_.net_detail_.empty()) {
        xinfo2(TSF"currentNetHeartInfo detail:%0,curHeart:%1,isStable:%2,failcount:%3,modifyTime:%4,type:%5,min_fail:%6",
               current_net_heart_info_.net_detail_, current_net_heart_info_.cur_heart_, current_net_heart_info_.is_stable_,
               current_net_heart_info_.fail_heart_count_, current_net_heart_info_.last_modify_time_
               ,(int)current_net_heart_info_.heart_type_, current_net_heart_info_.min_heart_fail_count_);
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
    succ_heart_count_ = fail_heart_count_ = min_heart_fail_count_ = 0;
    heart_type_ = kNoSmartHeartBeat;
    is_stable_ = false;
}
