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

#include <mutex>

#include "anti_avalanche.h"
#include "boost/bind.hpp"
#include "boost/ref.hpp"
#include "dynamic_timeout.h"
#include "mars/app/app.h"
#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/network/netinfo_util.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/singleton.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/config.h"
#include "mars/stn/proto/longlink_packer.h"
#include "mars/stn/stn.h"
#include "mars/stn/task_profile.h"
#include "net_check_logic.h"
#include "net_source.h"
#include "shortlink_task_manager.h"

#ifdef USE_LONG_LINK
#include "longlink_task_manager.h"
#include "netsource_timercheck.h"
#include "timing_sync.h"
#endif

#include "mars/app/app_manager.h"
#include "mars/stn/stn_manager.h"
#include "signalling_keeper.h"
#include "zombie_task_manager.h"

using namespace mars::stn;
using namespace mars::app;
using namespace mars::comm;

#define AYNC_HANDLER asyncreg_.Get()

static const int kShortlinkErrTime = 3;

// bool NetCore::need_use_longlink_ = true;

NetCore::NetCore(boot::Context* _context,
                 int _packer_encoder_version,
                 std::string _packer_encoder_name,
                 bool _use_long_link,
                 LongLinkEncoder* longlink_encoder,
                 std::string tls_group_name)
: packer_encoder_version_(_packer_encoder_version)
, packer_encoder_name_(_packer_encoder_name)
, need_use_longlink_(_use_long_link)
, messagequeue_creater_(true, XLOGGER_TAG)
, asyncreg_(MessageQueue::InstallAsyncHandler(messagequeue_creater_.CreateMessageQueue()))
, context_(_context)
, net_source_(new NetSource(*ActiveLogic::Instance(), context_))
, netcheck_logic_(new NetCheckLogic(context_, net_source_))
, anti_avalanche_(new AntiAvalanche(context_, ActiveLogic::Instance()->IsActive()))
, dynamic_timeout_(new DynamicTimeout)
, shortlink_task_manager_(
      new ShortLinkTaskManager(context_, net_source_, *dynamic_timeout_, messagequeue_creater_.GetMessageQueue(),
                               tls_group_name))
, shortlink_error_count_(0)
, shortlink_try_flag_(false)
, default_longlink_encoder(longlink_encoder)
#ifdef ANDROID
, wakeup_lock_(new WakeUpLock())
#endif
{
    if (context_->GetContextId() == "default") {
        NetCoreCreateBegin()();
    }
    xdebug_function(TSF "mars2");
    xwarn2(TSF "public component version: %0 %1", __DATE__, __TIME__);
    xassert2(messagequeue_creater_.GetMessageQueue() != MessageQueue::KInvalidQueueID,
             "CreateNewMessageQueue Error!!!");
    xinfo2(TSF "netcore messagequeue_id=%_, handler:(%_,%_)",
           messagequeue_creater_.GetMessageQueue(),
           asyncreg_.Get().queue,
           asyncreg_.Get().seq);

    std::string printinfo;

    SIMInfo info;
    getCurSIMInfo(info);
    printinfo = printinfo + "ISP_NAME : " + info.isp_name + "\n";
    printinfo = printinfo + "ISP_CODE : " + info.isp_code + "\n";

    AccountInfo account = context_->GetManager<AppManager>()->GetAccountInfo();

    if (0 != account.uin) {
        char uinBuffer[64] = {0};
        snprintf(uinBuffer, sizeof(uinBuffer), "%u", (unsigned int)account.uin);
        printinfo = printinfo + "Uin :" + uinBuffer + "\n";
    }

    if (!account.username.empty()) {
        printinfo = printinfo + "UserName :" + account.username + "\n";
    }

    char version[256] = {0};
    snprintf(version, sizeof(version), "0x%X", context_->GetManager<AppManager>()->GetClientVersion());
    printinfo = printinfo + "ClientVersion :" + version + "\n";

    xwarn2(TSF "\n%0", printinfo.c_str());

    {
        // note: iOS getwifiinfo may block for 10+ seconds sometimes
        ASYNC_BLOCK_START

        xinfo2(TSF "net info:%_", GetDetailNetInfo(false));

        ASYNC_BLOCK_END
    }

    xinfo_function();

    ActiveLogic::Instance()->SignalActive.connect(boost::bind(&NetCore::__OnSignalActive, this, _1));

    __InitShortLink();
    xinfo2(TSF "need longlink channel %_", need_use_longlink_);
    if (need_use_longlink_) {
        __InitLongLink();
    }
}

NetCore::~NetCore() {
    xinfo_function(TSF "mars2");
    asyncreg_.Cancel();
    if (!already_release_net_) {
        ReleaseNet();
    }
    // MessageQueue::MessageQueueCreater::ReleaseNewMessageQueue(MessageQueue::Handler2Queue(asyncreg_.Get()));
    MessageQueue::MessageQueueCreater::ReleaseNewMessageCreator(messagequeue_creater_);
#ifdef ANDROID
    delete wakeup_lock_;
#endif
}

void NetCore::ReleaseNet() {
    xinfo_function();
    already_release_net_ = true;
    ActiveLogic::Instance()->SignalActive.disconnect(boost::bind(&NetCore::__OnSignalActive, this, _1));

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {  // must disconnect signal
        auto longlink = longlink_task_manager_->DefaultLongLink();
        if (longlink && longlink->SignalKeeper()) {
            GetSignalOnNetworkDataChange().disconnect_all_slots();
        }
        longlink.reset();
        delete longlink_task_manager_;
    }
    push_preprocess_signal_.disconnect_all_slots();
    delete timing_sync_;
    delete zombie_task_manager_;
#endif
    delete shortlink_task_manager_;
    delete dynamic_timeout_;
    delete anti_avalanche_;
    delete netcheck_logic_;
    net_source_.reset();
}

