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
 * longlink_task_manager.h
 *
 *  Created on: 2012-7-17
 *      Author: yerungui
 */

#ifndef STN_SRC_LONGLINK_TASK_MANAGER_H_
#define STN_SRC_LONGLINK_TASK_MANAGER_H_

#include <list>
#include <stdint.h>
#include <set>

#include "boost/function.hpp"

#include "mars/comm/messagequeue/message_queue.h"
#include "mars/stn/stn.h"

#include "longlink_metadata.h"
#include "task_intercept.h"
#include "mars/boot/context.h"
#include "cellular_network_manager.h"
class AutoBuffer;

struct STChannelResp;

namespace mars {
namespace comm {
class ActiveLogic;
#ifdef ANDROID
class WakeUpLock;
#endif
}
    namespace stn {

struct TaskProfile;
class DynamicTimeout;
class LongLinkConnectMonitor;

class LongLinkTaskManager {
  public:
    boost::function<int (ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime)> fun_callback_;

    boost::function<void (ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, const std::string& _user_id)> fun_notify_retry_all_tasks;
    boost::function<void (const std::string& _name, int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port)> fun_notify_network_err_;
    boost::function<bool (const Task& _task, const void* _buffer, int _len)> fun_anti_avalanche_check_;
    
    boost::function<void (const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend)> fun_on_push_;
    
    boost::function<size_t (const std::string& _user_id, std::vector<std::string>& _host_list, bool _strict_match)> get_real_host_;
    boost::function<void (uint32_t _version, mars::stn::TlsHandshakeFrom _from)> on_handshake_ready_;
    boost::function<bool (int _error_code)> should_intercept_result_;

  public:
    LongLinkTaskManager(mars::boot::Context* _context, std::shared_ptr<NetSource> _netsource, comm::ActiveLogic& _activelogic, DynamicTimeout& _dynamictimeout, comm::MessageQueue::MessageQueue_t  _messagequeueid);
    virtual ~LongLinkTaskManager();

    bool StartTask(const Task& _task, int _channel);
    bool StopTask(uint32_t _taskid);
    bool HasTask(uint32_t _taskid) const;
    void ClearTasks();
    void RedoTasks();
    void TouchTasks();
    void RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id);

    // LongLink& LongLinkChannel(const std::string& _name) { return *longlink_; }
    // LongLinkConnectMonitor& getLongLinkConnectMonitor(const std::) { return *longlinkconnectmon_; }
    std::shared_ptr<LongLinkMetaData> GetLongLink(const std::string& _name);
    std::shared_ptr<LongLinkMetaData> GetLongLinkNoLock(const std::string& _name);
    void FixMinorRealhost(Task& _task);

    unsigned int GetTaskCount(const std::string& _name);
    unsigned int GetTasksContinuousFailCount();

    bool AddLongLink(LonglinkConfig& _config);
    bool AddMinorLink(const std::vector<std::string>& _hosts);
    bool IsMinorAvailable(const Task& _task);

    std::shared_ptr<LongLinkMetaData> DefaultLongLink();
    void OnNetworkChange();
    ConnectProfile GetConnectProfile(uint32_t _taskid);
    void ReleaseLongLink(const std::string _name);
    void ReleaseLongLink(std::shared_ptr<LongLinkMetaData> _linkmeta);
    bool DisconnectByTaskId(uint32_t _taskid, LongLinkErrCode::TDisconnectInternalCode _code);
    void AddForbidTlsHost(const std::vector<std::string>& _host);

  private:
    // from ILongLinkObserver
    void __OnResponse(const std::string& _name, ErrCmdType _error_type, int _error_code, uint32_t _cmdid, uint32_t _taskid, AutoBuffer& _body, AutoBuffer& _extension, const ConnectProfile& _connect_profile);
    void __OnSend(uint32_t _taskid);
    void __OnRecv(uint32_t _taskid, size_t _cachedsize, size_t _totalsize);
    void __SignalConnection(LongLink::TLongLinkStatus _connect_status, const std::string& _channel_id);
    void __OnHandshakeCompleted(uint32_t _version, mars::stn::TlsHandshakeFrom _from);

    void __RunLoop();
    void __RunOnTimeout();
    void __RunOnStartTask();

    void __BatchErrorRespHandle(const std::string _channel_name, ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, bool _callback_runing_task_only = true);
    bool __SingleRespHandle(std::list<TaskProfile>::iterator _it, ErrCmdType _err_type, int _err_code, int _fail_handle, const ConnectProfile& _connect_profile);
    void __BatchErrorRespHandleByUserId(const std::string& _user_id, ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, bool _callback_runing_task_only = true);

    std::list<TaskProfile>::iterator __Locate(uint32_t  _taskid);
#ifdef __APPLE__
    void __ResetLongLink(const std::string& _name);
#endif
    void __Disconnect(const std::string& _name, LongLinkErrCode::TDisconnectInternalCode code);
    void __RedoTasks(const std::string& _name, bool need_lock_link = true);
    void __DumpLongLinkChannelInfo();
    bool __ForbidUseTls(const std::vector<std::string>& _host_list);
    ConnectProfile __GetConnectionProfile(std::shared_ptr<LongLinkMetaData> longlink);
    
  private:
    boot::Context* context_;
    comm::MessageQueue::ScopeRegister     asyncreg_;
    std::list<TaskProfile>          lst_cmd_;
    uint64_t                        lastbatcherrortime_;   // ms
    unsigned long                   retry_interval_;	//ms
    unsigned int                    tasks_continuous_fail_count_;

    std::map<std::string, std::shared_ptr<LongLinkMetaData> > longlink_metas_;
    std::map<std::string, int>      longlink_id_;
    DynamicTimeout&                 dynamic_timeout_;

    std::shared_ptr<NetSource>      netsource_;
    comm::ActiveLogic&              active_logic_;

#ifdef ANDROID
    comm::WakeUpLock*                     wakeup_lock_;
#endif
#ifndef _WIN32
    typedef  comm::ScopedLock MetaScopedLock;
    comm::Mutex                     meta_mutex_;
#else // _WIN32
    typedef  comm::ScopedRecursiveLock MetaScopedLock;
    comm::RecursiveMutex            meta_mutex_;
#endif
    comm::Mutex                     mutex_;
    /*NO_DESTROY static */std::set<std::string>    forbid_tls_host_;
    TaskIntercept                   task_intercept_;
    bool already_release_manager_ = false;
    CellularNetworkManager* cellular_network_manager_;
};
    }
}

#endif // STN_SRC_LONGLINK_TASK_MANAGER_H_
