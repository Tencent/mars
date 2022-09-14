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
#include <string>

#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseevent.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/boost/signals2.hpp"
#include "mars/comm/alarm.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/singleton.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/log/appender.h"
#include "stn/src/net_core.h"  //一定要放这里，Mac os 编译
#include "stn/src/net_source.h"
#include "stn/src/proxy_test.h"
#include "stn/src/signalling_keeper.h"

#ifdef WIN32
#include <locale>

#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#include "boost/filesystem/path.hpp"
#endif

#include "mars/comm/thread/atomic_oper.h"
#include "mars/stn/stn_callback_bridge.h"
#include "mars/stn/stn_manager.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

static void onCreate() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->Init();
    context->GetStnManager()->OnCreate();
}

static void onDestroy() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnDestroy();
}

static void onSingalCrash(int _sig) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnSingalCrash(_sig);
}

static void onExceptionCrash() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnExceptionCrash();
}

static void onNetworkChange() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnNetworkChange();
}

static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnNetworkDataChange(_tag, _send, _recv);
}

#ifdef ANDROID
// must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnAlarm(_id);
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
    GetSignalOnDestroy().connect(&onDestroy);  // low priority signal func
    GetSignalOnSingalCrash().connect(&onSingalCrash);
    GetSignalOnExceptionCrash().connect(&onExceptionCrash);
    GetSignalOnNetworkChange().connect(5, &onNetworkChange);  // define group 5

#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif

    GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);
// callback
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->MakesureAuthed(_host, _user_id);
}

//流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->TrafficData(_send, _recv);
}

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->OnNewDns(_host, _longlink_host);
}
//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}
//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}
//任务执行结束
int OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->ReportConnectStatus(status, longlink_status);
}

void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void OnLongLinkStatusChange(int _status) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->OnLongLinkStatusChange(_status);
}
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}
//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void RequestSync() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->RequestSync();
}
//验证是否已登录

//底层询问上层http网络检查的域名列表
void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    BaseContext* context = CreateContext("default");
    //TODO cpan mars2
    //    context->GetStnManager()->RequestNetCheckShortLinkHosts(_hostlist);
}
//底层向上层上报cgi执行结果
void ReportTaskProfile(const TaskProfile& _task_profile) {
    BaseContext* context = CreateContext("default");
    //context->GetStnManager()->ReportTaskProfile(_task_profile);
}
//底层通知上层cgi命中限制
void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    BaseContext* context = CreateContext("default");
    //context->GetStnManager()->ReportTaskLimited(_check_type, _task, _param);
}
//底层上报域名dns结果
void ReportDnsProfile(const DnsProfile& _dns_profile) {
    BaseContext* context = CreateContext("default");
    //context->GetStnManager()->ReportDnsProfile(_dns_profile);
}
//.生成taskid.
uint32_t GenTaskID() {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->GenTaskID();
}

void SetCallback(Callback* const callback) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetCallback(callback);
}

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    BaseContext* context = CreateContext("default");
    //TODO cpan mars2
    //context->GetStnManager()->SetStnCallbackBridge(_callback_bridge);
}

// StnCallbackBridge* GetStnCallbackBridge() {
//    BaseContext* context = CreateContext("default");
//    return context->GetStnManager()->GetStnCallbackBridge();
//}

bool (*StartTask)(const Task& _task) = [](const Task& _task) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->StartTask(_task);
};

void (*StopTask)(uint32_t _taskid) = [](uint32_t _taskid) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->StopTask(_taskid);
};

bool (*HasTask)(uint32_t _taskid) = [](uint32_t _taskid) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->HasTask(_taskid);
};

void (*DisableLongLink)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->DisableLongLink();
};

void (*RedoTasks)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->RedoTasks();
};

void (*TouchTasks)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->TouchTasks();
};

void (*ClearTasks)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->ClearTasks();
};

void (*Reset)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->Reset();
};

void (*ResetAndInitEncoderVersion)(int _packer_encoder_version) = [](int _packer_encoder_version) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->ResetAndInitEncoderVersion(_packer_encoder_version);
};

void (*MakesureLonglinkConnected)() = []() {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->MakesureLonglinkConnected();
};

bool (*LongLinkIsConnected)() = []() {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->LongLinkIsConnected();
};

bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) =
    [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
        BaseContext* context = CreateContext("default");
        return context->GetStnManager()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    };

// void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports)
// {
//	SetLonglinkSvrAddr(host, ports, "");
//};

void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) = [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetLonglinkSvrAddr(host, ports, debugip);
};

// void SetShortlinkSvrAddr(const uint16_t port)
//{
//	NetSource::SetShortlink(port, "");
//};

void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip) = [](const uint16_t port, const std::string& debugip) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetShortlinkSvrAddr(port, debugip);
};

void (*SetDebugIP)(const std::string& host, const std::string& ip) = [](const std::string& host, const std::string& ip) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetDebugIP(host, ip);
};

void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist) = [](const std::string& host, const std::vector<std::string>& iplist) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetBackupIPs(host, iplist);
};

void (*SetSignallingStrategy)(long _period, long _keepTime) = [](long _period, long _keepTime) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->SetSignallingStrategy(_period, _keepTime);
};

void (*KeepSignalling)() = []() {
#ifdef USE_LONG_LINK
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->KeepSignalling();
#endif
};

void (*StopSignalling)() = []() {
#ifdef USE_LONG_LINK
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->StopSignalling();
#endif
};

uint32_t (*getNoopTaskID)() = []() {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->getNoopTaskID();
};

void (*CreateLonglink_ext)(LonglinkConfig& _config) = [](LonglinkConfig& _config) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->CreateLonglink_ext(_config);
};

void (*DestroyLonglink_ext)(const std::string& name) = [](const std::string& name) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->DestroyLonglink_ext(name);
};

bool (*LongLinkIsConnected_ext)(const std::string& name) = [](const std::string& name) {
    BaseContext* context = CreateContext("default");
    return context->GetStnManager()->LongLinkIsConnected_ext(name);
};

void (*MarkMainLonglink_ext)(const std::string& name) = [](const std::string& name) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->MarkMainLonglink_ext(name);
};

void (*MakesureLonglinkConnected_ext)(const std::string& name) = [](const std::string& name) {
    BaseContext* context = CreateContext("default");
    context->GetStnManager()->MakesureLonglinkConnected_ext(name);
};

// ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select) {
//    return context->GetStnManager()->GetConnectProfile(_taskid, _channel_select);
//}
// void AddServerBan(const std::string& _ip) {
//    context->GetStnManager()->AddServerBan(_ip);
//}
//
// void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLink::TDisconnectInternalCode _code) {
//    context->GetStnManager()->DisconnectLongLinkByTaskId(_taskid, _code);
//}
void network_export_symbols_0() {
}

}  // namespace stn
}  // namespace mars