void NetCore::__InitShortLink() {
    // async
    shortlink_task_manager_->fun_callback_ =
        boost::bind(&NetCore::__CallBack, this, (int)kCallFromShort, _1, _2, _3, _4, _5);

    // sync
    shortlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4, _5);
    shortlink_task_manager_->fun_notify_network_err_ =
        boost::bind(&NetCore::__OnShortLinkNetworkError, this, _1, _2, _3, _4, _5, _6);
    shortlink_task_manager_->fun_anti_avalanche_check_ =
        boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);
    shortlink_task_manager_->fun_shortlink_response_ = boost::bind(&NetCore::__OnShortLinkResponse, this, _1);
}

void NetCore::__InitLongLink() {
    xinfo_function();
#ifdef USE_LONG_LINK
    need_use_longlink_ = true;
    zombie_task_manager_ = new ZombieTaskManager(messagequeue_creater_.GetMessageQueue());
    zombie_task_manager_->fun_start_task_ = boost::bind(&NetCore::StartTask, this, _1);
    zombie_task_manager_->fun_callback_ =
        boost::bind(&NetCore::__CallBack, this, (int)kCallFromZombie, _1, _2, _3, _4, _5);

    timing_sync_ = new TimingSync(context_, *ActiveLogic::Instance());

    longlink_task_manager_ = new LongLinkTaskManager(context_,
                                                     net_source_,
                                                     *ActiveLogic::Instance(),
                                                     *dynamic_timeout_,
                                                     GetMessageQueueId(),
                                                     default_longlink_encoder);

    LonglinkConfig defaultConfig(DEFAULT_LONGLINK_NAME, DEFAULT_LONGLINK_GROUP, true);
    defaultConfig.is_keep_alive = true;
    defaultConfig.packer_encoder_name = packer_encoder_name_;
    defaultConfig.longlink_encoder = default_longlink_encoder;
    CreateLongLink(defaultConfig);

    // async
    longlink_task_manager_->fun_callback_ =
        boost::bind(&NetCore::__CallBack, this, (int)kCallFromLong, _1, _2, _3, _4, _5);

    // sync
    longlink_task_manager_->fun_notify_retry_all_tasks = boost::bind(&NetCore::RetryTasks, this, _1, _2, _3, _4, _5);
    longlink_task_manager_->fun_notify_network_err_ =
        boost::bind(&NetCore::__OnLongLinkNetworkError, this, _1, _2, _3, _4, _5, _6);
    longlink_task_manager_->fun_anti_avalanche_check_ = boost::bind(&AntiAvalanche::Check, anti_avalanche_, _1, _2, _3);

    longlink_task_manager_->fun_on_push_ = boost::bind(&NetCore::__OnPush, this, _1, _2, _3, _4, _5);
#endif
}

void NetCore::__Release(std::shared_ptr<NetCore> _instance) {
    xdebug_function(TSF "mars2");
    if (MessageQueue::CurrentThreadMessageQueue() != MessageQueue::Handler2Queue(_instance->asyncreg_.Get())) {
        WaitMessage(AsyncInvoke((MessageQueue::AsyncInvokeFunction)boost::bind(&NetCore::__Release, _instance),
                                _instance->asyncreg_.Get(),
                                "NetCore::__Release"));
        xdebug2(TSF "mars2 mq net_core WaitMessage AsyncInvoke __Release");
        return;
    }
    _instance.reset();
}

