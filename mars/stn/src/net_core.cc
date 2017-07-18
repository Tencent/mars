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

inline  static bool __ValidAndInitDefault(Task& _task, XLogger& _group) {
    if (2*60*1000 < _task.server_process_cost) {
        xerror2(TSF"server_process_cost invalid:%_ ", _task.server_process_cost) >> _group;
        return false;
    }
    
    if (30 < _task.retry_count) {
        xerror2(TSF"retrycount invalid:%_ ", _task.retry_count) >> _group;
        return false;
    }
    
    if (10 * 60 * 1000 < _task.total_timetout) {
        xerror2(TSF"total_timetout invalid:%_ ", _task.total_timetout) >> _group;
        return false;
    }
    
    if (_task.channel_select & Task::kChannelLong) {
        xassert2(_task.cmdid > 0);
        
        if (0 == _task.cmdid) {
            xerror2("use longlink, but 0 == _task.cmdid ") >> _group;
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
    
    return true;
}

#define AYNC_HANDLER asyncreg_.Get()

static const int kShortlinkErrTime = 3;


NetCore::NetCore()
    : messagequeue_creater_(true, XLOGGER_TAG)
    , asyncreg_(MessageQueue::InstallAsyncHandler(messagequeue_creater_.CreateMessageQueue()))
    , net_source_(new NetSource(*ActiveLogic::Singleton::Instance()))
    , netcheck_logic_(new NetCheckLogic())
    , anti_avalanche_(new AntiAvalanche(ActiveLogic::Singleton::Instance()->IsActive()))
    , dynamic_timeout_(new DynamicTimeout)
    , shortlink_task_manager_(new ShortLinkTaskManager(*net_source_, *dynamic_timeout_, messagequeue_creater_.GetMessageQueue()))
    , shortlink_error_count_(0)
#ifdef USE_LONG_LINK
    , zombie_task_manager_(new ZombieTaskManager(messagequeue_creater_.GetMessageQueue()))
    , longlink_task_manager_(new LongLinkTaskManager(*net_source_, *ActiveLogic::Singleton::Instance(), *dynamic_timeout_, messagequeue_creater_.GetMessageQueue()))
    , signalling_keeper_(new SignallingKeeper(longlink_task_manager_->LongLinkChannel(), messagequeue_creater_.GetMessageQueue()))
    , netsource_timercheck_(new NetSourceTimerCheck(net_source_, *ActiveLogic::Singleton::Instance(), longlink_task_manager_->LongLinkChannel(), messagequeue_creater_.GetMessageQueue()))
    , timing_sync_(new TimingSync(*ActiveLogic::Singleton::Instance()))
#endif
    , shortlink_try_flag_(false) {
    xwarn2(TSF"publiccomponent version: %0 %1", __DATE__, __TIME__);
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

    ActiveLogic::Singleton::Instance()->SignalActive.connect(boost::bind(&NetCore::__OnSignalActive, this, _1));


#ifdef USE_LONG_LINK
    zombie_task_manager_->fun_start_task_ = boost::bind(&NetCore::StartTask, this, _1);
    zombie_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromZombie, _1, _2, _3, _4, _5);
        
    // async
    longlink_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromLong, _1, _2, _3, _4, _5);

    // sync
    longlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4);
    longlink_task_manager_->fun_notify_network_err_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _1, _2, _3, _4, _5);
    longlink_task_manager_->fun_anti_avalanche_check_ = boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);
    longlink_task_manager_->LongLinkChannel().fun_network_report_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _1, _2, _3, _4, _5);

    longlink_task_manager_->LongLinkChannel().SignalConnection.connect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1));
    longlink_task_manager_->LongLinkChannel().SignalConnection.connect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1));

#ifdef __APPLE__
    longlink_task_manager_->getLongLinkConnectMonitor().fun_longlink_reset_ = boost::bind(&NetCore::__ResetLongLink, this);
#endif
        
    netsource_timercheck_->fun_time_check_suc_ = boost::bind(&NetCore::__OnTimerCheckSuc, this);

