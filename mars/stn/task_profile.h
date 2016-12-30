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
//  task_profile.h
//  stn
//
//  Created by yerungui on 16/3/23.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef TASK_PROFILE_H_
#define TASK_PROFILE_H_

#include <list>
#include <sstream>

#include "boost/shared_ptr.hpp"

#include "mars/comm/time_utils.h"
#include "mars/stn/stn.h"
#include "mars/stn/config.h"

namespace mars {
namespace stn  {

struct ProfileExtension {

	ProfileExtension() {}
	virtual ~ProfileExtension() {}

	virtual void Reset() {}
};

struct NoopProfile {

    NoopProfile() {
    	Reset();
    }

    void Reset() {
        success = false;
        noop_internal = 0;
        noop_actual_internal = 0;
    	noop_cost = 0;
        noop_starttime = 0;
    }

    bool     success;
    uint64_t noop_internal;
    uint64_t noop_actual_internal;
    uint64_t noop_cost;
    uint64_t noop_starttime;
};


struct ConnectProfile {
    
    ConnectProfile() {
        Reset();
    }
    
    void Reset(){
        net_type.clear();
        tid = 0;
        
        start_time = 0;
        dns_time = 0;
        dns_endtime = 0;
        //todo
        ip_items.clear();
        
        conn_reason = 0;
        conn_time = 0;
        conn_errcode = 0;
        ip.clear();
        port = 0;
        host.clear();
        ip_type = kIPSourceNULL;
        conn_rtt = 0;
        conn_cost = 0;
        tryip_count = 0;

        local_ip.clear();
        ip_index = -1;
        
        disconn_time = 0;
        disconn_errtype = kEctOK;
        disconn_errcode = 0;
        disconn_signal = 0;

        nat64 = false;

        noop_profiles.clear();
        if (extension_ptr)
        		extension_ptr->Reset();
    }
    
    std::string net_type;
    intmax_t tid;
    
    uint64_t start_time;
    uint64_t dns_time;
    uint64_t dns_endtime;
    std::vector<IPPortItem> ip_items;
    
    int conn_reason;
    uint64_t conn_time;
    int conn_errcode;
    unsigned int conn_rtt;
    unsigned long conn_cost;
    int tryip_count;

    std::string ip;
    uint16_t port;
    std::string host;
    IPSourceType ip_type;
    std::string local_ip;
    int ip_index;
    
    uint64_t disconn_time;
    ErrCmdType disconn_errtype;
    int disconn_errcode;
    unsigned int disconn_signal;

    bool nat64;

    std::vector<NoopProfile> noop_profiles;

    boost::shared_ptr<ProfileExtension> extension_ptr;
};

        
struct TransferProfile {
    TransferProfile(const Task& _task):task(_task){
        Reset();
    }
    
    void Reset() {
        connect_profile.Reset();
        loop_start_task_time = 0;
        first_start_send_time = 0;
        start_send_time = 0;
        last_receive_pkg_time = 0;
        read_write_timeout = 0;
        first_pkg_timeout = 0;
        
        sent_size = 0;
        send_data_size = 0;
        received_size = 0;
        receive_data_size = 0;
        
        external_ip.clear();
        
        error_type = 0;
        error_code = 0;
    }
    
    const Task& task;
    ConnectProfile connect_profile;
    
    uint64_t loop_start_task_time;  // ms
    uint64_t first_start_send_time; //ms
    uint64_t start_send_time;    // ms
    uint64_t last_receive_pkg_time;  // ms
    uint64_t read_write_timeout;    // ms
    uint64_t first_pkg_timeout;  // ms
    
    size_t sent_size;
    size_t send_data_size;
    size_t received_size;
    size_t receive_data_size;
    
    std::string external_ip;
    
    int error_type;
    int error_code;
};
        
struct TaskProfile {
    
    static uint64_t ComputeTaskTimeout(const Task& _task) {
        uint64_t readwritetimeout = 15 * 1000;
        
        if (0 < _task.server_process_cost)
            readwritetimeout = _task.server_process_cost + 15 * 1000;
        
        int trycount = 0;// DEF_TASK_RETRY_COUNT;
        
        if (0 <= _task.retry_count)
            trycount = _task.retry_count;
        
        trycount++;
        
        uint64_t task_timeout = (readwritetimeout + 5 * 1000) * trycount;
        
        if (0 < _task.total_timetout &&  (uint64_t)_task.total_timetout < task_timeout)
            task_timeout = _task.total_timetout;
        
        return  task_timeout;
    }
    
    TaskProfile(const Task& _task):task(_task), transfer_profile(task), task_timeout(ComputeTaskTimeout(_task)), start_task_time(::gettickcount()){
        
        remain_retry_count = task.retry_count;
        
        running_id = 0;
        
        end_task_time = 0;
        retry_start_time = 0;

        last_failed_dyntime_status = 0;
        current_dyntime_status = 0;
        
        antiavalanche_checked = false;
        
        use_proxy = false;
        retry_time_interval = 0;

        err_type = kEctOK;
        err_code = 0;
    }
    
    void InitSendParam() {
        transfer_profile.Reset();
        running_id = 0;
    }
    
    void PushHistory() {
        history_transfer_profiles.push_back(transfer_profile);
    }

    const Task task;
    TransferProfile transfer_profile;
    intptr_t running_id;
    
    const uint64_t task_timeout;
    const uint64_t start_task_time;  // ms
    uint64_t end_task_time;	//ms
    uint64_t retry_start_time;

    int remain_retry_count;
    
    int last_failed_dyntime_status;
    int current_dyntime_status;
    
    bool antiavalanche_checked;
    
    bool use_proxy;
    uint64_t retry_time_interval;    // ms

    ErrCmdType err_type;
    int err_code;
    int link_type;

    std::vector<TransferProfile> history_transfer_profiles;
};
        

void __SetLastFailedStatus(std::list<TaskProfile>::iterator _it);
uint64_t __ReadWriteTimeout(uint64_t  _first_pkg_timeout);
uint64_t  __FirstPkgTimeout(int64_t  _init_first_pkg_timeout, size_t _sendlen, int _send_count, int _dynamictimeout_status);
bool __CompareTask(const TaskProfile& _first, const TaskProfile& _second);
}}

#endif

