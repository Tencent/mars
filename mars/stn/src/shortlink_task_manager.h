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
 * shortlink_task_manager.h
 *
 *  Created on: 2012-8-24
 *      Author: zhouzhijie
 */

#ifndef STN_SRC_SHORTLINK_TASK_MANAGER_H_
#define STN_SRC_SHORTLINK_TASK_MANAGER_H_

#include <stdint.h>

#include <functional>
#include <list>
#include <map>

#include "boost/function.hpp"
#include "mars/boot/context.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"
#include "shortlink.h"
#include "socket_pool.h"
#include "task_intercept.h"

class AutoBuffer;

namespace mars {

namespace comm {
#ifdef ANDROID
class WakeUpLock;
#endif
}  // namespace comm

namespace stn {

class DynamicTimeout;

class ShortLinkTaskManager {
 public:
    boost::function<
        int(ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime)>
        fun_callback_;
    boost::function<void(int _line,
                         ErrCmdType _err_type,
                         int _err_code,
                         const std::string& _ip,
                         const std::string& _host,
                         uint16_t _port)>
        fun_notify_network_err_;
    boost::function<bool(const Task& _task, const void* _buffer, int _len)> fun_anti_avalanche_check_;
    boost::function<void(int _status_code)> fun_shortlink_response_;
    boost::function<
        void(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id)>
        fun_notify_retry_all_tasks;

    std::function<size_t(const std::string& _user_id,
                         std::vector<std::string>& _host_list,
                         const std::map<std::string, std::string>& extra_info)>
        get_real_host_strict_match_;

    std::function<void(bool _connect_timeout, struct tcp_info& _info)> add_weaknet_info_;

    std::function<size_t(const std::string& _user_id,
                         std::vector<std::string>& _host_list,
                         bool _strict_match,
                         const std::map<std::string, std::string>& extra_info)>
        get_real_host_;

    std::function<void(const int _error_type,
                       const int _error_code,
                       const int _use_ip_index,
                       const std::map<std::string, std::string>& extra_info)>
        task_connection_detail_;

    std::function<int(TaskProfile& _profile)> choose_protocol_;
    std::function<void(const TaskProfile& _profile)> on_timeout_or_remote_shutdown_;
    std::function<void(uint32_t _version, mars::stn::TlsHandshakeFrom _from)> on_handshake_ready_;
    std::function<bool(const std::vector<std::string> _host_list)> can_use_tls_;
    std::function<bool(int _error_code)> should_intercept_result_;

 public:
    ShortLinkTaskManager(boot::Context* _context,
                         std::shared_ptr<NetSource> _netsource,
                         DynamicTimeout& _dynamictimeout,
                         comm::MessageQueue::MessageQueue_t _messagequeueid,
                         std::string tls_group_name = "default");
    virtual ~ShortLinkTaskManager();

    bool StartTask(const Task& _task, PrepareProfile _prepare_profile);
    bool StopTask(uint32_t _taskid);
    bool HasTask(uint32_t _taskid) const;
    void ClearTasks();
    void RedoTasks();
    void TouchTasks();
    void RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid);
    void SetDebugHost(const std::string& _host) {
        debug_host_ = _host;
    }

    unsigned int GetTasksContinuousFailCount();

    ConnectProfile GetConnectProfile(uint32_t _taskid) const;

 private:
    void __RunLoop();
    void __RunOnTimeout();
    void __RunOnStartTask();

    void __OnResponse(ShortLinkInterface* _worker,
                      ErrCmdType _err_type,
                      int _status,
                      AutoBuffer& _body,
                      AutoBuffer& _extension,
                      bool _cancel_retry,
                      ConnectProfile& _conn_profile);
    void __OnSend(ShortLinkInterface* _worker);
    void __OnRecv(ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size);

    void __BatchErrorRespHandle(ErrCmdType _err_type,
                                int _err_code,
                                int _fail_handle,
                                uint32_t _src_taskid,
                                bool _callback_runing_task_only = true);
    bool __SingleRespHandle(std::list<TaskProfile>::iterator _it,
                            ErrCmdType _err_type,
                            int _err_code,
                            int _fail_handle,
                            size_t _resp_length,
                            const ConnectProfile& _connect_profile);
    bool __SingleRespHandleByWorker(ShortLinkInterface* _worker,
                                    ErrCmdType _err_type,
                                    int _err_code,
                                    int _fail_handle,
                                    size_t _resp_length,
                                    const ConnectProfile& _connect_profile);

