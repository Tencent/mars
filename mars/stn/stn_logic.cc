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
//  stn_logic.cc
//  network
//
//  Created by yanguoyue on 16/2/18.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include "mars/stn/stn_logic.h"

#include <stdlib.h>

#include <map>

#include "mars/log/appender.h"

#include "mars/baseevent/baseprjevent.h"
#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseevent.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/singleton.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/alarm.h"
#include "mars/boost/signals2.hpp"
#include "stn/src/net_core.h"//一定要放这里，Mac os 编译
#include "stn/src/net_source.h"
#include "stn/src/signalling_keeper.h"
#include "stn/src/proxy_test.h"

#ifdef WIN32
#include <locale>
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#endif

#include "mars/comm/thread/atomic_oper.h"
#include "mars/stn/stn_callback_bridge.h"
#include "mars/stn/stn_manager.h"
#include "mars/stn/stn_callback_bridge.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";


static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

static void onCreate() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->Init();
    stnManager->OnCreate();
}

static void onDestroy() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnDestroy();
}

static void onSingalCrash(int _sig) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnSingalCrash(_sig);
}

static void onExceptionCrash() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnExceptionCrash();
}

static void onNetworkChange(void (*pre_change)()) {
    xinfo2(TSF "cpan debug OnNetworkChange");
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnNetworkChange(pre_change);
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnNetworkDataChange(_tag, _send, _recv);
}

#ifdef ANDROID
//must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnAlarm(_id);
}
#endif

static void __initbind_baseprjevent() {

#ifdef WIN32
	boost::filesystem::path::imbue(std::locale(std::locale(), new boost::filesystem::detail::utf8_codecvt_facet));
#endif

#ifdef ANDROID
    mars::baseevent::addLoadModule(kLibName);
    GetSignalOnAlarm().connect(&onAlarm);
#endif
    GetSignalOnCreate().connect(&onCreate);
    GetSignalOnInitBeforeOnCreate().connect(boost::bind(&onInitConfigBeforeOnCreate, _1));
    GetSignalOnDestroy().connect(&onDestroy);   //low priority signal func
    GetSignalOnSingalCrash().connect(&onSingalCrash);
    GetSignalOnExceptionCrash().connect(&onExceptionCrash);
    GetSignalOnNetworkChange().connect(5, boost::bind(&onNetworkChange, &mars::comm::OnPlatformNetworkChange));    //define group 5

    
#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif
    
    GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);
// callback
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->MakesureAuthed(_host, _user_id);
}

//流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->TrafficData(_send, _recv);
}

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->OnNewDns(_host, _longlink_host);
}
//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}
//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}
//任务执行结束
int OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ReportConnectStatus(status, longlink_status);
}

void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void OnLongLinkStatusChange(int _status) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->OnLongLinkStatusChange(_status);
}
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}
//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void RequestSync() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->RequestSync();
}
//验证是否已登录

//底层询问上层http网络检查的域名列表
void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->RequestNetCheckShortLinkHosts(_hostlist);
}
//底层向上层上报cgi执行结果
void ReportTaskProfile(const TaskProfile& _task_profile) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ReportTaskProfile(_task_profile);
}
//底层通知上层cgi命中限制
void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ReportTaskLimited(_check_type, _task, _param);
}
//底层上报域名dns结果
void ReportDnsProfile(const DnsProfile& _dns_profile) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ReportDnsProfile(_dns_profile);
}
//.生成taskid.
uint32_t GenTaskID() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->GenTaskID();
}

void SetCallback(Callback* const callback) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetCallback(callback);
}

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetStnCallbackBridge(_callback_bridge);
}

 StnCallbackBridge* GetStnCallbackBridge() {
     StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->GetStnCallbackBridge();
}

bool (*StartTask)(const Task& _task) = [](const Task& _task) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->StartTask(_task);
};

void (*StopTask)(uint32_t _taskid) = [](uint32_t _taskid) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->StopTask(_taskid);
};

bool (*HasTask)(uint32_t _taskid) = [](uint32_t _taskid) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->HasTask(_taskid);
};

void (*DisableLongLink)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->DisableLongLink();
};

void (*RedoTasks)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->RedoTasks();
};

void (*TouchTasks)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->TouchTasks();
};

void (*ClearTasks)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ClearTasks();
};

void (*Reset)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->Reset();
};

void (*ResetAndInitEncoderVersion)(int _packer_encoder_version) = [](int _packer_encoder_version) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->ResetAndInitEncoderVersion(_packer_encoder_version);

};

void (*MakesureLonglinkConnected)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->MakesureLonglinkConnected();
};

bool (*LongLinkIsConnected)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->LongLinkIsConnected();
};

bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) =
    [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
        StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
        return stnManager->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    };

//void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports)
// {
//	SetLonglinkSvrAddr(host, ports, "");
//};

void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) = [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetLonglinkSvrAddr(host, ports, debugip);
};

//void SetShortlinkSvrAddr(const uint16_t port)
//{
//	NetSource::SetShortlink(port, "");
//};

void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip) = [](const uint16_t port, const std::string& debugip) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetShortlinkSvrAddr(port, debugip);
};

void (*SetDebugIP)(const std::string& host, const std::string& ip) = [](const std::string& host, const std::string& ip) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetDebugIP(host, ip);
};

void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist) = [](const std::string& host, const std::vector<std::string>& iplist) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetBackupIPs(host, iplist);
};

void (*SetSignallingStrategy)(long _period, long _keepTime) = [](long _period, long _keepTime) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->SetSignallingStrategy(_period, _keepTime);
};

void (*KeepSignalling)() = []() {
#ifdef USE_LONG_LINK
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->KeepSignalling();
#endif
};

void (*StopSignalling)() = []() {
#ifdef USE_LONG_LINK
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->StopSignalling();
#endif
};

uint32_t (*getNoopTaskID)() = []() {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->getNoopTaskID();
};

void (*CreateLonglink_ext)(LonglinkConfig& _config) = [](LonglinkConfig& _config) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->CreateLonglink_ext(_config);
};

void (*DestroyLonglink_ext)(const std::string& name) = [](const std::string& name) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->DestroyLonglink_ext(name);
};

bool (*LongLinkIsConnected_ext)(const std::string& name) = [](const std::string& name) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    return stnManager->LongLinkIsConnected_ext(name);
};

void (*MarkMainLonglink_ext)(const std::string& name) = [](const std::string& name) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->MarkMainLonglink_ext(name);
};

void (*MakesureLonglinkConnected_ext)(const std::string& name) = [](const std::string& name) {
    StnManager* stnManager = (StnManager*)mars::boot::CreateContext("default")->GetManager("Stn");
    stnManager->MakesureLonglinkConnected_ext(name);
};

// ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select) {
//    return stnManager->GetConnectProfile(_taskid, _channel_select);
//}
// void AddServerBan(const std::string& _ip) {
//    stnManager->AddServerBan(_ip);
//}
//
// void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLink::TDisconnectInternalCode _code) {
//    stnManager->DisconnectLongLinkByTaskId(_taskid, _code);
//}
void network_export_symbols_0() {
}

}  // namespace stn
}  // namespace mars
