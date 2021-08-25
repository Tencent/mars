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
 * net_core.cc
 *
 *  Created on: 2012-7-18
 *      Author: yerungui
 */

#include "net_core.h"

#include <stdlib.h>

#include "boost/bind.hpp"
#include "boost/ref.hpp"


#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/network/netinfo_util.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/singleton.h"
#include "mars/comm/platform_comm.h"
#include "mars/stn/stn.h"

#include "mars/app/app.h"
#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/stn/config.h"
#include "mars/stn/task_profile.h"
#include "mars/stn/proto/longlink_packer.h"

#include "net_source.h"
#include "net_check_logic.h"
#include "anti_avalanche.h"
#include "shortlink_task_manager.h"
#include "dynamic_timeout.h"

#ifdef USE_LONG_LINK
#include "longlink_task_manager.h"
#include "netsource_timercheck.h"
#include "timing_sync.h"
#endif

#include "signalling_keeper.h"
#include "zombie_task_manager.h"

using namespace mars::stn;
using namespace mars::app;
using namespace mars::comm;


#define AYNC_HANDLER asyncreg_.Get()

static const int kShortlinkErrTime = 3;

NetCore::NetCore()
    : messagequeue_creater_(true, XLOGGER_TAG)
    , asyncreg_(MessageQueue::InstallAsyncHandler(messagequeue_creater_.CreateMessageQueue()))
    , net_source_(new NetSource(*ActiveLogic::Instance()))
    , netcheck_logic_(new NetCheckLogic())
    , anti_avalanche_(new AntiAvalanche(ActiveLogic::Instance()->IsActive()))
    , dynamic_timeout_(new DynamicTimeout)
    , shortlink_task_manager_(new ShortLinkTaskManager(*net_source_, *dynamic_timeout_, messagequeue_creater_.GetMessageQueue()))
    , shortlink_error_count_(0)
    , shortlink_try_flag_(false) {
    xwarn2(TSF"public component version: %0 %1", __DATE__, __TIME__);
    xassert2(messagequeue_creater_.GetMessageQueue() != MessageQueue::KInvalidQueueID, "CreateNewMessageQueue Error!!!");
    xinfo2(TSF"netcore messagequeue_id=%_, handler:(%_,%_)", messagequeue_creater_.GetMessageQueue(), asyncreg_.Get().queue, asyncreg_.Get().seq);

    std::string printinfo;

    SIMInfo info;
    getCurSIMInfo(info);
    printinfo = printinfo + "ISP_NAME : " + info.isp_name + "\n";
    printinfo = printinfo + "ISP_CODE : " + info.isp_code + "\n";

    AccountInfo account = ::GetAccountInfo();

    if (0 != account.uin) {
        char uinBuffer[64] = {0};
        snprintf(uinBuffer, sizeof(uinBuffer), "%u", (unsigned int)account.uin);
        printinfo = printinfo + "Uin :" + uinBuffer  + "\n";
    }

    if (!account.username.empty()) {
        printinfo = printinfo + "UserName :" + account.username + "\n";
    }

    char version[256] = {0};
    snprintf(version, sizeof(version), "0x%X", mars::app::GetClientVersion());
    printinfo = printinfo + "ClientVersion :" + version + "\n";

    xwarn2(TSF"\n%0", printinfo.c_str());

    {
        //note: iOS getwifiinfo may block for 10+ seconds sometimes
        ASYNC_BLOCK_START

        xinfo2(TSF"net info:%_", GetDetailNetInfo());
        
        ASYNC_BLOCK_END
    }
                   
    xinfo_function();

    ActiveLogic::Instance()->SignalActive.connect(boost::bind(&NetCore::__OnSignalActive, this, _1));

    __InitLongLink();
    __InitShortLink();
}

NetCore::~NetCore() {
    xinfo_function();

    ActiveLogic::Instance()->SignalActive.disconnect(boost::bind(&NetCore::__OnSignalActive, this, _1));
    asyncreg_.Cancel();
#ifdef USE_LONG_LINK
    {   //must disconnect signal
        auto longlink = longlink_task_manager_->DefaultLongLink();
        if (longlink && longlink->SignalKeeper()) {
            GetSignalOnNetworkDataChange().disconnect_all_slots();
        }
    }
    delete longlink_task_manager_;

    push_preprocess_signal_.disconnect_all_slots();

    delete timing_sync_;
    delete zombie_task_manager_;
#endif
    delete shortlink_task_manager_;
    delete dynamic_timeout_;
    
    delete anti_avalanche_;
    delete netcheck_logic_;
    delete net_source_;
    
    MessageQueue::MessageQueueCreater::ReleaseNewMessageQueue(MessageQueue::Handler2Queue(asyncreg_.Get()));
}