    std::list<TaskProfile>::iterator __LocateBySeq(intptr_t _running_id);

    bool __DeleteShortLink(std::list<TaskProfile>::iterator _it);
    SOCKET __OnGetCacheSocket(const IPPortItem& _address);
    void __OnHandshakeCompleted(uint32_t _version, mars::stn::TlsHandshakeFrom _from);
    void __OnRequestTimeout(ShortLinkInterface* _worker, int _errorcode);
    void __OnAddWeakNetInfo(bool _connect_timeout, struct tcp_info& _info);

 private:
    bool __GetInterceptTaskInfo(const std::string& _name, std::string& _last_data);
    int __OnGetStatus();
    // void __OnCgiTaskStatistic(std::string _cgi_uri, unsigned int _total_size, uint64_t _cost_time);
    void __OnCgiTaskStatistic(ShortLinkInterface* _worker, unsigned int body_length);
    void __OnAddInterceptTask(const std::string& _name, const std::string& _data);
    void __OnSocketPoolReport(bool _is_reused, bool _has_received, bool _is_decode_ok);
    void __OnSocketPoolTryAdd(IPPortItem item, ConnectProfile& _conn_profile);

    void __OnSetUserProxy(bool _user_proxy);
    void __OnResetFailCount();
    void __OnInCreaseFailCount();
    void __OnReq2BufTime(ShortLinkInterface* _worker, uint64_t begin_req2buf_time, uint64_t end_req2buf_time);
    void __OnBuf2RespTime(ShortLinkInterface* _worker, uint64_t begin_buf2resp_time, uint64_t end_buf2resp_time);

    void __OnClientSequenceId(ShortLinkInterface* _worker, int client_sequence_id);
    void __OnServerSequenceId(ShortLinkInterface* _worker, int server_sequence_id);

    void __OnRecvDataTime(ShortLinkInterface* _worker, size_t receive_data_size, uint64_t last_receive_pkg_time);
    void __OnUpdateTimeout(ShortLinkInterface* _worker,
                           uint64_t loop_start_task_time,
                           uint64_t first_pkg_timeout,
                           uint64_t read_write_timeout,
                           size_t send_data_size,
                           int current_dyntime_status);
    void __OnSetForceNoRetry(ShortLinkInterface* _worker, bool force_no_retry);
    void __OnIncreateRemainRetryCount(ShortLinkInterface* _worker, bool before);
    void __OnSetLastFailedStatus(ShortLinkInterface* _worker);
    void __OnUpdateConnectProfile(ShortLinkInterface* worker, ConnectProfile& connect_profile);
    // int __OnGetSendCount();
    void __OnTotalCheckAuthTime(ShortLinkInterface* _worker,
                                uint64_t begin_check_auth_time,
                                uint64_t end_check_auth_time);
    void __OnMakeSureAuthTime(ShortLinkInterface* _worker,
                              uint64_t begin_make_sure_auth_time,
                              uint64_t end_make_sure_auth_time);
    void __CheckAuthAndNotify(std::list<TaskProfile>::iterator _it);
    void __OnSetFirstAuthFlag(ShortLinkInterface* _worker, FirstAuthFlag first_auth_flag);

 private:
    boot::Context* context_;
    comm::MessageQueue::ScopeRegister asyncreg_;
    std::shared_ptr<NetSource> net_source_;

    std::list<TaskProfile> lst_cmd_;

    bool default_use_proxy_;
    unsigned int tasks_continuous_fail_count_;
    DynamicTimeout& dynamic_timeout_;
    std::string debug_host_;
#ifdef ANDROID
    comm::WakeUpLock* wakeup_lock_;
#endif
    SocketPool socket_pool_;
    TaskIntercept task_intercept_;
    bool already_release_manager_ = false;
    std::string m_tls_group_name_ = "default";
    bool is_handle_reqresp_buff_in_worker_ = false;  // do req2buf and buf2resp on worker thread
};

}  // namespace stn
}  // namespace mars

#endif  // STN_SRC_SHORTLINK_TASK_MANAGER_H_