bool NetCore::__ValidAndInitDefault(Task& _task, XLogger& _group) {
    if (2 * 60 * 1000 < _task.server_process_cost) {
        xerror2(TSF "server_process_cost invalid:%_ ", _task.server_process_cost) >> _group;
        return false;
    }

    if (30 < _task.retry_count) {
        xerror2(TSF "retrycount invalid:%_ ", _task.retry_count) >> _group;
        return false;
    }

    if (10 * 60 * 1000 < _task.total_timeout) {
        xerror2(TSF "total_timeout invalid:%_ ", _task.total_timeout) >> _group;
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

    if (0 > _task.retry_count) {
        _task.retry_count = DEF_TASK_RETRY_COUNT;
    }

    //    if((_task.channel_select==Task::kChannelBoth||_task.channel_select==Task::kChannelLong)
    //       &&longlink_task_manager_->GetLongLink(_task.channel_name)==nullptr){
    //        return false;
    //    }

    return true;
}

int NetCore::__ChooseChannel(const Task& _task,
                             std::shared_ptr<LongLinkMetaData> _longlink,
                             std::shared_ptr<LongLinkMetaData> _minorLong) {
    bool longlinkOk = (_longlink != nullptr);
    longlinkOk = longlinkOk && LongLink::kConnected == _longlink->Channel()->ConnectStatus();

    if (longlinkOk && _task.channel_strategy == Task::kChannelFastStrategy) {
        xinfo2(TSF "long link task count:%0, ", longlink_task_manager_->GetTaskCount(_task.channel_name));
        longlinkOk = longlinkOk
                     && (longlink_task_manager_->GetTaskCount(_task.channel_name) <= kFastSendUseLonglinkTaskCntLimit);
    }

    bool minorOk = _task.minorlong_host_list.empty() ? false : longlink_task_manager_->IsMinorAvailable(_task);
    int channel = _task.channel_select;
    switch (_task.channel_select) {
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
    if (already_release_net_) {
        xinfo2(TSF "net core had release. ignore.");
        return;
    }
    xgroup2_define(group);
    xinfo2(TSF
           "task start long short taskid:%0, cmdid:%1, need_authed:%2, cgi:%3, channel_select:%4, limit_flow:%5, "
           "channel_name:%6",
           _task.taskid,
           _task.cmdid,
           _task.need_authed,
           _task.cgi.c_str(),
           _task.channel_select,
           _task.limit_flow,
           _task.channel_name)
        >> group;
    xinfo2(
        TSF
        "host:%_, send_only:%_, cmdid:%_, server_process_cost:%_, retrycount:%_,  channel_strategy:%_, channel_name:%_",
        _task.shortlink_host_list.empty() ? "" : _task.shortlink_host_list.front(),
        _task.send_only,
        _task.cmdid,
        _task.server_process_cost,
        _task.retry_count,
        _task.channel_strategy,
        _task.channel_name)
        >> group;
    xinfo2(TSF " total_timeout:%_, network_status_sensitive:%_, priority:%_, report_arg:%_",
           _task.total_timeout,
           _task.network_status_sensitive,
           _task.priority,
           _task.report_arg)
        >> group;
    PrepareProfile _profile;
    _profile.start_task_call_time = gettickcount();
    Task task = _task;
    if (!__ValidAndInitDefault(task, group)) {
        ConnectProfile profile;
        /* mars2
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalTaskParam, profile);
        */
        context_->GetManager<StnManager>()
            ->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalTaskParam, profile);
        return;
    }

#ifdef ANDROID
    /*cancel the last wakeuplock*/
    if (context_->GetManager<AppManager>() != nullptr) {
        wakeup_lock_->Lock(context_->GetManager<AppManager>()->GetConfig<int>(kKeyShortLinkWakeupLockBefroeCMD,
                                                                              kShortLinkWakeupLockBefroeCMD));
    } else {
        xinfo2(TSF "appmanager no exist.");
        wakeup_lock_->Lock(kShortLinkWakeupLockBefroeCMD);
    }
#endif

    _profile.begin_process_hosts_time = gettickcount();
    if (task_process_hook_) {
        task_process_hook_(task);
    }
    _profile.end_process_hosts_time = gettickcount();
    xdebug2(TSF "task process hook time %_", (_profile.end_process_hosts_time - _profile.begin_process_hosts_time));
#ifdef ANDROID
    wakeup_lock_->Unlock();
#endif
    if (0 == task.channel_select) {
        xerror2(TSF "error channelType (%_, %_), ", kEctLocal, kEctLocalChannelSelect) >> group;
        ConnectProfile profile;
        /*mars2
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalChannelSelect, profile);
         */
        context_->GetManager<StnManager>()
            ->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalChannelSelect, profile);
        return;
    }

    std::shared_ptr<LongLinkMetaData> longlink = nullptr;
    if (need_use_longlink_) {
        longlink = longlink_task_manager_->GetLongLink(task.channel_name);
    }

    //.下列逻辑是为了notify而做的，目前notify ack不需要在已有长连上进行，因此这个判断条件不需要了.
    // if ((task.channel_select == Task::kChannelLong || task.channel_select == Task::kChannelMinorLong) && (!longlink
    // || !longlink->IsConnected())){
    //     //.必须长链或副长链，但指定连接不存在，则回调失败.
    //     xerror2(TSF"err no longlink (%_, %_), ", kEctLocal, kEctLocalLongLinkUnAvailable) >> group;
    //     /* mars2
    //     OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalLongLinkUnAvailable,
    //     ConnectProfile());
    //     */
    //     context_->GetManager<StnManager>()->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal,
    //     kEctLocalLongLinkUnAvailable, ConnectProfile()); return;
    // }

    std::shared_ptr<LongLinkMetaData> minorlonglink = nullptr;
    if ((task.channel_select & Task::kChannelMinorLong) && !task.minorlong_host_list.empty()) {
        longlink_task_manager_->FixMinorRealhost(task);
        std::string host = task.minorlong_host_list.empty() ? "" : task.minorlong_host_list.front();
        if (!host.empty())
            minorlonglink = longlink_task_manager_->GetLongLink(host);
    }
    if (task.network_status_sensitive && kNoNet == ::getNetInfo()
#ifdef USE_LONG_LINK
        && longlink && LongLink::kConnected != longlink->Channel()->ConnectStatus()
#endif
    ) {
        xerror2(TSF "error no net (%_, %_), ", kEctLocal, kEctLocalNoNet) >> group;
        ConnectProfile profile;
        /* mars2
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
        */
        context_->GetManager<StnManager>()
            ->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
        return;
    }

#ifdef ANDROID
    if (kNoNet == ::getNetInfo() && !ActiveLogic::Instance()->IsActive()
#ifdef USE_LONG_LINK
        && longlink != nullptr
        && LongLink::kConnected != longlink_task_manager_->GetLongLink(task.channel_name)->Channel()->ConnectStatus()
#endif
    ) {
        xerror2(TSF "error no net (%_, %_) return when no active, ", kEctLocal, kEctLocalNoNet) >> group;
        ConnectProfile profile;
        /* mars2
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
         */
        context_->GetManager<StnManager>()
            ->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalNoNet, profile);
        return;
    }
#endif

    bool start_ok = false;