void NetCore::__InitShortLink(){
    // async
    shortlink_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromShort, _1, _2, _3, _4, _5);
    
    // sync
    shortlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4, _5);
    shortlink_task_manager_->fun_notify_network_err_ = boost::bind(&NetCore::__OnShortLinkNetworkError, this, _1, _2, _3, _4, _5, _6);
    shortlink_task_manager_->fun_anti_avalanche_check_ = boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);
    shortlink_task_manager_->fun_shortlink_response_ = boost::bind(&NetCore::__OnShortLinkResponse, this, _1);
}

void NetCore::__InitLongLink(){
#ifdef USE_LONG_LINK
    zombie_task_manager_ = new ZombieTaskManager(messagequeue_creater_.GetMessageQueue());
    zombie_task_manager_->fun_start_task_ = boost::bind(&NetCore::StartTask, this, _1);
    zombie_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromZombie, _1, _2, _3, _4, _5);

    timing_sync_ = new TimingSync(*ActiveLogic::Instance());

    longlink_task_manager_ = new LongLinkTaskManager(*net_source_, *ActiveLogic::Instance(), *dynamic_timeout_, GetMessageQueueId());

    LonglinkConfig defaultConfig(DEFAULT_LONGLINK_NAME, DEFAULT_LONGLINK_GROUP, true);
    defaultConfig.is_keep_alive = true;
    CreateLongLink(defaultConfig);

    // async
    longlink_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromLong, _1, _2, _3, _4, _5);
    
    // sync
    longlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4, _5);
    longlink_task_manager_->fun_notify_network_err_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _1, _2, _3, _4, _5, _6);
    longlink_task_manager_->fun_anti_avalanche_check_ = boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);
    
    longlink_task_manager_->fun_on_push_ = boost::bind(&NetCore::__OnPush, this, _1, _2, _3, _4, _5);
#endif
}

void NetCore::__Release(NetCore* _instance) {
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(_instance->asyncreg_.Get())) {
        WaitMessage(AsyncInvoke((MessageQueue::AsyncInvokeFunction)boost::bind(&NetCore::__Release, _instance), _instance->asyncreg_.Get(), "NetCore::__Release"));
        return;
    }
    
    delete _instance;
}

bool NetCore::__ValidAndInitDefault(Task& _task, XLogger& _group) {
    if (2*60*1000 < _task.server_process_cost) {
        xerror2(TSF"server_process_cost invalid:%_ ", _task.server_process_cost) >> _group;
        return false;
    }
    
    if (30 < _task.retry_count) {
        xerror2(TSF"retrycount invalid:%_ ", _task.retry_count) >> _group;
        return false;
    }
    
    if (10 * 60 * 1000 < _task.total_timeout) {
        xerror2(TSF"total_timeout invalid:%_ ", _task.total_timeout) >> _group;
        return false;
    }
    
    if (_task.channel_select & Task::kChannelLong) {
        if (0 == _task.cmdid) {
            xwarn2(" use longlink, but 0 == _task.cmdid ") >> _group;
            _task.channel_select &= ~Task::kChannelLong;
        }
    }
    
    if (_task.channel_select & Task::kChannelShort) {
        xassert2(!_task.cgi.empty());
        if (_task.cgi.empty()) {
            xerror2("use shortlink, but cgi is empty ") >> _group;
            _task.channel_select &= ~Task::kChannelShort;
        }
    }
    
    if (0 >  _task.retry_count) {
        _task.retry_count = DEF_TASK_RETRY_COUNT;
    }
    
//    if((_task.channel_select==Task::kChannelBoth||_task.channel_select==Task::kChannelLong)
//       &&longlink_task_manager_->GetLongLink(_task.channel_name)==nullptr){
//        return false;
//    }
    
    return true;
}

