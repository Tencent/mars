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
 * shortlink_interface.h
 *
 *  Created on: Jul 21, 2016
 *      Author: wutianqiang
 */

#ifndef SRC_SHORTLINK_INTERFACE_H_
#define SRC_SHORTLINK_INTERFACE_H_

#include "mars/comm/autobuffer.h"
#include "mars/comm/messagequeue/callback.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

class ShortLinkInterface {
 public:
    virtual ~ShortLinkInterface() {
        func_add_weak_net_info = NULL;
        func_weak_net_report = NULL;
    };
    virtual void OnStart() = 0;
    virtual void SendRequest(AutoBuffer& _buffer_req, AutoBuffer& _buffer_extend) = 0;
    virtual ConnectProfile Profile() const {
        return ConnectProfile();
    }
    virtual bool IsKeepAlive() const {
        return false;
    }
    virtual void SetUseProtocol(int _protocol) {
    }
    virtual void SetDebugHost(const std::string& _host) {
    }
    virtual void OnConnectHandshakeCompleted() {
    }
    virtual void OnNetTimeout() {
    }
    virtual void SetRunByTaskManager(bool flag) = 0;
    virtual bool IsRunByTaskManager() const {
        return true;
    }

    virtual bool OnSingleRespHandle(TaskProfile& _task_profile,
                                    ErrCmdType _err_type,
                                    int _err_code,
                                    int _fail_handle,
                                    size_t _resp_length,
                                    const ConnectProfile& _connect_profile) = 0;

    CallBack<boost::function<void(int _line,
                                  ErrCmdType _errtype,
                                  int _errcode,
                                  const std::string& _ip,
                                  const std::string& _host,
                                  uint16_t _port)> >
        func_network_report;  // message queen
    CallBack<boost::function<void(ShortLinkInterface* _worker,
                                  ErrCmdType _err_type,
                                  int _status,
                                  AutoBuffer& _body,
                                  AutoBuffer& _extension,
                                  bool _cancel_retry,
                                  ConnectProfile& _conn_profile)> >
        OnResponse;
    // CallBack<boost::function<void(ShortLinkInterface* _worker)> > OnSend;
    // CallBack<boost::function<void(ShortLinkInterface* _worker, unsigned int _cached_size, unsigned int _total_size)>
    // >
    //   OnRecv;
    boost::function<void(uint32_t _tls_version, mars::stn::TlsHandshakeFrom _from)> OnHandshakeCompleted;
    boost::function<SOCKET(const IPPortItem& _address)> GetCacheSocket;

    std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist)> func_host_filter;
    std::function<void(bool _connect_timeout, struct tcp_info& _info)> func_add_weak_net_info = NULL;
    std::function<void(bool _timeout, struct tcp_info& _info)> func_weak_net_report = NULL;
    //    CallBack<boost::function<bool(ShortLinkInterface* _worker,
    //                                  TaskProfile& _task_profile,
    //                                  ErrCmdType _err_type,
    //                                  int _err_code,
    //                                  int _fail_handle,
    //                                  size_t _resp_length,
    //                                  const ConnectProfile& _connect_profile)> >
    //        OnSingleRespHandle;
    boost::function<bool(const Task& _task, const void* _buffer, int _len)> fun_anti_avalanche_check_;
    std::function<bool(const std::string& _name, std::string& _last_data)> OnGetInterceptTaskInfo;
    std::function<int()> OnGetStatus;
    boost::function<void(int _status_code)> fun_shortlink_response_;
    boost::function<
        void(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id)>
        fun_notify_retry_all_tasks;
    boost::function<void(int _line,
                         ErrCmdType _err_type,
                         int _err_code,
                         const std::string& _ip,
                         const std::string& _host,
                         uint16_t _port)>
        fun_notify_network_err_;
    std::function<void(std::string _cgi_uri, unsigned int _total_size, uint64_t _cost_time)> OnCgiTaskStatistic;
    std::function<bool(int _error_code)> should_intercept_result_;
    std::function<void(const std::string& _name, const std::string& _data)> OnAddInterceptTask;
    std::function<void(bool _is_reused, bool _has_received, bool _is_decode_ok)> OnSocketPoolReport;
    std::function<void(IPPortItem item, ConnectProfile& _conn_profile)> OnSocketPoolTryAddCache;

    std::function<void(const int _error_type, const int _error_code, const int _use_ip_index)> task_connection_detail_;
    boost::function<
        int(ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime)>
        fun_callback_;
    std::function<void(const TaskProfile& _profile)> on_timeout_or_remote_shutdown_;
    std::function<void(intptr_t& _running_id)> on_delete_shortlink_;
    std::function<void(intptr_t& _running_id)> on_delete_shortlink_erase_cmd_list_;
    std::function<void(bool _user_proxy)> on_set_use_proxy_;
    std::function<void()> on_reset_fail_count_;
    std::function<void()> on_increase_fail_count_;
};

}  // namespace stn
}  // namespace mars

#endif /* SRC_SHORTLINK_INTERFACE_H_ */
