// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  weak_network_logic.cc
//
//  Created by zhouzhijie on 2017-10-20.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "weak_network_logic.h"
#include "mars/comm/xlogger/xlogger.h"

#define MARK_TIMEOUT (60*1000)
#define WEAK_CONNECT_RTT (2 * 1000)
#define WEAK_PKG_SPAN (2*1000)
#define GOOD_TASK_SPAN (600)
#define SURE_WEAK_SPAN (5*1000)
#define WEAK_TASK_SPAN (5*1000)
#define WEAK_LEAST_SPAN (8*1000)
//#define LAST_CONNECTINFO_VALID_SPAN (10*1000)

namespace mars {
namespace stn {

    //do not delete or insert
    enum TKey {
        kEnterWeak = 0,
        kExitWeak,
        kWeakTime,
        kCGICount,
        kCGICost,
        kCGISucc,
        kSceneRtt,
        kSceneIndex,
        kSceneFirstPkg,
        kScenePkgPkg,
        kSceneTask,
        kExitSceneTask,
        kExitSceneTimeout,
        kExitSceneConnect,
        kExitSceneBackground,
        kExitQuickConnectNoNet,
        kSceneTaskBad,
        kFailStepDns = 31,
        kFailStepConnect,
        kFailStepFirstPkg,
        kFailStepPkgPkg,
        kFailStepDecode,
        kFailStepOther,
        kFailStepTimeout,
        kFailStepServer,
        kFailCurrent,
        kFailSecond,
        kFailThird,
        kFailMore,
    };
    
    WeakNetworkLogic::WeakNetworkLogic():is_curr_weak_(false), connect_after_weak_(0)
        , last_connect_fail_tick_(false), last_connect_suc_tick_(false), cgi_fail_num_(0) {
        ActiveLogic::Instance()->SignalForeground.connect(boost::bind(&WeakNetworkLogic::__SignalForeground, this, _1));
    }
    
    WeakNetworkLogic::~WeakNetworkLogic() {
        ActiveLogic::Instance()->SignalForeground.disconnect(boost::bind(&WeakNetworkLogic::__SignalForeground, this, _1));
    }
    
    void WeakNetworkLogic::__SignalForeground(bool _is_foreground) {
        if(!_is_foreground && is_curr_weak_) {
            __MarkWeak(false);
            __ReportWeakLogic(kExitSceneBackground, 1, false);
            xinfo2(TSF"weak network end");
        }
    }
    
    void WeakNetworkLogic::__ReportWeakLogic(int _key, int _value, bool _is_important) {
        xinfo2(TSF"weak idkey:(%_, %_)", _key, _value);
        if (report_weak_logic_) {
            report_weak_logic_(_key, _value, _is_important);
        }
    }
    
    bool WeakNetworkLogic::IsCurrentNetworkWeak() {
        if(is_curr_weak_) {
            if(last_mark_tick_.gettickspan() < MARK_TIMEOUT)    return true;
            else {
                __MarkWeak(false);
                __ReportWeakLogic(kExitSceneTimeout, 1, false);
                xinfo2(TSF"weak network end");
                return false;
            }
        }
        return false;
    }
    
    bool WeakNetworkLogic::IsLastValidConnectFail(int64_t &_span) {
        xassert2((last_connect_fail_tick_.isValid() ^ last_connect_suc_tick_.isValid())
            , TSF"last connect status wrong:(%_, %_)", last_connect_fail_tick_.isValid(), last_connect_suc_tick_.isValid());
        if(last_connect_fail_tick_.isValid()) {
            _span = last_connect_fail_tick_.gettickspan();
            return true;
        } else if(last_connect_suc_tick_.isValid()) {
            _span = last_connect_suc_tick_.gettickspan();
            return false;
        }
        return false;
    }
    
