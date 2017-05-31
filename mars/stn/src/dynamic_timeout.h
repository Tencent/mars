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
*  dynamic_timeout.h
*  network
*
*  Created by caoshaokun on 15/10/28.
*  Copyright © 2015 Tencent. All rights reserved.
*/

#ifndef STN_SRC_DYNAMIC_TIMEOUT_H_
#define STN_SRC_DYNAMIC_TIMEOUT_H_

#include <bitset>
#include <string>

enum DynamicTimeoutStatus {
    kEValuating = 1,
    kExcellent,
    kBad
};

namespace mars {
    namespace stn {

class DynamicTimeout {
    
  public:
    DynamicTimeout();
    virtual ~DynamicTimeout();
    
    void ResetStatus();
    
    void CgiTaskStatistic(std::string _cgi_uri, unsigned int _total_size, uint64_t _cost_time);
    
    int GetStatus();
    
  private:
    void __StatusSwitch(std::string _cgi_uri, int _task_status);
    
  private:
    int                     dyntime_status_;
    unsigned int            dyntime_continuous_good_count_;
    unsigned long           dyntime_latest_bigpkg_goodtime_;  //ms
    std::bitset<10>         dyntime_failed_normal_count_;
    unsigned long           dyntime_fncount_latstmodify_time_;    //ms
    size_t                  dyntime_fncount_pos_;
};
        
    }
}

#endif // STN_SRC_DYNAMIC_TIMEOUT_H_
