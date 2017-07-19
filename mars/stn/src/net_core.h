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

#include "mars/comm/singleton.h"
#include "mars/comm/messagequeue/message_queue.h"

#include "mars/stn/stn.h"
#include "mars/stn/config.h"
#ifdef USE_LONG_LINK
#include "mars/stn/src/longlink.h"
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
    void    CancelAndWait() { messagequeue_creater_.CancelAndWait(); }
    
    void    StartTask(const Task& _task);
    void    StopTask(uint32_t _taskid);
    bool    HasTask(uint32_t _taskid) const;
    void    ClearTasks();
    void    RedoTasks();
    void    RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid);

    void    MakeSureLongLinkConnect();
    bool    LongLinkIsConnected();
    void    OnNetworkChange();

    void	KeepSignal();
    void	StopSignal();

#ifdef USE_LONG_LINK
    LongLink& Longlink();
#endif

  private:
    NetCore();
    virtual ~NetCore();
    static void __Release(NetCore* _instance);
    
  private:
    int     __CallBack(int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime);
    void    __OnShortLinkNetworkError(int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port);

    void    __OnShortLinkResponse(int _status_code);

#ifdef USE_LONG_LINK
    void    __OnLongLinkNetworkError(int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
    void    __OnLongLinkConnStatusChange(LongLink::TLongLinkStatus _status);
    void    __ResetLongLink();
#endif
    
    void    __ConnStatusCallBack();
    void    __OnTimerCheckSuc();
    
    void    __OnSignalActive(bool _isactive);

  private:
    NetCore(const NetCore&);
    NetCore& operator=(const NetCore&);

  private:
    MessageQueue::MessageQueueCreater   messagequeue_creater_;
    MessageQueue::ScopeRegister         asyncreg_;
    NetSource*                          net_source_;
    NetCheckLogic*                      netcheck_logic_;
    AntiAvalanche*                      anti_avalanche_;
    
    DynamicTimeout*                     dynamic_timeout_;
    ShortLinkTaskManager*               shortlink_task_manager_;
    int                                 shortlink_error_count_;

#ifdef USE_LONG_LINK
    ZombieTaskManager*                  zombie_task_manager_;
    LongLinkTaskManager*                longlink_task_manager_;
    SignallingKeeper*                   signalling_keeper_;
    NetSourceTimerCheck*                netsource_timercheck_;
    TimingSync*                         timing_sync_;
#endif

    bool                                shortlink_try_flag_;

};
        
}}

#endif // STN_SRC_NET_CORE_H_
