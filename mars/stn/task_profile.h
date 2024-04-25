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


#include "mars/comm/time_utils.h"
#include "mars/comm/comm_data.h"
#include "mars/stn/stn.h"
#include "mars/stn/config.h"
#include "mars/comm/socket/unix_socket.h"

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
        nettype_for_report = -1;
        ispcode = 0;
        tid = 0;
        
        start_time = 0;
        dns_time = 0;
        dns_endtime = 0;
        //todo
        ip_items.clear();
        
        conn_reason = 0;
        conn_time = 0;
        conn_errcode = 0;
        rw_errcode = 0;
        ip.clear();
        port = 0;
        host.clear();
        ip_type = kIPSourceNULL;
        conn_rtt = 0;
        conn_cost = 0;
        tryip_count = 0;
        send_request_cost = 0;
        recv_reponse_cost = 0;
        certverify_cost = 0;
        is0rtt = 0;
        is_fast_fallback_tcp = 0;
        quic_rw_timeout_source = TimeoutSource::kClientDefault;
        quic_rw_timeout_ms = 5000;

        local_ip.clear();
        local_port = 0;
        ip_index = -1;
        transport_protocol = Task::kTransportProtocolTCP;
        link_type = Task::kChannelLong;
        tried_443port = 0;
        tried_80port = 0;
        
        disconn_time = 0;
        disconn_errtype = kEctOK;
        disconn_errcode = 0;
        disconn_signal = 0;

        nat64 = false;

        noop_profiles.clear();
        if (extension_ptr)
        		extension_ptr->Reset();
        socket_fd = INVALID_SOCKET;
        keepalive_timeout = 0;
        is_reused_fd = false;
        req_byte_count = 0;
        cgi.clear();
        ipv6_connect_failed = false;
        ipv6only_but_v4_successful = false;
        ipv6_failed_but_v4_successful = false;
        dual_stack_but_use_v4 = false;

        start_connect_time = 0;
        connect_successful_time = 0;
        start_tls_handshake_time = 0;
        tls_handshake_successful_time = 0;
        start_send_packet_time = 0;
        start_read_packet_time = 0;
        read_packet_finished_time = 0;
        retrans_byte_count = 0;
		
        tls_handshake_mismatch = false;
        tls_handshake_success = false;
        channel_type = 0;
        rtt_by_socket = 0;

        task_id = 0;
        
        is_bind_cellular_network = false;
    }
    
    std::string net_type;
    int nettype_for_report;
    int ispcode;
    intmax_t tid;
    
    uint64_t start_time;
    uint64_t dns_time;
    uint64_t dns_endtime;
    std::vector<IPPortItem> ip_items;
    
    int conn_reason;
    uint64_t conn_time;
    int conn_errcode;
    int rw_errcode;
    unsigned int conn_rtt;
    unsigned long conn_cost;
    int tryip_count;
    uint64_t send_request_cost;
    uint64_t recv_reponse_cost;
    int certverify_cost;
    int is0rtt;
    int is_fast_fallback_tcp;
    TimeoutSource quic_rw_timeout_source; //0:client.default; 1: svr.default; 2: cgi.special
    unsigned quic_rw_timeout_ms;

    std::string ip;
    uint16_t port;
    std::string host;
    IPSourceType ip_type;
    std::string local_ip;
    uint16_t local_port;
    int ip_index;
    int transport_protocol;
    int link_type;
    int tried_443port;
    int tried_80port;
    
    uint64_t disconn_time;
    ErrCmdType disconn_errtype;
    int disconn_errcode;
    unsigned int disconn_signal;

    bool nat64;

    std::vector<NoopProfile> noop_profiles;

    std::shared_ptr<ProfileExtension> extension_ptr;
    mars::comm::ProxyInfo proxy_info;

    //keep alive config
    SOCKET socket_fd;
    int (*closefunc)(SOCKET) = &socket_close;
    SOCKET (*createstream_func)(SOCKET) = nullptr;
    bool (*issubstream_func)(SOCKET) = nullptr;
    uint32_t keepalive_timeout;
    bool is_reused_fd;
    int local_net_stack;
    uint64_t req_byte_count;
    std::string cgi; 
    bool ipv6_connect_failed;
    bool ipv6only_but_v4_successful;
    bool ipv6_failed_but_v4_successful;
    bool dual_stack_but_use_v4;
    //opreator identify
    std::string connection_identify;
    bool tls_handshake_mismatch;
    bool tls_handshake_success;
	
	//for cgi caller
    uint64_t start_connect_time;
    uint64_t connect_successful_time;
    uint64_t start_tls_handshake_time;
    uint64_t tls_handshake_successful_time;
    uint64_t start_send_packet_time;
    uint64_t start_read_packet_time;
    uint64_t read_packet_finished_time;
    uint64_t retrans_byte_count;
    int channel_type;
    int rtt_by_socket;

    uint32_t task_id;
    
    bool is_bind_cellular_network;
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
    
    const Task task; //change "const Task& task" to "const Task task". fix a memory reuse bug.
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
    