#ifdef USE_LONG_LINK
    if (need_use_longlink_ && longlink != nullptr && LongLink::kConnected != longlink->Channel()->ConnectStatus()
        && (Task::kChannelLong & task.channel_select) && ActiveLogic::Instance()->IsForeground()
        && (15 * 60 * 1000 >= gettickcount() - ActiveLogic::Instance()->LastForegroundChangeTime())) {
        longlink->Monitor()->MakeSureConnected();
    }
    if ((Task::kChannelMinorLong & task.channel_select) && ActiveLogic::Instance()->IsForeground()) {
        if (!minorlonglink) {
            bool ret = longlink_task_manager_->AddMinorLink(task.minorlong_host_list);
            if (ret) {
                minorlonglink = longlink_task_manager_->GetLongLink(task.minorlong_host_list.front());
                minorlonglink->Monitor()->MakeSureConnected();
            }
        } else if (LongLink::kConnected != minorlonglink->Channel()->ConnectStatus()) {
            minorlonglink->Channel()->SvrTrigOff();
            minorlonglink->Monitor()->MakeSureConnected();
        }
    }
#endif

    xgroup2() << group;
    if (!need_use_longlink_) {
        start_ok = shortlink_task_manager_->StartTask(task, _profile);
    } else {
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
                start_ok = shortlink_task_manager_->StartTask(task, _profile);
                break;

            default:
                xassert2(false);
                break;
        }
    }

    if (!start_ok) {
        xerror2(TSF "taskid:%_, error starttask (%_, %_)", task.taskid, kEctLocal, kEctLocalStartTaskFail);
        ConnectProfile profile;
        /* mars2
        OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalStartTaskFail, profile);
         */
        context_->GetManager<StnManager>()
            ->OnTaskEnd(task.taskid, task.user_context, task.user_id, kEctLocal, kEctLocalStartTaskFail, profile);
    } else {
#ifdef USE_LONG_LINK
        if (need_use_longlink_) {
            zombie_task_manager_->OnNetCoreStartTask();
        }
#endif
    }

    ASYNC_BLOCK_END
}

void NetCore::StopTask(uint32_t _taskid) {
    ASYNC_BLOCK_START

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        if (longlink_task_manager_->StopTask(_taskid))
            return;

        if (zombie_task_manager_->StopTask(_taskid))
            return;
    }
#endif

    if (shortlink_task_manager_->StopTask(_taskid))
        return;

    xerror2(TSF "task no found taskid:%0", _taskid);

    ASYNC_BLOCK_END
}

bool NetCore::HasTask(uint32_t _taskid) const {
    WAIT_SYNC2ASYNC_FUNC(boost::bind(&NetCore::HasTask, this, _taskid));

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        if (longlink_task_manager_->HasTask(_taskid))
            return true;

        if (zombie_task_manager_->HasTask(_taskid))
            return true;
    }
#endif
    if (shortlink_task_manager_->HasTask(_taskid))
        return true;

    return false;
}

void NetCore::ClearTasks() {
    ASYNC_BLOCK_START

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        longlink_task_manager_->ClearTasks();
        zombie_task_manager_->ClearTasks();
    }
#endif
    shortlink_task_manager_->ClearTasks();

    ASYNC_BLOCK_END
}

void NetCore::OnNetworkChange() {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::OnNetworkChange, this));  // if already messagequeue, no need to async

    xinfo_function();

    std::string ip_stack_log;
    TLocalIPStack ip_stack = local_ipstack_detect_log(ip_stack_log);

    switch (::getNetInfo()) {
        case kNoNet:
            xinfo2(TSF "task network change current network:no network");
            break;

        case kWifi: {
            WifiInfo info;
            getCurWifiInfo(info);
            xinfo2(TSF "task network change current network:wifi, ssid:%_, ip_stack:%_, log:%_",
                   info.ssid,
                   TLocalIPStackStr[ip_stack],
                   ip_stack_log);
        } break;

        case kMobile: {
            SIMInfo info;
            getCurSIMInfo(info);
            RadioAccessNetworkInfo raninfo;
            getCurRadioAccessNetworkInfo(raninfo);
            xinfo2(TSF
                   "task network change current network:mobile, ispname:%_, ispcode:%_, ran:%_, ip_stack:%_, log:%_",
                   info.isp_name,
                   info.isp_code,
                   raninfo.radio_access_network,
                   TLocalIPStackStr[ip_stack],
                   ip_stack_log);
        } break;

        case kOtherNet:
            xinfo2(TSF "task network change current network:other, ip_stack:%_, log:%_",
                   TLocalIPStackStr[ip_stack],
                   ip_stack_log);
            break;

        default:
            xassert2(false);
            break;
    }

    net_source_->ClearCache();
    dynamic_timeout_->ResetStatus();
#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        timing_sync_->OnNetworkChange();

        longlink_task_manager_->OnNetworkChange();

        zombie_task_manager_->RedoTasks();
    }
#endif

    shortlink_task_manager_->RedoTasks();

    shortlink_try_flag_ = false;
    shortlink_error_count_ = 0;
}

void NetCore::KeepSignal() {
    ASYNC_BLOCK_START
    if (!need_use_longlink_) {
        return;
    }
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if (!longlink)
        return;
    longlink->SignalKeeper()->Keep();
    ASYNC_BLOCK_END
}

void NetCore::StopSignal() {
    ASYNC_BLOCK_START
    if (!need_use_longlink_) {
        return;
    }
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if (!longlink)
        return;
    longlink->SignalKeeper()->Stop();
    ASYNC_BLOCK_END
}

