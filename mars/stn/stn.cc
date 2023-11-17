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
//  stn.cpp
//  stn
//
//  Created by yanguoyue on 16/3/3.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "mars/stn/stn.h"

#include "mars/boost/config.hpp"
#include "mars/comm/thread/atomic_oper.h"
/* mars2
static const uint32_t kReservedTaskIDStart = 0xFFFFFFF0;
*/

namespace mars {
namespace stn {

static uint32_t gs_taskid = 1;
Task::Task() : Task(atomic_inc32(&gs_taskid)) {
}

Task::Task(uint32_t _taskid) {
    taskid = _taskid;
    cmdid = 0;
    channel_id = 0;
    channel_select = 0;
    transport_protocol = kTransportProtocolTCP;

    send_only = false;
    need_authed = false;
    limit_flow = true;
    limit_frequency = true;

    channel_strategy = kChannelNormalStrategy;
    network_status_sensitive = false;
    priority = kTaskPriorityNormal;

    retry_count = -1;
    server_process_cost = -1;
    total_timeout = -1;
    user_context = NULL;
    long_polling = false;
    long_polling_timeout = -1;

    channel_name = DEFAULT_LONGLINK_NAME;
    max_minorlinks = 1;

    redirect_type = HostRedirectType::kOther;

    client_sequence_id = 0;
    server_sequence_id = 0;

    need_realtime_netinfo = false;
}

/* mars2
uint32_t GenTaskID(){
    if (BOOST_UNLIKELY(atomic_read32(&gs_taskid) >= kReservedTaskIDStart)) {
        atomic_write32(&gs_taskid, 1);
    }
    return atomic_inc32(&gs_taskid);
}
*/

}  // namespace stn
}  // namespace mars
