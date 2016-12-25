//
//  task_profile.c
//  stn
//
//  Created by yerungui on 16/3/28.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/platform_comm.h"
#include "mars/stn/task_profile.h"

#include "dynamic_timeout.h"

namespace mars {
namespace stn {

void __SetLastFailedStatus(std::list<TaskProfile>::iterator _it){
    if (_it->remain_retry_count > 0) {
        _it->last_failed_dyntime_status = _it->current_dyntime_status;
    }
}

 uint64_t __ReadWriteTimeout(uint64_t  _first_pkg_timeout) {
    uint64_t rate = (kMobile != getNetInfo()) ? kWifiMinRate : kGPRSMinRate;
    return  _first_pkg_timeout + 1000 * kMaxRecvLen / rate;
}

uint64_t  __FirstPkgTimeout(int64_t  _init_first_pkg_timeout, size_t _sendlen, int _send_count, int _dynamictimeout_status) {
    xassert2(3600 * 1000 >= _init_first_pkg_timeout, TSF"server_cost:%_ ", _init_first_pkg_timeout);
    
    uint64_t ret = 0;
    uint64_t task_delay = (kMobile != getNetInfo()) ? kWifiTaskDelay : kGPRSTaskDelay;
    
    if (_dynamictimeout_status == kExcellent && _init_first_pkg_timeout == 0) {
        ret = (kMobile != getNetInfo()) ? kDynTimeFirstPackageWifiTimeout : kDynTimeFirstPackageGPRSTimeout;
        ret += _send_count * task_delay;
    }
    else{
        uint64_t rate = (kMobile != getNetInfo()) ? kWifiMinRate : kGPRSMinRate;
        uint64_t base_rw_timeout = (kMobile != getNetInfo()) ? kBaseFirstPackageWifiTimeout : kBaseFirstPackageGPRSTimeout;
        uint64_t max_rw_timeout = (kMobile != getNetInfo()) ? kMaxFirstPackageWifiTimeout : kMaxFirstPackageGPRSTimeout;
        
        if (0 < _init_first_pkg_timeout) {
            ret = _init_first_pkg_timeout + 1000 * _sendlen / rate;
        } else {
            ret =     base_rw_timeout + 1000 * _sendlen / rate;
            ret = ret < max_rw_timeout ? ret : max_rw_timeout;
        }
        
        ret += _send_count * task_delay;
    }
    
    return ret;
}

bool __CompareTask(const TaskProfile& _first, const TaskProfile& _second) {
    return _first.task.priority < _second.task.priority;
}

}}