//do not insert or delete
enum TaskFailStep {
    kStepSucc = 0,
    kStepDns,
    kStepConnect,
    kStepFirstPkg,
    kStepPkgPkg,
    kStepDecode,
    kStepOther,
    kStepTimeout,
    kStepServer,
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
        if (_task.long_polling) {
            task_timeout = (_task.long_polling_timeout + 5 * 1000);
        }
        
        if (0 < _task.total_timeout &&  (uint64_t)_task.total_timeout < task_timeout)
            task_timeout = _task.total_timeout;
        
        return  task_timeout;
    }
    
    TaskProfile(const Task& _task):task(_task), transfer_profile(task), task_timeout(ComputeTaskTimeout(_task)), start_task_time(::gettickcount()){
        
        remain_retry_count = task.retry_count;
        force_no_retry = false;
        
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
        link_type = 0;
        allow_sessiontimeout_retry = true;

        //mars2
        is_weak_network = false;
        is_last_valid_connect_fail = false;
    }
    
    void InitSendParam() {
        transfer_profile.Reset();
        running_id = 0;
    }
    
    void PushHistory() {
        history_transfer_profiles.push_back(transfer_profile);
    }
    
    TaskFailStep GetFailStep() const {
        if(kEctOK == err_type && 0 == err_code) return kStepSucc;
        if(kEctDns == err_type) return kStepDns;
        if(transfer_profile.connect_profile.ip_index == -1) return kStepConnect;
        if(transfer_profile.last_receive_pkg_time == 0) return kStepFirstPkg;
        if(kEctEnDecode == err_type)    return kStepDecode;
        if(kEctSocket == err_type || kEctHttp == err_type || kEctNetMsgXP == err_type)  return kStepPkgPkg;
        if(kEctLocalTaskTimeout == err_code)    return kStepTimeout;
        if(kEctServer == err_type || (kEctOK == err_type && err_code != 0))    return kStepServer;
        return kStepOther;
    }

    Task task;
    TransferProfile transfer_profile;
    intptr_t running_id;
    
    const uint64_t task_timeout;
    const uint64_t start_task_time;  // ms
    uint64_t end_task_time;	//ms
    uint64_t retry_start_time;

    int remain_retry_count;
    bool force_no_retry;
    
    int last_failed_dyntime_status;
    int current_dyntime_status;
    
    bool antiavalanche_checked;
    
    bool use_proxy;
    uint64_t retry_time_interval;    // ms

    ErrCmdType err_type;
    int err_code;
    int link_type;
    bool allow_sessiontimeout_retry;

    std::vector<TransferProfile> history_transfer_profiles;
    std::string channel_name;

    //mars2
    bool is_weak_network;
    bool is_last_valid_connect_fail;
};
        

void __SetLastFailedStatus(std::list<TaskProfile>::iterator _it);
uint64_t __ReadWriteTimeout(uint64_t  _first_pkg_timeout);
uint64_t  __FirstPkgTimeout(int64_t  _init_first_pkg_timeout, size_t _sendlen, int _send_count, int _dynamictimeout_status);
bool __CompareTask(const TaskProfile& _first, const TaskProfile& _second);
}}

#endif