int NetCore::__ChooseChannel(const Task& _task, std::shared_ptr<LongLinkMetaData> _longlink, std::shared_ptr<LongLinkMetaData> _minorLong) {
    bool longlinkOk = (_longlink != nullptr);
    longlinkOk = longlinkOk && LongLink::kConnected == _longlink->Channel()->ConnectStatus();

    if (longlinkOk && _task.channel_strategy == Task::kChannelFastStrategy) {
        xinfo2(TSF"long link task count:%0, ", longlink_task_manager_->GetTaskCount(_task.channel_name));
        longlinkOk = longlinkOk && (longlink_task_manager_->GetTaskCount(_task.channel_name) <= kFastSendUseLonglinkTaskCntLimit);
    }

    bool minorOk = _task.minorlong_host_list.empty() ? false : longlink_task_manager_->IsMinorAvailable(_task);
    int channel = _task.channel_select;
    switch(_task.channel_select) {
        case Task::kChannelAll:
            channel = minorOk ? Task::kChannelMinorLong : (longlinkOk ? Task::kChannelLong : Task::kChannelShort);
            break;
        case Task::kChannelNormal:
            channel = minorOk ? Task::kChannelMinorLong : Task::kChannelShort;
            break;
        case Task::kChannelBoth:
            channel = longlinkOk ? Task::kChannelLong : Task::kChannelShort;
            break;
        default:
            break;
    }
    return channel;
}

void NetCore::StartTask(const Task& _task) {
    
    ASYNC_BLOCK_START

    xgroup2_define(group);
    xinfo2(TSF"task start long short taskid:%0, cmdid:%1, need_authed:%2, cgi:%3, channel_select:%4, limit_flow:%5, channel_name:%6",
           _task.taskid, _task.cmdid, _task.need_authed, _task.cgi.c_str(), _task.channel_select, _task.limit_flow, _task.channel_name) >> group;
    xinfo2(TSF"host:%_, send_only:%_, cmdid:%_, server_process_cost:%_, retrycount:%_,  channel_strategy:%_, channel_name:%_",
           _task.shortlink_host_list.empty()?"":_task.shortlink_host_list.front(),
           _task.send_only, _task.cmdid, _task.server_process_cost, _task.retry_count, _task.channel_strategy,
           _task.channel_name) >> group;
    xinfo2(TSF" total_timeout:%_, network_status_sensitive:%_, priority:%_, report_arg:%_",  _task.total_timeout,  _task.network_status_sensitive, _task.priority, _task.report_arg) >> group;

    Task task = _task;
    if (!__ValidAndInitDefault(task, group)) {
        ConnectProfile profile;
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalTaskParam, profile);
        return;
    }
    
    if (task_process_hook_) {
    	task_process_hook_(task);
    }

    if (0 == task.channel_select) {
        xerror2(TSF"error channelType (%_, %_), ", kEctLocal, kEctLocalChannelSelect) >> group;
        ConnectProfile profile;
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalChannelSelect, profile);
        return;
    }


    xdebug2(TSF"longlink size: %_", longlink_task_manager_->GetLongLinkSize());
    auto longlink = longlink_task_manager_->GetLongLink(task.channel_name);

    if (task.channel_name == RUNON_MAIN_LONGLINK_NAME){
        longlink = longlink_task_manager_->DefaultLongLink();
        xassert2(longlink && longlink->Channel()->ConnectStatus() == LongLink::kConnected);
        if (!longlink || longlink->Channel()->ConnectStatus() != LongLink::kConnected){
            xerror2(TSF"error main longlink unavailable. (%_, %_), ", kEctLocal, kEctMainLongLinkUnAvailable) >> group;
            return;
        }
        
        xassert2(longlink->Config().IsMain());
        task.channel_name = longlink->Config().name;
    }
    std::shared_ptr<LongLinkMetaData> minorlonglink = nullptr;
    if((task.channel_select & Task::kChannelMinorLong) && !task.minorlong_host_list.empty()) {
        longlink_task_manager_->FixMinorRealhost(task);
        std::string host = task.minorlong_host_list.empty()? "" : task.minorlong_host_list.front();
        if(!host.empty())
            minorlonglink = longlink_task_manager_->GetLongLink(host);
    }
    if (task.network_status_sensitive && kNoNet ==::getNetInfo()
#ifdef USE_LONG_LINK
        && longlink && LongLink::kConnected != longlink->Channel()->ConnectStatus()
#endif
        ) {
        xerror2(TSF"error no net (%_, %_), ", kEctLocal, kEctLocalNoNet) >> group;
        ConnectProfile profile;
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
        return;
    }
    
