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
 * net_core.h
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#ifndef STN_SRC_NET_CORE_H_
#define STN_SRC_NET_CORE_H_

#include <vector>
#include <string>
#include <unordered_map>

#include "mars/comm/singleton.h"
#include "mars/comm/messagequeue/message_queue.h"

#include "mars/stn/stn.h"
#include "mars/stn/config.h"
#ifdef USE_LONG_LINK
#include "mars/stn/src/longlink.h"
#include "mars/stn/src/longlink_metadata.h"
#endif

namespace mars {
    
    namespace stn {

class NetSource;

    
class ShortLinkTaskManager;
        
#ifdef USE_LONG_LINK
class LongLinkTaskManager;
class TimingSync;
class ZombieTaskManager;
class NetSourceTimerCheck;
#endif
        
class SignallingKeeper;
class NetCheckLogic;
class DynamicTimeout;
class AntiAvalanche;

enum {
    kCallFromLong,
    kCallFromShort,
    kCallFromZombie,
};

class NetCore {
  public:
    SINGLETON_INTRUSIVE(NetCore, new NetCore, __Release);

  public:
    boost::function<void (Task& _task)> task_process_hook_;
    boost::function<int (int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task)> task_callback_hook_;
    boost::signals2::signal<void (uint32_t _cmdid, const AutoBuffer& _buffer)> push_preprocess_signal_;

  public:
    MessageQueue::MessageQueue_t GetMessageQueueId() { return messagequeue_creater_.GetMessageQueue(); }
    NetSource& GetNetSourceRef() {return *net_source_;}
    
    void    CancelAndWait() { messagequeue_creater_.CancelAndWait(); }
    
    void    StartTask(const Task& _task);
    void    StopTask(uint32_t _taskid);
    bool    HasTask(uint32_t _taskid) const;
    void    ClearTasks();
    void    RedoTasks();
    void    RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id);

    void    MakeSureLongLinkConnect();
    bool    LongLinkIsConnected();
    void    OnNetworkChange();

    void	KeepSignal();
    void	StopSignal();

    ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select);
    void AddServerBan(const std::string& _ip);
    
#ifdef USE_LONG_LINK
    void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLink::TDisconnectInternalCode _code);
    std::shared_ptr<LongLink>        CreateLongLink(const LonglinkConfig& _config);
    void                DestroyLongLink(const std::string& _name);
    void                MakeSureLongLinkConnect_ext(const std::string& _name);
    bool                LongLinkIsConnected_ext(const std::string& _name);
    void                MarkMainLonglink_ext(const std::string& _name);
    std::shared_ptr<LongLink> DefaultLongLink();
    std::shared_ptr<LongLinkMetaData> GetLongLink(const std::string& _name);
    std::shared_ptr<LongLinkMetaData> DefaultLongLinkMeta();
#endif

  private:
    NetCore();
    virtual ~NetCore();
    static void __Release(NetCore* _instance);
    
  private:
    void    __InitLongLink();
    void    __InitShortLink();
    bool    __ValidAndInitDefault(Task& _task, XLogger& _group);
    
    int     __CallBack(int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime);
    void    __OnShortLinkNetworkError(int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port);

    void    __OnShortLinkResponse(int _status_code);

#ifdef USE_LONG_LINK
    void    __OnLongLinkNetworkError(const std::string& _name, int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
    void    __OnLongLinkConnStatusChange(LongLink::TLongLinkStatus _status, const std::string& _channel_id);
#endif
    
    void    __ConnStatusCallBack();
    void    __OnTimerCheckSuc(const std::string& _name);
    
    void    __OnSignalActive(bool _isactive);

    void    __OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend);
  private:
    NetCore(const NetCore&);
    NetCore& operator=(const NetCore&);

  private:
    MessageQueue::MessageQueueCreater           messagequeue_creater_;
    MessageQueue::ScopeRegister                 asyncreg_;
    NetSource*                                  net_source_;
    NetCheckLogic*                              netcheck_logic_;
    AntiAvalanche*                              anti_avalanche_;
    
    DynamicTimeout*                             dynamic_timeout_;
    ShortLinkTaskManager*                       shortlink_task_manager_;
    int                                         shortlink_error_count_;

#ifdef USE_LONG_LINK
    ZombieTaskManager*                          zombie_task_manager_;
    LongLinkTaskManager*                        longlink_task_manager_;
    
    TimingSync*                                 timing_sync_;
#endif
    
    bool                                        shortlink_try_flag_;
};
        
}}

#endif // STN_SRC_NET_CORE_H_