#endif

    // async
    shortlink_task_manager_->fun_callback_ = boost::bind(&NetCore::__CallBack, this, (int)kCallFromShort, _1, _2, _3, _4, _5);

    // sync
    shortlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4);
    shortlink_task_manager_->fun_notify_network_err_ = boost::bind(&NetCore::__OnShortLinkNetworkError, this, _1, _2, _3, _4, _5, _6);
    shortlink_task_manager_->fun_anti_avalanche_check_ = boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);
    shortlink_task_manager_->fun_shortlink_response_ = boost::bind(&NetCore::__OnShortLinkResponse, this, _1);

        
#ifdef USE_LONG_LINK
    GetSignalOnNetworkDataChange().connect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, signalling_keeper_, _1, _2, _3));
	signalling_keeper_->fun_send_signalling_buffer_ = boost::bind(&LongLink::SendWhenNoData, &longlink_task_manager_->LongLinkChannel(), _1, _2, _3, Task::kSignallingKeeperTaskID);
#endif

}

NetCore::~NetCore() {
    xinfo_function();

    ActiveLogic::Singleton::Instance()->SignalActive.disconnect(boost::bind(&NetCore::__OnSignalActive, this, _1));
    asyncreg_.Cancel();


#ifdef USE_LONG_LINK
    GetSignalOnNetworkDataChange().disconnect(boost::bind(&SignallingKeeper::OnNetWorkDataChanged, signalling_keeper_, _1, _2, _3));
    
    longlink_task_manager_->LongLinkChannel().SignalConnection.disconnect_all_slots();
    longlink_task_manager_->LongLinkChannel().broadcast_linkstatus_signal_.disconnect_all_slots();

    push_preprocess_signal_.disconnect_all_slots();

    delete netsource_timercheck_;
    delete signalling_keeper_;
    delete longlink_task_manager_;
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

void NetCore::__Release(NetCore* _instance) {
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(_instance->asyncreg_.Get())) {
        WaitMessage(AsyncInvoke((MessageQueue::AsyncInvokeFunction)boost::bind(&NetCore::__Release, _instance), _instance->asyncreg_.Get()));
        return;
    }
    
    delete _instance;
}


