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

#include "mars/boost/shared_ptr.hpp"
#include "mars/boost/weak_ptr.hpp"
#include "mars/boost/signals2.hpp"
#include "mars/boost/function.hpp"
#include "mars/comm/socket/getsocktcpinfo.h"

#include "mars/boot/context.h"

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
//    SINGLETON_INTRUSIVE(NetCore, new NetCore, __Release);
    
    static boost::signals2::signal<void ()>& NetCoreCreateBegin() {
        static boost::signals2::signal<void ()> s_signal;
        return s_signal;
    }

    static boost::signals2::signal<void (std::shared_ptr<NetCore>)>& NetCoreCreate() {
        static boost::signals2::signal<void (std::shared_ptr<NetCore>)> s_signal;
        return s_signal;
    }

    static boost::signals2::signal<void ()>& NetCoreRelease() {
        static boost::signals2::signal<void ()> s_signal;
        return s_signal;
    }
    

  public:
    boost::function<void (Task& _task)> task_process_hook_;
    boost::function<int (int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task)> task_callback_hook_;
    boost::signals2::signal<void (uint32_t _cmdid, const AutoBuffer& _buffer)> push_preprocess_signal_;


  public:
    comm::MessageQueue::MessageQueue_t GetMessageQueueId() { return messagequeue_creater_.GetMessageQueue(); }
//    std::shared_ptr<NetSource> GetNetSourceRef() {return net_source_;}
    
    void    CancelAndWait() { messagequeue_creater_.CancelAndWait(); }
    
    void    StartTask(const Task& _task);
    void    StopTask(uint32_t _taskid);
    bool    HasTask(uint32_t _taskid) const;
    void    ClearTasks();
    void    RedoTasks();
    void    TouchTasks();
    void    RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id);

    void    MakeSureLongLinkConnect();
    bool    LongLinkIsConnected();
    void    OnNetworkChange();
    bool    UseLongLink() {return need_use_longlink_; }

    void	KeepSignal();
    void	StopSignal();

    ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select);
    void AddServerBan(const std::string& _ip);
    void SetDebugHost(const std::string& _host);
    void ForbidLonglinkTlsHost(const std::vector<std::string>& _host);
    void InitHistory2BannedList();
    void SetIpConnectTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout);

    std::shared_ptr<NetSource> GetNetSource();
    int GetPackerEncoderVersion();
public:
    
#ifdef USE_LONG_LINK
    void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLinkErrCode::TDisconnectInternalCode _code);
    std::shared_ptr<LongLink>        CreateLongLink(LonglinkConfig& _config);
    bool AddMinorLongLink(const std::vector<std::string>& _hosts);
    void                DestroyLongLink(const std::string& _name);
    void                MakeSureLongLinkConnect_ext(const std::string& _name);
    bool                LongLinkIsConnected_ext(const std::string& _name);
    void                MarkMainLonglink_ext(const std::string& _name);
    std::shared_ptr<LongLink> DefaultLongLink();
    std::shared_ptr<LongLinkMetaData> GetLongLink(const std::string& _name);
    std::shared_ptr<LongLinkMetaData> DefaultLongLinkMeta();
#endif

  public:
    NetCore(boot::Context* _context, int _packer_encoder_version, bool _use_long_link = true);
    virtual ~NetCore();
    static void __Release(std::shared_ptr<NetCore> _instance);
    void ReleaseNet();
    
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
    int __ChooseChannel(const Task& _task, std::shared_ptr<LongLinkMetaData> _longlink, std::shared_ptr<LongLinkMetaData> _minorLong);
  private:
    NetCore(const NetCore&);
    NetCore& operator=(const NetCore&);

  public:
     void SetNeedUseLongLink(bool flag);
     void SetGetRealHostFunc(const std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist)> func);
     void SetAddWeakNetInfo(const std::function<void(bool _connect_timeout, struct tcp_info& _info)> func);

     void SetLongLinkGetRealHostFunc(std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist, bool _strict_match)> func);
     void SetLongLinkOnHandShakeReady(std::function<void(uint32_t _version, mars::stn::TlsHandshakeFrom _from)> func);
     void SetLongLinkShouldInterceptResult(std::function<bool(int _error_code)> func);

     void SetShortLinkGetRealHostFunc(std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist, bool _strict_match)> func);
     void SetShortLinkTaskConnectionDetail(std::function<void(const int _error_type, const int _error_code, const int _use_ip_index)> func);
     void SetShortLinkChooseProtocol(std::function<int(TaskProfile& _profile)> func);
     void SetShortLinkOnTimeoutOrRemoteShutdown(std::function<void(const TaskProfile& _profile)> func);
     void SetShortLinkOnHandShakeReady(std::function<void(uint32_t _version, mars::stn::TlsHandshakeFrom _from)> func);
     void SetShortLinkCanUseTls(std::function<bool(const std::vector<std::string>& _host_list)> func);
     void SetShortLinkShouldInterceptResult(std::function<bool(int _error_code)> func);

  public:
     bool IsAlreadyRelease();

  private:
    int packer_encoder_version_ ;
    bool need_use_longlink_;
    bool already_release_net_ = false;
    comm::MessageQueue::MessageQueueCreater           messagequeue_creater_;
    comm::MessageQueue::ScopeRegister                 asyncreg_;
    boot::Context*                              context_;
    std::shared_ptr<NetSource>                  net_source_;
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
    int all_connect_status_ = 0;
    int longlink_connect_status_ = 0;
    ConnNetType conn_net_type_ = kConnNetUnknown;
};
        
}}

#endif // STN_SRC_NET_CORE_H_