#ifdef ANDROID
    if (kNoNet == ::getNetInfo() && !ActiveLogic::Instance()->IsActive()
#ifdef USE_LONG_LINK
    && LongLink::kConnected != longlink_task_manager_->GetLongLink(task.channel_name)->Channel()->ConnectStatus()
#endif
    ){
        xerror2(TSF" error no net (%_, %_) return when no active", kEctLocal, kEctLocalNoNet) >> group;
        ConnectProfile profile;
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
        return;
    }
#endif

    bool start_ok = false;

#ifdef USE_LONG_LINK
    if (longlink && LongLink::kConnected != longlink->Channel()->ConnectStatus()
           && (Task::kChannelLong & task.channel_select) && ActiveLogic::Instance()->IsForeground()
           && (15 * 60 * 1000 >= gettickcount() - ActiveLogic::Instance()->LastForegroundChangeTime())) {
        longlink->Monitor()->MakeSureConnected();
    }
    if ((Task::kChannelMinorLong & task.channel_select) && ActiveLogic::Instance()->IsForeground()) {
        if(!minorlonglink) {
            bool ret = longlink_task_manager_->AddMinorLink(task.minorlong_host_list);
            if(ret) {
                minorlonglink = longlink_task_manager_->GetLongLink(task.minorlong_host_list.front());
                minorlonglink->Monitor()->MakeSureConnected();
            }
        } else if(LongLink::kConnected != minorlonglink->Channel()->ConnectStatus()) {
            minorlonglink->Channel()->SvrTrigOff();
            minorlonglink->Monitor()->MakeSureConnected();
        }
    }
#endif

    xgroup2() << group;

    int channel = __ChooseChannel(task, longlink, minorlonglink);
    switch (channel) {
#ifdef USE_LONG_LINK
    case Task::kChannelMinorLong:
    case Task::kChannelLong:
        start_ok = longlink_task_manager_->StartTask(task, channel);
        break;
#endif

    case Task::kChannelShort:
        task.shortlink_fallback_hostlist = task.shortlink_host_list;
        start_ok = shortlink_task_manager_->StartTask(task);
        break;

    default:
        xassert2(false);
        break;
    }

    if (!start_ok) {
        xerror2(TSF"taskid:%_, error starttask (%_, %_)", task.taskid, kEctLocal, kEctLocalStartTaskFail);
        ConnectProfile profile;
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalStartTaskFail, profile);
    } else {
#ifdef USE_LONG_LINK
        zombie_task_manager_->OnNetCoreStartTask();
#endif
    }
        
    ASYNC_BLOCK_END
}

void NetCore::StopTask(uint32_t _taskid) {
   ASYNC_BLOCK_START
    
#ifdef USE_LONG_LINK
    if(longlink_task_manager_->StopTask(_taskid))   return;

    if (zombie_task_manager_->StopTask(_taskid)) return;
#endif

    if (shortlink_task_manager_->StopTask(_taskid)) return;

    xerror2(TSF"task no found taskid:%0", _taskid);
    
   ASYNC_BLOCK_END
}

bool NetCore::HasTask(uint32_t _taskid) const {
	WAIT_SYNC2ASYNC_FUNC(boost::bind(&NetCore::HasTask, this, _taskid));

#ifdef USE_LONG_LINK
    if(longlink_task_manager_->HasTask(_taskid))    return true;

    if (zombie_task_manager_->HasTask(_taskid)) return true;
#endif
    if (shortlink_task_manager_->HasTask(_taskid)) return true;
    
	   return false;
}

void NetCore::ClearTasks() {
    ASYNC_BLOCK_START
    
#ifdef USE_LONG_LINK
    longlink_task_manager_->ClearTasks();
    zombie_task_manager_->ClearTasks();
#endif
    shortlink_task_manager_->ClearTasks();
    
    ASYNC_BLOCK_END
}