void NetCore::StartTask(const Task& _task) {
    
    ASYNC_BLOCK_START

    xgroup2_define(group);
    xinfo2(TSF"task start long short taskid:%0, cmdid:%1, need_authed:%2, cgi:%3, channel_select:%4, limit_flow:%5, ",
           _task.taskid, _task.cmdid, _task.need_authed, _task.cgi.c_str(), _task.channel_select, _task.limit_flow) >> group;
    xinfo2(TSF"host:%_, send_only:%_, cmdid:%_, server_process_cost:%_, retrycount:%_,  channel_strategy:%_, ",
    		_task.shortlink_host_list.empty()?"":_task.shortlink_host_list.front(), _task.send_only, _task.cmdid, _task.server_process_cost, _task.retry_count, _task.channel_strategy) >> group;
    xinfo2(TSF" total_timetout:%_, network_status_sensitive:%_, priority:%_, report_arg:%_",  _task.total_timetout,  _task.network_status_sensitive, _task.priority, _task.report_arg) >> group;

    Task task = _task;
    if (!__ValidAndInitDefault(task, group)) {
        OnTaskEnd(task.taskid, task.user_context, kEctLocal, kEctLocalTaskParam);
        return;
    }
    
    if (task_process_hook_) {
    	task_process_hook_(task);
    }

    if (0 == task.channel_select) {
        xerror2(TSF"error channelType (%_, %_), ", kEctLocal, kEctLocalChannelSelect) >> group;
        
        OnTaskEnd(task.taskid, task.user_context, kEctLocal, kEctLocalChannelSelect);
        return;
    }
    
    if (task.network_status_sensitive && kNoNet ==::getNetInfo()
#ifdef USE_LONG_LINK
        && LongLink::kConnected != longlink_task_manager_->LongLinkChannel().ConnectStatus()
#endif
        ) {
        xerror2(TSF"error no net (%_, %_), ", kEctLocal, kEctLocalNoNet) >> group;
        OnTaskEnd(task.taskid, task.user_context, kEctLocal, kEctLocalNoNet);
        return;
    }

    bool start_ok = false;

#ifdef USE_LONG_LINK

    if (LongLink::kConnected != longlink_task_manager_->LongLinkChannel().ConnectStatus()
            && (Task::kChannelLong & task.channel_select) && ActiveLogic::Singleton::Instance()->IsForeground()

            && (15 * 60 * 1000 >= gettickcount() - ActiveLogic::Singleton::Instance()->LastForegroundChangeTime()))
        longlink_task_manager_->getLongLinkConnectMonitor().MakeSureConnected();

#endif

    xgroup2() << group;

    switch (task.channel_select) {
    case Task::kChannelBoth: {

#ifdef USE_LONG_LINK
        bool bUseLongLink = LongLink::kConnected == longlink_task_manager_->LongLinkChannel().ConnectStatus();

        if (bUseLongLink && task.channel_strategy == Task::kChannelFastStrategy) {
            xinfo2(TSF"long link task count:%0, ", longlink_task_manager_->GetTaskCount());
            bUseLongLink = bUseLongLink && (longlink_task_manager_->GetTaskCount() <= kFastSendUseLonglinkTaskCntLimit);
        }

        if (bUseLongLink)
            start_ok = longlink_task_manager_->StartTask(task);
        else
#endif
            start_ok = shortlink_task_manager_->StartTask(task);
    }
    break;
#ifdef USE_LONG_LINK

    case Task::kChannelLong:
        start_ok = longlink_task_manager_->StartTask(task);
        break;
#endif

    case Task::kChannelShort:
        start_ok = shortlink_task_manager_->StartTask(task);
        break;

    default:
        xassert2(false);
        break;
    }

    if (!start_ok) {
        xerror2(TSF"taskid:%_, error starttask (%_, %_)", task.taskid, kEctLocal, kEctLocalStartTaskFail);
        OnTaskEnd(task.taskid, task.user_context, kEctLocal, kEctLocalStartTaskFail);
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
    if (longlink_task_manager_->StopTask(_taskid)) return;
    if (zombie_task_manager_->StopTask(_taskid)) return;
#endif

    if (shortlink_task_manager_->StopTask(_taskid)) return;

    xerror2(TSF"task no found taskid:%0", _taskid);
    
   ASYNC_BLOCK_END
}

bool NetCore::HasTask(uint32_t _taskid) const {
	WAIT_SYNC2ASYNC_FUNC(boost::bind(&NetCore::HasTask, this, _taskid));

#ifdef USE_LONG_LINK
    if (longlink_task_manager_->HasTask(_taskid)) return true;
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
    
#ifdef USE_LONG_LINK
    netsource_timercheck_->CancelConnect();
#endif

    net_source_->ClearCache();
    
    dynamic_timeout_->ResetStatus();
#ifdef USE_LONG_LINK
    timing_sync_->OnNetworkChange();
    if (longlink_task_manager_->getLongLinkConnectMonitor().NetworkChange())
        longlink_task_manager_->RedoTasks();
    zombie_task_manager_->RedoTasks();
#endif
    
    shortlink_task_manager_->RedoTasks();
    
    shortlink_try_flag_ = false;
    shortlink_error_count_ = 0;
    
}

void NetCore::KeepSignal() {
    ASYNC_BLOCK_START
	signalling_keeper_->Keep();
    ASYNC_BLOCK_END
}

void NetCore::StopSignal() {
    ASYNC_BLOCK_START
	signalling_keeper_->Stop();
    ASYNC_BLOCK_END
}

#ifdef USE_LONG_LINK
LongLink& NetCore::Longlink() { return longlink_task_manager_->LongLinkChannel();}

#ifdef __APPLE__
void NetCore::__ResetLongLink() {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__ResetLongLink, this));

    longlink_task_manager_->LongLinkChannel().Disconnect(LongLink::kNetworkChange);
    longlink_task_manager_->RedoTasks();
    
}
#endif
#endif  // apple

void NetCore::RedoTasks() {
   ASYNC_BLOCK_START
    
    xinfo_function();
    
#ifdef USE_LONG_LINK
    netsource_timercheck_->CancelConnect();
#endif

    net_source_->ClearCache();

#ifdef USE_LONG_LINK
    longlink_task_manager_->LongLinkChannel().Disconnect(LongLink::kReset);
    longlink_task_manager_->LongLinkChannel().MakeSureConnected();
    longlink_task_manager_->RedoTasks();
    zombie_task_manager_->RedoTasks();
#endif
    shortlink_task_manager_->RedoTasks();
    
   ASYNC_BLOCK_END
}