#ifdef USE_LONG_LINK
void NetCore::DisconnectLongLinkByTaskId(uint32_t _taskid, LongLinkErrCode::TDisconnectInternalCode _code) {
    if (need_use_longlink_) {
        longlink_task_manager_->DisconnectByTaskId(_taskid, _code);
    }
}

//#ifdef __APPLE__
// void NetCore::__ResetLongLink() {
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
    if (need_use_longlink_) {
        longlink_task_manager_->RedoTasks();
        zombie_task_manager_->RedoTasks();
    }
#endif
    shortlink_task_manager_->RedoTasks();

    ASYNC_BLOCK_END
}

void NetCore::TouchTasks() {
    ASYNC_BLOCK_START
    xinfo_function();
#ifdef USE_LONG_LINK
    longlink_task_manager_->TouchTasks();
#endif
    shortlink_task_manager_->TouchTasks();
    ASYNC_BLOCK_END
}

void NetCore::RetryTasks(ErrCmdType _err_type,
                         int _err_code,
                         int _fail_handle,
                         uint32_t _src_taskid,
                         std::string _user_id) {
    ASYNC_BLOCK_START
    xinfo2(TSF "shortlink_task_manager retry task id %_", _src_taskid);
    shortlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid);
#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        longlink_task_manager_->RetryTasks(_err_type, _err_code, _fail_handle, _src_taskid, _user_id);
    }
#endif
    ASYNC_BLOCK_END
}

void NetCore::MakeSureLongLinkConnect() {
    if (!need_use_longlink_) {
        return;
    }
#ifdef USE_LONG_LINK
    ASYNC_BLOCK_START
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if (!longlink) {
        xassert2(false, TSF "longlink meta is null");
        return;
    }
    auto longlink_channel = longlink->Channel();
    if (longlink_channel) {
        longlink_channel->MakeSureConnected();
    } else {
        xassert2(false, TSF "longlink channel is null");
    }
    ASYNC_BLOCK_END
#endif
}

bool NetCore::LongLinkIsConnected() {
    if (!need_use_longlink_) {
        return false;
    }
#ifdef USE_LONG_LINK
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if (!longlink)
        return false;
    return LongLink::kConnected == longlink->Channel()->ConnectStatus();
#else
    return false;
#endif
}

int NetCore::__CallBack(int _from,
                        ErrCmdType _err_type,
                        int _err_code,
                        int _fail_handle,
                        const Task& _task,
                        unsigned int _taskcosttime) {
    if (task_callback_hook_ && 0 == task_callback_hook_(_from, _err_type, _err_code, _fail_handle, _task)) {
        xwarn2(TSF "task_callback_hook let task return. taskid:%_, cgi%_.", _task.taskid, _task.cgi);
        return 0;
    }

    ConnectProfile profile;
    if (kEctLocal == _err_type && kEctLocalReset == _err_code)  // ~MMCore
        /* mars2
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
         */
        return context_->GetManager<StnManager>()
            ->OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
    if (kEctOK == _err_type || kTaskFailHandleTaskEnd == _fail_handle)
        /* mars2
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code,
        GetConnectProfile(_task.taskid, _task.channel_select));
         */
        return context_->GetManager<StnManager>()->OnTaskEnd(_task.taskid,
                                                             _task.user_context,
                                                             _task.user_id,
                                                             _err_type,
                                                             _err_code,
                                                             GetConnectProfile(_task.taskid, _task.channel_select));

    if (kCallFromZombie == _from)
        /* mars2
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
        */
        return context_->GetManager<StnManager>()
            ->OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        if (!zombie_task_manager_->SaveTask(_task, _taskcosttime)) {
            /* mars2
            return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
            */
            return context_->GetManager<StnManager>()
                ->OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
        }
        return 0;
    } else {
        /* mars2
        return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
        */
        return context_->GetManager<StnManager>()
            ->OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
    }
#else
    /* mars2
    return OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
    */
    return context_->GetManager<StnManager>()
        ->OnTaskEnd(_task.taskid, _task.user_context, _task.user_id, _err_type, _err_code, profile);
#endif
}

void NetCore::__OnShortLinkResponse(int _status_code) {
    if (_status_code == 301 || _status_code == 302 || _status_code == 307) {
#ifdef USE_LONG_LINK
        if (need_use_longlink_) {
            auto longlink = longlink_task_manager_->DefaultLongLink();
            if (!longlink)
                return;
            LongLink::TLongLinkStatus longlink_status = longlink->Channel()->ConnectStatus();
            unsigned int continues_fail_count = longlink_task_manager_->GetTasksContinuousFailCount();
            xinfo2(TSF "status code:%0, long link status:%1, longlink task continue fail count:%2",
                   _status_code,
                   longlink_status,
                   continues_fail_count);

            if (LongLink::kConnected == longlink_status && continues_fail_count == 0) {
                return;
            }
        }

#endif
        // TODO callback
    }
}

#ifdef USE_LONG_LINK