void NetCore::OnNetworkChange() {
    
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::OnNetworkChange, this));  //if already messagequeue, no need to async

    xinfo_function();

    std::string ip_stack_log;
    TLocalIPStack ip_stack = local_ipstack_detect_log(ip_stack_log);

    switch (::getNetInfo()) {
    case kNoNet:
        xinfo2(TSF"task network change current network:no network");
        break;

    case kWifi: {
        WifiInfo info;
        getCurWifiInfo(info);
        xinfo2(TSF"task network change current network:wifi, ssid:%_, ip_stack:%_, log:%_", info.ssid, TLocalIPStackStr[ip_stack], ip_stack_log);
    }
    break;

    case kMobile: {
        SIMInfo info;
        getCurSIMInfo(info);
        RadioAccessNetworkInfo raninfo;
        getCurRadioAccessNetworkInfo(raninfo);
        xinfo2(TSF"task network change current network:mobile, ispname:%_, ispcode:%_, ran:%_, ip_stack:%_, log:%_", info.isp_name, info.isp_code, raninfo.radio_access_network, TLocalIPStackStr[ip_stack], ip_stack_log);
    }
    break;

    case kOtherNet:
        xinfo2(TSF"task network change current network:other, ip_stack:%_, log:%_", TLocalIPStackStr[ip_stack], ip_stack_log);
        break;

    default:
        xassert2(false);
        break;
    }

    net_source_->ClearCache();
    
    dynamic_timeout_->ResetStatus();
#ifdef USE_LONG_LINK
    timing_sync_->OnNetworkChange();
    
    longlink_task_manager_->OnNetworkChange();

    zombie_task_manager_->RedoTasks();
#endif
    
    shortlink_task_manager_->RedoTasks();
    
    shortlink_try_flag_ = false;
    shortlink_error_count_ = 0;
    
}

void NetCore::KeepSignal() {
    ASYNC_BLOCK_START
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if(!longlink)    return;
    longlink->SignalKeeper()->Keep();
    ASYNC_BLOCK_END
}

void NetCore::StopSignal() {
    ASYNC_BLOCK_START
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if(!longlink)    return;
    longlink->SignalKeeper()->Stop();
    ASYNC_BLOCK_END
}

#ifdef USE_LONG_LINK
void NetCore::DisconnectLongLinkByTaskId(uint32_t _taskid, LongLink::TDisconnectInternalCode _code){
    longlink_task_manager_->DisconnectByTaskId(_taskid, _code);
}

//#ifdef __APPLE__
//void NetCore::__ResetLongLink() {
//    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__ResetLongLink, this));
//    longlink_task_managers_[DEFAULT_LONGLINK_NAME]->LongLinkChannel().Disconnect(LongLink::kNetworkChange);
//    longlink_task_managers_[DEFAULT_LONGLINK_NAME]->RedoTasks();
//}
//#endif
#endif

void NetCore::RedoTasks() {
   ASYNC_BLOCK_START
    
    xinfo_function();

    net_source_->ClearCache();

#ifdef USE_LONG_LINK
    longlink_task_manager_->RedoTasks();
    zombie_task_manager_->RedoTasks();
#endif
    shortlink_task_manager_->RedoTasks();
    
   ASYNC_BLOCK_END
}

void NetCore::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid, std::string _user_id) {
    xinfo2(TSF"shortlink_task_manager retry task id %_", _src_taskid);
	shortlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid);
#ifdef USE_LONG_LINK
    longlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid, _user_id);
#endif
}

void NetCore::MakeSureLongLinkConnect() {
#ifdef USE_LONG_LINK
    ASYNC_BLOCK_START
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if(!longlink) {
        xassert2(false, TSF"longlink meta is null");
        return;
    }   
    auto longlink_channel = longlink->Channel();
    if (longlink_channel) {
        longlink_channel->MakeSureConnected();
    } else {
        xassert2(false, TSF"longlink channel is null");
    }
    ASYNC_BLOCK_END
#endif
}

bool NetCore::LongLinkIsConnected() {
#ifdef USE_LONG_LINK
	auto longlink = longlink_task_manager_->DefaultLongLink();
	if(!longlink)    return false;
    return LongLink::kConnected == longlink->Channel()->ConnectStatus();
#else
    return false;
#endif
}

int NetCore::__CallBack(int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime) {
    if (task_callback_hook_ && 0 == task_callback_hook_(_from, _err_type, _err_code, _fail_handle, _task)) {
        xwarn2(TSF"task_callback_hook let task return. taskid:%_, cgi%_.", _task.taskid, _task.cgi);
        return 0;
     }
    if (kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle)
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, GetConnectProfile(_task.taskid, Task::kChannelShort));

    ConnectProfile profile;
    if (kCallFromZombie == _from)
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);