void NetCore::RetryTasks(ErrCmdType _err_type, int _err_code, int _fail_handle, uint32_t _src_taskid) {

	shortlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid);
#ifdef USE_LONG_LINK
	longlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid);
#endif
}

void NetCore::MakeSureLongLinkConnect() {
#ifdef USE_LONG_LINK
    ASYNC_BLOCK_START
    longlink_task_manager_->LongLinkChannel().MakeSureConnected();
    ASYNC_BLOCK_END
#endif
}

bool NetCore::LongLinkIsConnected() {
#ifdef USE_LONG_LINK
    return LongLink::kConnected == longlink_task_manager_->LongLinkChannel().ConnectStatus();
#endif
    return false;
}

int NetCore::__CallBack(int _from, ErrCmdType _err_type, int _err_code, int _fail_handle, const Task& _task, unsigned int _taskcosttime) {

	if (task_callback_hook_ && 0 == task_callback_hook_(_from, _err_type, _err_code, _fail_handle, _task)) {
		xwarn2(TSF"task_callback_hook let task return. taskid:%_, cgi%_.", _task.taskid, _task.cgi);
		return 0;
	}

    if (kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle)
    	return OnTaskEnd(_task.taskid, _task.user_context, _err_type, _err_code);

    if (kCallFromZombie == _from) return OnTaskEnd(_task.taskid, _task.user_context, _err_type, _err_code);

#ifdef USE_LONG_LINK
    if (!zombie_task_manager_->SaveTask(_task, _taskcosttime))
#endif
        return OnTaskEnd(_task.taskid, _task.user_context, _err_type, _err_code);

    return 0;
}

void NetCore::__OnShortLinkResponse(int _status_code) {
    if (_status_code == 301 || _status_code == 302 || _status_code == 307) {
        
#ifdef USE_LONG_LINK
        LongLink::TLongLinkStatus longlink_status = longlink_task_manager_->LongLinkChannel().ConnectStatus();
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

//void NetCore::__OnPush(uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _buf) {
//
//    if (is_push_data(_cmdid, _taskid)) {
//        xinfo2(TSF"task push seq:%_, cmdid:%_, len:%_", _taskid, _cmdid, _buf.Length());
//        push_preprocess_signal_(_cmdid, _buf);
//        OnPush(_cmdid, _buf);
//    }
//}

void NetCore::__OnLongLinkNetworkError(int _line, ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__OnLongLinkNetworkError, this, _line, _err_type,  _err_code, _ip, _port));
    xassert2(MessageQueue::CurrentThreadMessageQueue() == messagequeue_creater_.GetMessageQueue());

    netcheck_logic_->UpdateLongLinkInfo(longlink_task_manager_->GetTasksContinuousFailCount(), _err_type == kEctOK);
    OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);

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
void NetCore::__OnLongLinkConnStatusChange(LongLink::TLongLinkStatus _status) {
    if (LongLink::kConnected == _status) zombie_task_manager_->RedoTasks();

    __ConnStatusCallBack();
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
    longlink_connstatus = longlink_task_manager_->LongLinkChannel().ConnectStatus();
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

    xinfo2(TSF"reportNetConnectInfo all_connstatus:%_, longlink_connstatus:%_", all_connstatus, longlink_connstatus);
    ReportConnectStatus(all_connstatus, longlink_connstatus);
}

void NetCore::__OnTimerCheckSuc() {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::__OnTimerCheckSuc, this));
    
#ifdef USE_LONG_LINK
    xinfo2("netsource timercheck disconnect longlink");
    longlink_task_manager_->LongLinkChannel().Disconnect(LongLink::kTimeCheckSucc);
    
#endif

}

void NetCore::__OnSignalActive(bool _isactive) {
    ASYNC_BLOCK_START
    
    anti_avalanche_->OnSignalActive(_isactive);
    
    ASYNC_BLOCK_END
}