    void WeakNetworkLogic::OnConnectEvent(bool _is_suc, int _rtt, int _index) {
        xdebug2(TSF"connect(%_, %_, %_", _is_suc, _rtt, _index);
        if(_is_suc) {
            last_connect_fail_tick_.setInvalid();
            last_connect_suc_tick_.gettickcount();
        }
        else {
            last_connect_fail_tick_.gettickcount();
            last_connect_suc_tick_.setInvalid();
        }
        
        if(!ActiveLogic::Instance()->IsForeground()) {
        	xdebug2(TSF"is background, this:%_", ActiveLogic::Instance().get());
	        return;
        }

        xdebug2(TSF"connect in foreground");
        if(is_curr_weak_)   ++connect_after_weak_;
        if(!_is_suc) {
            if(is_curr_weak_ && last_mark_tick_.gettickspan() >= WEAK_LEAST_SPAN) {
                __MarkWeak(false);
                __ReportWeakLogic(kExitSceneConnect, 1, false);
                if(connect_after_weak_ <= 1 && first_mark_tick_.gettickspan() < SURE_WEAK_SPAN) __ReportWeakLogic(kExitQuickConnectNoNet, 1, false);
                xinfo2(TSF"weak network end");
            }
            return;
        }
        
        bool is_weak = false;
        if(_index > 0)  {
            is_weak = true;
            if(!is_curr_weak_)  __ReportWeakLogic(kSceneIndex, 1, false);
        }
        else if(_rtt > WEAK_CONNECT_RTT) {
            is_weak = true;
            if(!is_curr_weak_)  __ReportWeakLogic(kSceneRtt, 1, false);
        }
        
        if(is_weak) {
            if(!is_curr_weak_) {
                __MarkWeak(true);
                xinfo2(TSF"weak network rtt:%_, index:%_", _rtt, _index);
            } else {
                last_mark_tick_.gettickcount();
            }
        }
    }
    
    void WeakNetworkLogic::OnPkgEvent(bool _is_firstpkg, int _span) {
        if(!ActiveLogic::Instance()->IsForeground())
            return;
        
        bool is_weak = (_span > WEAK_PKG_SPAN);
        if(is_weak) {
            if(!is_curr_weak_) {
                __MarkWeak(true);
                __ReportWeakLogic(_is_firstpkg ? kSceneFirstPkg : kScenePkgPkg, 1, false);
                xinfo2(TSF"weak network span:%_", _span);
            } else {
                last_mark_tick_.gettickcount();
            }
        }
    }
    
    void WeakNetworkLogic::OnTaskEvent(const TaskProfile& _task_profile) {
        if(!ActiveLogic::Instance()->IsForeground())
            return;
        
        bool old_weak = is_curr_weak_;
        bool is_weak = false;
        if(_task_profile.transfer_profile.connect_profile.ip_index > 0 && _task_profile.err_type != kEctOK && _task_profile.err_type != kEctEnDecode) {
            is_weak = true;
            if(!is_curr_weak_)   __ReportWeakLogic(kSceneTask, 1, false);
        }
        else if(_task_profile.err_type == kEctOK && (_task_profile.end_task_time - _task_profile.start_task_time) >= WEAK_TASK_SPAN) {
            is_weak = true;
            if(!is_curr_weak_)  __ReportWeakLogic(kSceneTaskBad, 1, false);
        }
        if(is_weak) {
            if(!is_curr_weak_) {
                __MarkWeak(true);
                xinfo2(TSF"weak network errtype:%_", _task_profile.err_type);
            } else {
                last_mark_tick_.gettickcount();
            }
        } else {
            if(_task_profile.err_type == kEctOK && (_task_profile.end_task_time - _task_profile.start_task_time) < GOOD_TASK_SPAN) {
                if(is_curr_weak_ && last_mark_tick_.gettickspan() >= WEAK_LEAST_SPAN) {
                    __MarkWeak(false);
                    __ReportWeakLogic(kExitSceneTask, 1, false);
                    xinfo2(TSF"weak network end");
                }
            }
        }
        
        if(is_curr_weak_ || old_weak) {
            __ReportWeakLogic(kCGICount, 1, false);
            if(_task_profile.err_type == kEctOK) {
                __ReportWeakLogic(kCGISucc, 1, false);
                __ReportWeakLogic(kCGICost, (int)(_task_profile.end_task_time - _task_profile.start_task_time), false);
            } else {
                ++cgi_fail_num_;
                __ReportWeakLogic(kFailStepDns + _task_profile.GetFailStep() - 1, 1, false);
                __ReportWeakLogic((kFailCurrent + (cgi_fail_num_ >= 4 ? 4 : cgi_fail_num_) - 1), 1, false);
            }
        }
    }

    void WeakNetworkLogic::__MarkWeak(bool _isWeak) {
        if(_isWeak) {
            is_curr_weak_ = true;
            connect_after_weak_ = 0;
            cgi_fail_num_ = 0;
            first_mark_tick_.gettickcount();
            last_mark_tick_.gettickcount();
            __ReportWeakLogic(kEnterWeak, 1, false);
        } else {
            is_curr_weak_ = false;
            __ReportWeakLogic(kExitWeak, 1, false);
            __ReportWeakLogic(kWeakTime, (int)first_mark_tick_.gettickspan(), false);
        }
    }
    
}
}