#ifdef USE_LONG_LINK
    if (!zombie_task_manager_->SaveTask(_task, _taskcosttime))
#endif
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);

    return 0;
}

void NetCore::__OnShortLinkResponse(int _status_code) {
    if (_status_code == 301 || _status_code == 302 || _status_code == 307) {
        
#ifdef USE_LONG_LINK
	    auto longlink = longlink_task_manager_->DefaultLongLink();
	    if(!longlink)    return;
        LongLink::TLongLinkStatus longlink_status = longlink->Channel()->ConnectStatus();
        unsigned int continues_fail_count = longlink_task_manager_->GetTasksContinuousFailCount();
        xinfo2(TSF"status code:%0, long link status:%1, longlink task continue fail count:%2", _status_code, longlink_status, continues_fail_count);
        
        if (LongLink::kConnected == longlink_status && continues_fail_count == 0) {
            return;
        }
#endif
        // TODO callback
    }
}

#ifdef USE_LONG_LINK

void NetCore::__OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    xinfo2(TSF"task push seq:%_, cmdid:%_, len:%_, %_", _taskid, _cmdid, _body.Length(), _channel_id);
    push_preprocess_signal_(_cmdid, _body, _channel_id);
    OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

void NetCore::__OnLongLinkNetworkError(const std::string& _name, int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__OnLongLinkNetworkError, this, _name, _line, _err_type,  _err_code, _ip, _port));
    xassert2(MessageQueue::CurrentThreadMessageQueue() == messagequeue_creater_.GetMessageQueue());

    netcheck_logic_->UpdateLongLinkInfo(longlink_task_manager_->GetTasksContinuousFailCount(), _err_type == kEctOK);
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if(longlink && longlink->Config().IsMain()) {
        OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
    }

    if (kEctOK == _err_type) zombie_task_manager_->RedoTasks();

    if (kEctDial == _err_type) return;

    if (kEctHttp == _err_type) return;

    if (kEctServer == _err_type) return;

    if (kEctLocal == _err_type) return;

    net_source_->ReportLongIP(_err_type == kEctOK, _ip, _port);

}
#endif

void NetCore::__OnShortLinkNetworkError(int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__OnShortLinkNetworkError, this, _line, _err_type,  _err_code, _ip, _host, _port));
    xassert2(MessageQueue::CurrentThreadMessageQueue() == messagequeue_creater_.GetMessageQueue());

    netcheck_logic_->UpdateShortLinkInfo(shortlink_task_manager_->GetTasksContinuousFailCount(), _err_type == kEctOK);
    OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);

    shortlink_try_flag_ = true;

    if (_err_type == kEctOK) {
        shortlink_error_count_ = 0;
    } else {
        ++shortlink_error_count_;
    }

    __ConnStatusCallBack();

#ifdef USE_LONG_LINK
    if (kEctOK == _err_type) zombie_task_manager_->RedoTasks();
#endif

    if (kEctDial == _err_type) return;

    if (kEctNetMsgXP == _err_type) return;

    if (kEctServer == _err_type) return;

    if (kEctLocal == _err_type) return;

    net_source_->ReportShortIP(_err_type == kEctOK, _ip, _host, _port);

}


#ifdef USE_LONG_LINK
void NetCore::__OnLongLinkConnStatusChange(LongLink::TLongLinkStatus _status, const std::string& _channel_id) {
    if (LongLink::kConnected == _status) zombie_task_manager_->RedoTasks();

    __ConnStatusCallBack();
    OnLongLinkStatusChange(_status);
}
#endif

