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
 * weak_network_logic.h
 *
 *  Created on: 2017-10-20
 *      Author: zhouzhijie
 */

#ifndef weak_network_logic_h
#define weak_network_logic_h

#include "mars/comm/singleton.h"
#include "comm/tickcount.h"
#include "mars/stn/task_profile.h"
#include "mars/baseevent/active_logic.h"

namespace mars {
namespace stn {
    
class WeakNetworkLogic {
public:
    SINGLETON_INTRUSIVE(WeakNetworkLogic, new WeakNetworkLogic, delete);
    boost::function<void (int _key, int _value, bool _is_important)> report_weak_logic_;
    
    bool IsCurrentNetworkWeak();
    void OnConnectEvent(bool _is_suc, int _rtt, int _index);
    void OnPkgEvent(bool _is_firstpkg, int _span);
    void OnTaskEvent(const TaskProfile& _task_profile);
    bool IsLastValidConnectFail(int64_t &_span);
    
private:
    WeakNetworkLogic();
    virtual ~WeakNetworkLogic();
    void __SignalForeground(bool _is_foreground);
    void __ReportWeakLogic(int _key, int _value, bool _is_important);
    void __MarkWeak(bool _isWeak);
    
private:
    tickcount_t first_mark_tick_;
    tickcount_t last_mark_tick_;
    bool is_curr_weak_;
    unsigned int connect_after_weak_;
    tickcount_t last_connect_fail_tick_;
    tickcount_t last_connect_suc_tick_;
    uint32_t cgi_fail_num_;
};
    
}
}

#endif /* weak_network_logic_h */