void NetCore::__OnPush(const std::string& _channel_id,
                       uint32_t _cmdid,
                       uint32_t _taskid,
                       const AutoBuffer& _body,
                       const AutoBuffer& _extend) {
    xinfo2(TSF "task push name:%_, seq:%_, cmdid:%_, len:%_", _channel_id, _taskid, _cmdid, _body.Length());
    push_preprocess_signal_(_cmdid, _body);
    /* mars2
    OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
    */
    context_->GetManager<mars::stn::StnManager>()->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

void NetCore::__OnLongLinkNetworkError(const std::string& _name,
                                       int _line,
                                       ErrCmdType _err_type,
                                       int _err_code,
                                       const std::string& _ip,
                                       uint16_t _port) {
    xverbose_function(TSF "mars2");
    if (!need_use_longlink_) {
        return;
    }
    SYNC2ASYNC_FUNC(
        boost::bind(&NetCore::__OnLongLinkNetworkError, this, _name, _line, _err_type, _err_code, _ip, _port));
    xassert2(MessageQueue::CurrentThreadMessageQueue() == messagequeue_creater_.GetMessageQueue());

    if (already_release_net_) {
        return;
    }

    netcheck_logic_->UpdateLongLinkInfo(longlink_task_manager_->GetTasksContinuousFailCount(), _err_type == kEctOK);
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if (longlink && longlink->Config().IsMain()) {
        /* mars2
        OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
        */
        context_->GetManager<mars::stn::StnManager>()->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
    }

    if (kEctOK == _err_type)
        zombie_task_manager_->RedoTasks();

    if (kEctDial == _err_type)
        return;

    if (kEctHttp == _err_type)
        return;

    if (kEctServer == _err_type)
        return;

    if (kEctLocal == _err_type)
        return;

    net_source_->ReportLongIP(_err_type == kEctOK, _ip, _port);
}
#endif

void NetCore::__OnShortLinkNetworkError(int _line,
                                        ErrCmdType _err_type,
                                        int _err_code,
                                        const std::string& _ip,
                                        const std::string& _host,
                                        uint16_t _port) {
    SYNC2ASYNC_FUNC(
        boost::bind(&NetCore::__OnShortLinkNetworkError, this, _line, _err_type, _err_code, _ip, _host, _port));
    xassert2(MessageQueue::CurrentThreadMessageQueue() == messagequeue_creater_.GetMessageQueue());

    if (already_release_net_) {
        return;
    }

    netcheck_logic_->UpdateShortLinkInfo(shortlink_task_manager_->GetTasksContinuousFailCount(), _err_type == kEctOK);
    /* mars2
    OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
    */
    context_->GetManager<mars::stn::StnManager>()->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
    shortlink_try_flag_ = true;

    if (_err_type == kEctOK) {
        shortlink_error_count_ = 0;
    } else {
        ++shortlink_error_count_;
    }

    __ConnStatusCallBack();

#ifdef USE_LONG_LINK
    if (need_use_longlink_) {
        if (kEctOK == _err_type)
            zombie_task_manager_->RedoTasks();
    }
#endif

    if (kEctDial == _err_type)
        return;

    if (kEctNetMsgXP == _err_type)
        return;

    if (kEctServer == _err_type)
        return;

    if (kEctLocal == _err_type)
        return;

    net_source_->ReportShortIP(_err_type == kEctOK, _ip, _host, _port);
}

#ifdef USE_LONG_LINK
void NetCore::__OnLongLinkConnStatusChange(LongLink::TLongLinkStatus _status, const std::string& _channel_id) {
    xverbose_function(TSF "mars2");
    if (!need_use_longlink_) {
        return;
    }
    if (LongLink::kConnected == _status)
        zombie_task_manager_->RedoTasks();

    __ConnStatusCallBack();
    /* mars2
    OnLongLinkStatusChange(_status);
    */
    context_->GetManager<mars::stn::StnManager>()->OnLongLinkStatusChange(_status);
}
#endif

void NetCore::__ConnStatusCallBack() {
    xverbose_function(TSF "mars2");
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

    if (!need_use_longlink_) {
        if (shortlink_error_count_ >= kShortlinkErrTime) {
            all_connstatus = kServerFailed;
        } else if (0 == shortlink_error_count_) {
            all_connstatus = kConnected;
        } else {
            all_connstatus = kConnected;
        }
    } else {
#ifdef USE_LONG_LINK
        auto longlinkMeta = longlink_task_manager_->DefaultLongLink();
        if (!longlinkMeta)
            return;
        auto longlink = longlinkMeta->Channel();
        if (!longlink)
            return;
        longlink_connstatus = longlink->ConnectStatus();
        switch (longlink_connstatus) {
            case LongLink::kDisConnected:
                return;
            case LongLink::kConnectFailed:
                if (shortlink_try_flag_) {
                    if (0 == shortlink_error_count_) {
                        all_connstatus = kConnected;
                    } else if (shortlink_error_count_ >= kShortlinkErrTime) {
                        all_connstatus = kServerFailed;
                    } else {
                        all_connstatus = kNetworkUnkown;
                    }
                } else {
                    all_connstatus = kNetworkUnkown;
                }
                longlink_connstatus = kServerFailed;
                break;

            case LongLink::kConnectIdle:
            case LongLink::kConnecting:
                if (shortlink_try_flag_) {
                    if (0 == shortlink_error_count_) {
                        all_connstatus = kConnected;
                    } else if (shortlink_error_count_ >= kShortlinkErrTime) {
                        all_connstatus = kServerFailed;
                    } else {
                        all_connstatus = kConnecting;
                    }
                } else {
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
    }
    if (all_connstatus != all_connect_status_ || longlink_connstatus != longlink_connect_status_) {  // logs limits
        all_connect_status_ = all_connstatus;
        longlink_connect_status_ = longlink_connstatus;
        xinfo2(TSF "reportNetConnectInfo all_connstatus:%_, longlink_connstatus:%_",
               all_connstatus,
               longlink_connstatus);
    }
    /*mars2
    ReportConnectStatus(all_connstatus, longlink_connstatus);
    */
    context_->GetManager<mars::stn::StnManager>()->ReportConnectStatus(all_connstatus, longlink_connstatus);
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
    xdebug2(TSF "channel %_", _channel_select);
    if (_channel_select == Task::kChannelShort) {
        return shortlink_task_manager_->GetConnectProfile(_taskid);
    }
#ifdef USE_LONG_LINK
    else if (_channel_select == Task::kChannelLong || _channel_select == Task::kChannelMinorLong
             || _channel_select == Task::kChannelBoth) {
        if (need_use_longlink_) {
            return longlink_task_manager_->GetConnectProfile(_taskid);
        }
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
std::shared_ptr<LongLink> NetCore::CreateLongLink(LonglinkConfig& _config) {
    if (!need_use_longlink_) {
        return nullptr;
    }
    auto oldDefault = longlink_task_manager_->DefaultLongLink();
    if (!longlink_task_manager_->AddLongLink(_config)) {
        auto longlink = longlink_task_manager_->GetLongLink(_config.name);
        if (longlink) {
            xwarn2(TSF "already has longlink named:%_", _config.name);
            return longlink_task_manager_->GetLongLink(_config.name)->Channel();
        }
    }

    auto longlink = longlink_task_manager_->GetLongLink(_config.name);
    if (!longlink) {
        xassert2(false, TSF "get longlink nullptr with name:%_", _config.name.c_str());
        return nullptr;
    }
    auto longlink_channel = longlink->Channel();
    if (longlink == nullptr || longlink_channel == nullptr) {
        xassert2(false, TSF "get longlink nullptr with name:%_", _config.name.c_str());
        return nullptr;
    }

    if (_config.IsMain() && oldDefault) {
        xinfo2(TSF "change default longlink to name:%_, group:%_", _config.name, _config.group);
        oldDefault->Channel()->SignalConnection.disconnect(
            boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
        oldDefault->Channel()->SignalConnection.disconnect(
            boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
        GetSignalOnNetworkDataChange().disconnect(
            boost::bind(&SignallingKeeper::OnNetWorkDataChanged, oldDefault->SignalKeeper().get(), _1, _2, _3));
        oldDefault->Config().isMain = false;
    }

    if (_config.IsMain()) {
        longlink_channel->fun_network_report_ =
            boost::bind(&NetCore::__OnLongLinkNetworkError, this, _config.name, _1, _2, _3, _4, _5);
        longlink_channel->SignalConnection.connect(
            boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
        longlink_channel->SignalConnection.connect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
        if (longlink->SignalKeeper()) {
            GetSignalOnNetworkDataChange().connect(
                boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
        }
    }

    xinfo2(TSF "create long link %_", _config.name);
    return longlink_channel;
}

bool NetCore::AddMinorLongLink(const std::vector<std::string>& _hosts) {
    return longlink_task_manager_->AddMinorLink(_hosts);
}

void NetCore::ForbidLonglinkTlsHost(const std::vector<std::string>& _host) {
    longlink_task_manager_->AddForbidTlsHost(_host);
}

void NetCore::InitHistory2BannedList() {
    auto once_fun = [this]() {
        xdebug2("InitHistory2BannedList");
        if (net_source_) {
            net_source_->InitHistory2BannedList(false);
        }
    };
    /*NO_DESTROY static */ std::once_flag of;
    std::call_once(of, once_fun);
}

void NetCore::SetIpConnectTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout) {
    SYNC2ASYNC_FUNC(boost::bind(&NetCore::SetIpConnectTimeout, this, _v4_timeout, _v6_timeout));
    if (net_source_) {
        xinfo2(TSF "receive ip timeout: %_, %_", _v4_timeout, _v6_timeout);
        net_source_->SetIpConnectTimeout(_v4_timeout, _v6_timeout);
    }
}

void NetCore::DestroyLongLink(const std::string& _name) {
    WAIT_SYNC2ASYNC_FUNC(boost::bind(&NetCore::DestroyLongLink, this, _name));
    if (!need_use_longlink_) {
        xwarn2("doesn't use longlink channel");
        return;
    }
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if (!longlink) {
        xwarn2(TSF "destroy long link failure: no such long link exists %_", _name);
        return;
    }

    if (longlink->SignalKeeper()) {
        GetSignalOnNetworkDataChange().disconnect(
            boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
    }
    longlink->Channel()->SignalConnection.disconnect_all_slots();
    longlink->Channel()->broadcast_linkstatus_signal_.disconnect_all_slots();
    longlink.reset();  // do not hold the shared_ptr

    longlink_task_manager_->ReleaseLongLink(_name);
    xinfo2(TSF "destroy long link %_ ", _name);
}

void NetCore::MarkMainLonglink_ext(const std::string& _name) {
    auto oldLink = DefaultLongLink();
    auto oldMeta = DefaultLongLinkMeta();
    auto newLinkMeta = GetLongLink(_name);
    auto newLink = newLinkMeta ? newLinkMeta->Channel() : nullptr;
    if (oldLink == nullptr || newLink == nullptr || oldMeta == nullptr || oldMeta->Config().name == _name) {
        xerror2(TSF "link nullptr, old:%_, new:%_, or same longlink", (oldLink == nullptr), (newLink == nullptr));
        return;
    }

    xinfo2(TSF "change default longlink to name:%_", _name);
    oldLink->SignalConnection.disconnect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));
    oldLink->SignalConnection.disconnect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
    GetSignalOnNetworkDataChange().disconnect(
        boost::bind(&SignallingKeeper::OnNetWorkDataChanged, oldMeta->SignalKeeper().get(), _1, _2, _3));
    oldMeta->Config().isMain = false;

    newLink->fun_network_report_ = boost::bind(&NetCore::__OnLongLinkNetworkError, this, _name, _1, _2, _3, _4, _5);
    newLink->SignalConnection.connect(boost::bind(&TimingSync::OnLongLinkStatuChanged, timing_sync_, _1, _2));
    newLink->SignalConnection.connect(boost::bind(&NetCore::__OnLongLinkConnStatusChange, this, _1, _2));

    auto longlink = GetLongLink(_name);
    if (longlink && longlink->SignalKeeper()) {
        GetSignalOnNetworkDataChange().connect(
            boost::bind(&SignallingKeeper::OnNetWorkDataChanged, longlink->SignalKeeper().get(), _1, _2, _3));
    }
    longlink->Config().isMain = true;

    return;
}

void NetCore::MakeSureLongLinkConnect_ext(const std::string& _name) {
    ASYNC_BLOCK_START
    if (!need_use_longlink_) {
        return;
    }
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if (longlink) {
        longlink->Channel()->MakeSureConnected();
    }
    ASYNC_BLOCK_END
}

bool NetCore::LongLinkIsConnected_ext(const std::string& _name) {
    if (!need_use_longlink_) {
        return false;
    }
    auto longlink = longlink_task_manager_->GetLongLink(_name);
    if (longlink) {
        if (longlink->Channel()->ConnectStatus() == LongLink::kConnected) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<LongLink> NetCore::DefaultLongLink() {
    if (!need_use_longlink_) {
        return nullptr;
    }
    auto longlink = longlink_task_manager_->DefaultLongLink();
    if (!longlink)
        return nullptr;
    return longlink->Channel();
}

std::shared_ptr<LongLinkMetaData> NetCore::DefaultLongLinkMeta() {
    if (!need_use_longlink_) {
        return nullptr;
    }
    return longlink_task_manager_->DefaultLongLink();
}

std::shared_ptr<LongLinkMetaData> NetCore::GetLongLink(const std::string& _name) {
    if (!need_use_longlink_) {
        return nullptr;
    }
    return longlink_task_manager_->GetLongLink(_name);
}

void NetCore::SetDebugHost(const std::string& _host) {
    shortlink_task_manager_->SetDebugHost(_host);
}

std::shared_ptr<NetSource> NetCore::GetNetSource() {
    return net_source_;
}

int NetCore::GetPackerEncoderVersion() {
    return packer_encoder_version_;
}

std::string NetCore::GetPackerEncoderName() {
    return packer_encoder_name_;
}

void NetCore::SetNeedUseLongLink(bool flag) {
    need_use_longlink_ = flag;
}

void NetCore::SetGetRealHostFunc(
    const std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist, const std::map<std::string, std::string>& extra_info)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->get_real_host_strict_match_ = func;
    }
}

void NetCore::SetAddWeakNetInfo(const std::function<void(bool _connect_timeout, struct tcp_info& _info)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->add_weaknet_info_ = func;
    }
}

void NetCore::SetLongLinkGetRealHostFunc(
    std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist, bool _strict_match, const std::map<std::string, std::string>& extra_info)> func) {
    if (longlink_task_manager_) {
        longlink_task_manager_->get_real_host_ = func;
    }
}

void NetCore::SetLongLinkOnHandShakeReady(
    std::function<void(uint32_t _version, mars::stn::TlsHandshakeFrom _from)> func) {
    if (longlink_task_manager_) {
        xdebug2(TSF "mars2 SetLongLinkOnHandShakeReady suss.");
        longlink_task_manager_->on_handshake_ready_ = func;
    } else {
        xinfo2(TSF "mars2 SetLongLinkOnHandShakeReady fail.");
    }
}

void NetCore::SetLongLinkShouldInterceptResult(std::function<bool(int _error_code)> func) {
    if (longlink_task_manager_) {
        longlink_task_manager_->should_intercept_result_ = func;
    }
}

void NetCore::SetShortLinkGetRealHostFunc(
    std::function<size_t(const std::string& _user_id, std::vector<std::string>& _hostlist, bool _strict_match, const std::map<std::string, std::string>& extra_info)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->get_real_host_ = func;
    }
}

void NetCore::SetShortLinkTaskConnectionDetail(
    std::function<void(const int _error_type, const int _error_code, const int _use_ip_index, const std::map<std::string, std::string>& extra_info)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->task_connection_detail_ = func;
    }
}

void NetCore::SetShortLinkChooseProtocol(std::function<int(TaskProfile& _profile)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->choose_protocol_ = func;
    }
}

void NetCore::SetShortLinkOnTimeoutOrRemoteShutdown(std::function<void(const TaskProfile& _profile)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->on_timeout_or_remote_shutdown_ = func;
    }
}

void NetCore::SetShortLinkOnHandShakeReady(
    std::function<void(uint32_t _version, mars::stn::TlsHandshakeFrom _from)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->on_handshake_ready_ = func;
    }
}

void NetCore::SetShortLinkCanUseTls(std::function<bool(const std::vector<std::string>& _host_list)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->can_use_tls_ = func;
    }
}

void NetCore::SetShortLinkShouldInterceptResult(std::function<bool(int _error_code)> func) {
    if (shortlink_task_manager_) {
        shortlink_task_manager_->should_intercept_result_ = func;
    }
}

bool NetCore::IsAlreadyRelease() {
    return already_release_net_;
}

#endif