void NetCore::__ConnStatusCallBack() {
    int all_connstatus = kNetworkUnavailable;

    if (shortlink_try_flag_) {
        if (shortlink_error_count_ >= kShortlinkErrTime) {
            all_connstatus = kServerFailed;
        } else if (0 == shortlink_error_count_) {
            all_connstatus = kConnected;
        } else {
            all_connstatus = kNetworkUnkown;
        }
    } else {
        all_connstatus = kNetworkUnkown;
    }
    int longlink_connstatus = kNetworkUnkown;
#ifdef USE_LONG_LINK
	auto longlinkMeta = longlink_task_manager_->DefaultLongLink();
	if(!longlinkMeta)    return;
    auto longlink = longlinkMeta->Channel();
    if (!longlink) return;
    longlink_connstatus = longlink->ConnectStatus();
    switch (longlink_connstatus) {
        case LongLink::kDisConnected:
            return;
        case LongLink::kConnectFailed:
            if (shortlink_try_flag_) {
                if (0 == shortlink_error_count_) {
                    all_connstatus = kConnected;
                }
                else if (shortlink_error_count_ >= kShortlinkErrTime) {
                    all_connstatus = kServerFailed;
                }
                else {
                    all_connstatus = kNetworkUnkown;
                }
            }
            else {
                all_connstatus = kNetworkUnkown;
            }
            longlink_connstatus = kServerFailed;
            break;
            
        case LongLink::kConnectIdle:
        case LongLink::kConnecting:
            if (shortlink_try_flag_) {
                if (0 == shortlink_error_count_) {
                    all_connstatus = kConnected;
                }
                else if (shortlink_error_count_ >= kShortlinkErrTime) {
                    all_connstatus = kServerFailed;
                }
                else {
                    all_connstatus = kConnecting;
                }
            }
            else {
                all_connstatus = kConnecting;
            }
            
            longlink_connstatus = kConnecting;
            break;
            
        case LongLink::kConnected:
            all_connstatus = kConnected;
            shortlink_error_count_ = 0;
            shortlink_try_flag_ = false;
            longlink_connstatus = kConnected;
            break;
            
        default:
            xassert2(false);
            break;
    }
#else
    if (shortlink_error_count_ >= kShortlinkErrTime) {
        all_connstatus = kServerFailed;
    } else if (0 == shortlink_error_count_) {
        all_connstatus = kConnected;
    } else {
        all_connstatus = kConnected;
    }
#endif
    if(all_connstatus != all_connect_status_ || longlink_connstatus != longlink_connect_status_) {      // logs limits
        all_connect_status_ = all_connstatus;
        longlink_connect_status_ = longlink_connstatus;
        xinfo2(TSF"reportNetConnectInfo all_connstatus:%_, longlink_connstatus:%_", all_connstatus, longlink_connstatus);
    }
    ReportConnectStatus(all_connstatus, longlink_connstatus);
}

void NetCore::__OnSignalActive(bool _isactive) {
    ASYNC_BLOCK_START
    
    anti_avalanche_->OnSignalActive(_isactive);
    
    ASYNC_BLOCK_END
}

void NetCore::AddServerBan(const std::string& _ip) {
    net_source_->AddServerBan(_ip);
}

ConnectProfile NetCore::GetConnectProfile(uint32_t _taskid, int _channel_select) {
    if (_channel_select == Task::kChannelShort) {
        return shortlink_task_manager_->GetConnectProfile(_taskid);
    }
#ifdef USE_LONG_LINK
    else if (_channel_select == Task::kChannelLong || _channel_select == Task::kChannelMinorLong) {
        return longlink_task_manager_->GetConnectProfile(_taskid);
    }
#endif
    return ConnectProfile();
}

//===----------------------------------------------------------------------===//
///
/// Multi long link APIs
///
//===----------------------------------------------------------------------===//
#ifdef USE_LONG_LINK
std::shared_ptr<LongLink> NetCore::CreateLongLink(LonglinkConfig& _config){
    auto oldDefault = longlink_task_manager_->DefaultLongLink();
    if(!longlink_task_manager_->AddLongLink(_config)) {
        auto longlink = longlink_task_manager_->GetLongLink(_config.name);
        if(longlink) {
	        xwarn2(TSF"already has longlink named:%_", _config.name);
	        return longlink_task_manager_->GetLongLink(_config.name)->Channel();
        }
    }

    auto longlink = longlink_task_manager_->GetLongLink(_config.name);
    if(!longlink) {
	    xassert2(false, TSF"get longlink nullptr with name:%_", _config.name.c_str());
	    return nullptr;
    }
    auto longlink_channel = longlink->Channel();
    if(!longlink_channel) {
        xassert2(false, TSF"get longlink nullptr with name:%_", _config.name.c_str());
        return nullptr;
    }
    
    if(_config.IsMain() && oldDefault) {
        xinfo2(TSF"change default longlink to name:%_, group:%_", _config.name, _config.group);
        oldDefault->Channel()->SignalConnection.disconnect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
        oldDefault->Channel()->SignalConnection.disconnect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
        GetSignalOnNetworkDataChange().disconnect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, oldDefault->SignalKeeper().get(), _1, _2, _3));
        oldDefault->Config().isMain = false;
    }
    
    if(_config.IsMain()) {
        longlink_channel->fun_network_report_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _config.name, _1, _2, _3, _4, _5);
        longlink_channel->SignalConnection.connect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
        longlink_channel->SignalConnection.connect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
        if(longlink->SignalKeeper()) {
            GetSignalOnNetworkDataChange().connect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
        }
    }
    
    xinfo2(TSF"create long link %_", _config.name);
    return longlink_channel;
}

