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
 * net_check_logic.h
 *
 *  Created on: 2014-7-1
 *      Author: yanguoyue
 */

#ifndef STN_SRC_NET_CHECK_LOGIC_H_
#define STN_SRC_NET_CHECK_LOGIC_H_

#include <list>
#include <string>

#include "net_source.h"

class CommFrequencyLimit;

namespace mars {
    namespace stn {
    
class NetSource;

class NetCheckLogic {
  public:
    NetCheckLogic();
    ~NetCheckLogic();

    void UpdateLongLinkInfo(unsigned int _continues_fail_count, bool _task_succ);
    void UpdateShortLinkInfo(unsigned int _continue_fail_count, bool _task_succ);

  private:
    struct NetTaskStatusItem{
		uint32_t records;
		uint64_t last_failedtime;
		NetTaskStatusItem(): records(0xFFFFFFFF), last_failedtime(0) {}
	};

  private:
    bool __ShouldNetCheck();
    void __StartNetCheck();

  private:

    CommFrequencyLimit* frequency_limit_;
    NetSource::DnsUtil dns_util_;

    unsigned long long last_netcheck_time_;

    NetTaskStatusItem longlink_taskstatus_item_;;
    NetTaskStatusItem shortlink_taskstatus_item_;
};

    }
}

#endif // STN_SRC_NET_CHECK_LOGIC_H_
