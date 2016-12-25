//
//  stn.cpp
//  stn
//
//  Created by yanguoyue on 16/3/3.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "mars/stn/stn.h"

#include "mars/comm/thread/atomic_oper.h"


namespace mars{
    namespace stn{
        
Task::Task() {
    
#ifndef ANDROID
    static uint32_t s_taskid = 1;
    taskid = atomic_inc32(&s_taskid);
#else
    taskid = 0;
#endif
    
    cmdid = 0;
    channel_select = 0;
    
    send_only = false;
    need_authed = false;
    limit_flow = true;
    limit_frequency = true;
    
    channel_strategy = kChannelNormalStrategy;
    network_status_sensitive = false;
    priority = kTaskPriorityNormal;
    
    retry_count = -1;
    server_process_cost = -1;
    total_timetout = -1;
    user_context = NULL;

}
        
    }
}