bool NetCore::AddMinorLongLink(const std::vector<std::string>& _hosts) {
    return longlink_task_manager_->AddMinorLink(_hosts);
}


void NetCore::ForbidLonglinkTlsHost(const std::vector<std::string>& _host) {
    longlink_task_manager_->AddForbidTlsHost(_host);
}

void NetCore::DestroyLongLink(const std::string& _name){
	WAIT_SYNC2ASYNC_FUNC(boost::bind(&NetCore::DestroyLongLink, this, _name));
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if(!longlink) {
        xwarn2(TSF"destroy long link failure: no such long link exists %_",_name);
        return;
    }
    
    if(longlink->SignalKeeper()) {
        GetSignalOnNetworkDataChange().disconnect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
    }
    longlink->Channel()->SignalConnection.disconnect_all_slots();
    longlink->Channel()->broadcast_linkstatus_signal_.disconnect_all_slots();
    longlink.reset();   // do not hold the shared_ptr

    longlink_task_manager_->ReleaseLongLink(_name);
    xinfo2(TSF"destroy long link %_ ", _name);
}

void NetCore::MarkMainLonglink_ext(const std::string& _name) {
    auto oldLink = DefaultLongLink();
    auto oldMeta = DefaultLongLinkMeta();
    auto newLinkMeta = GetLongLink(_name);
	auto newLink = newLinkMeta ? newLinkMeta->Channel() : nullptr;
	if(oldLink == nullptr || newLink == nullptr || oldMeta == nullptr || oldMeta->Config().name == _name) {
		xerror2(TSF"link nullptr, old:%_, new:%_, or same longlink", (oldLink==nullptr), (newLink==nullptr));
		return;
	}

    xinfo2(TSF"change default longlink to name:%_", _name);
    oldLink->SignalConnection.disconnect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
    oldLink->SignalConnection.disconnect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
    GetSignalOnNetworkDataChange().disconnect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, oldMeta->SignalKeeper().get(), _1, _2, _3));
	oldMeta->Config().isMain = false;
    
    newLink->fun_network_report_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _name, _1, _2, _3, _4, _5);
    newLink->SignalConnection.connect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
    newLink->SignalConnection.connect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));

    auto longlink = GetLongLink(_name);
    if(longlink && longlink->SignalKeeper()) {
        GetSignalOnNetworkDataChange().connect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
    }
	longlink->Config().isMain = true;

    return;
}

void NetCore::MakeSureLongLinkConnect_ext(const std::string& _name) {
    ASYNC_BLOCK_START
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if(longlink){
        longlink->Channel()->MakeSureConnected();
    }
    ASYNC_BLOCK_END
}

bool NetCore::LongLinkIsConnected_ext(const std::string& _name) {
	auto longlink = longlink_task_manager_->GetLongLink(_name);
    if(longlink){
        if(longlink->Channel()->ConnectStatus()==LongLink::kConnected){
            return true;
        }
    }
    return false;
}

std::shared_ptr<LongLink> NetCore::DefaultLongLink() {
	auto longlink = longlink_task_manager_->DefaultLongLink();
    if(!longlink)
        return nullptr;
    return longlink->Channel();
}

std::shared_ptr<LongLinkMetaData> NetCore::DefaultLongLinkMeta() {
    return longlink_task_manager_->DefaultLongLink();
}

std::shared_ptr<LongLinkMetaData> NetCore::GetLongLink(const std::string& _name) {
    return longlink_task_manager_->GetLongLink(_name);
}


void NetCore::SetDebugHost(const std::string& _host) {
    shortlink_task_manager_->SetDebugHost(_host);
}

#ifdef USE_LONG_LINK
LongLinkTaskManager* NetCore::LonglinkTaskManagerInstance() {
    return longlink_task_manager_;
}
#endif
ShortLinkTaskManager* NetCore::ShortLinkTaskManagerInstance() {
    return shortlink_task_manager_;
}
#endif
