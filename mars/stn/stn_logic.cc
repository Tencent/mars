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
#include <string>
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

#include "stn_manager.h"
#include "mars/comm/thread/atomic_oper.h"
#include "mars/stn/stn_callback_bridge.h"

using namespace mars::comm;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";
static std::shared_ptr<StnManager> stn_manager;

//#define STN_WEAK_CALL(func) \
//    boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
//    if (!stn_ptr) {\
//        xwarn2(TSF"stn uncreate");\
//        return;\
//    }\
//    stn_ptr->func
//
//#define STN_RETURN_WEAK_CALL(func) \
//    boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
//    if (!stn_ptr) {\
//        xwarn2(TSF"stn uncreate");\
//        return false;\
//    }\
//    stn_ptr->func;\
//    return true
//
//#define STN_WEAK_CALL_RETURN(func, ret) \
//	boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
//    if (stn_ptr) \
//    {\
//    	ret = stn_ptr->func;\
//    }

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    xinfo2(TSF"stn oninit: %_", _packer_encoder_version);
    //LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
    stn_manager->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

static void onCreate() {
//#if !UWP && !defined(WIN32)
//    signal(SIGPIPE, SIG_IGN);
//#endif
//    xinfo2(TSF"stn oncreate");
//    ActiveLogic::Instance();
//    NetCore::Singleton::Instance();
    if(!stn_manager){
        stn_manager = std::make_shared<StnManager>();
    }
    stn_manager->Init();
    stn_manager->OnCreate();
}

static void onDestroy() {
    xinfo2(TSF"stn onDestroy");

    //NetCore::Singleton::Release();
    //SINGLETON_RELEASE_ALL();
    
    // others use activelogic may crash after activelogic release. eg: LongLinkConnectMonitor
    // ActiveLogic::Singleton::Release();
    stn_manager->OnDestroy();
}

static void onSingalCrash(int _sig) {
    //mars::xlog::appender_close();
    stn_manager->OnSingalCrash(_sig);
}

static void onExceptionCrash() {
    //mars::xlog::appender_close();
    stn_manager->OnExceptionCrash();
}

static void onNetworkChange() {

    //STN_WEAK_CALL(OnNetworkChange());
    stn_manager->OnNetworkChange();
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    
    /*if (NULL == _tag || strnlen(_tag, 1024) == 0) {
        xassert2(false);
        return;
    }
    
    if (0 == strcmp(_tag, XLOGGER_TAG)) {
        TrafficData(_send, _recv);
    }*/
    stn_manager->OnNetworkDataChange(_tag, _send, _recv);
}

#ifdef ANDROID
//must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    //Alarm::onAlarmImpl(_id);
    stn_manager->OnAlarm(_id);
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
    GetSignalOnNetworkChange().connect(5, &onNetworkChange);    //define group 5

    
#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif
    
    GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);
//callback
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    return stn_manager->MakesureAuthed(_host, _user_id);
}

//流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    stn_manager->TrafficData(_send, _recv);
}

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) {
    return stn_manager->OnNewDns(_host, _longlink_host);
}
//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    stn_manager->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}
//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    return stn_manager->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    return stn_manager->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}
//任务执行结束
int  OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    return stn_manager->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    stn_manager->ReportConnectStatus(status, longlink_status);
}

void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    stn_manager->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    stn_manager->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void OnLongLinkStatusChange(int _status) {
    stn_manager->OnLongLinkStatusChange(_status);
}
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    return stn_manager->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}
//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    return stn_manager->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void RequestSync() {
    stn_manager->RequestSync();
}
//验证是否已登录

//底层询问上层http网络检查的域名列表
void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    stn_manager->RequestNetCheckShortLinkHosts(_hostlist);
}
//底层向上层上报cgi执行结果
void ReportTaskProfile(const TaskProfile& _task_profile) {
    stn_manager->ReportTaskProfile(_task_profile);
}
//底层通知上层cgi命中限制
void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    stn_manager->ReportTaskLimited(_check_type, _task, _param);
}
//底层上报域名dns结果
void ReportDnsProfile(const DnsProfile& _dns_profile) {
    stn_manager->ReportDnsProfile(_dns_profile);
}
//.生成taskid.
uint32_t GenTaskID() {
    return stn_manager->GenTaskID();
}

void SetCallback(Callback* const callback) {
    if(!stn_manager){
        stn_manager = std::make_shared<StnManager>();
        //stn_manager->Init();//不可以这里init
    }
    stn_manager->SetCallback(callback);
}

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge){
    if(!stn_manager){
        stn_manager = std::make_shared<StnManager>();
        //stn_manager->Init();//不可以这里init
    }
    stn_manager->SetStnCallbackBridge(_callback_bridge);
}

StnCallbackBridge* GetStnCallbackBridge() {
    if(!stn_manager){
        stn_manager = std::make_shared<StnManager>();
        //stn_manager->Init();//不可以这里init
    }
    return stn_manager->GetStnCallbackBridge();
}

bool (*StartTask)(const Task& _task)
= [](const Task& _task) {
    //STN_RETURN_WEAK_CALL(StartTask(_task));
    return stn_manager->StartTask(_task);
};

void (*StopTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
    //STN_WEAK_CALL(StopTask(_taskid));
    stn_manager->StopTask(_taskid);
};

bool (*HasTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
//	bool has_task = false;
//	STN_WEAK_CALL_RETURN(HasTask(_taskid), has_task);
//	return has_task;
    return stn_manager->HasTask(_taskid);
};

void (*DisableLongLink)()
= []() {
    //NetCore::need_use_longlink_ = false;
    stn_manager->DisableLongLink();
};

void (*RedoTasks)()
= []() {
   //STN_WEAK_CALL(RedoTasks());
   stn_manager->RedoTasks();
};

void (*TouchTasks)()
= []() {
   //STN_WEAK_CALL(TouchTasks());
   stn_manager->TouchTasks();
};

void (*ClearTasks)()
= []() {
   //STN_WEAK_CALL(ClearTasks());
   stn_manager->ClearTasks();
};

void (*Reset)()
= []() {
	/*xinfo2(TSF "stn reset");
	NetCore::Singleton::Release();
	NetCore::Singleton::Instance();*/
    stn_manager->Reset();
};

void (*ResetAndInitEncoderVersion)(int _packer_encoder_version)
= [](int _packer_encoder_version) {
	/*xinfo2(TSF "stn reset, encoder version: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
	NetCore::Singleton::Release();
	NetCore::Singleton::Instance();*/
    stn_manager->ResetAndInitEncoderVersion(_packer_encoder_version);
};

void (*MakesureLonglinkConnected)()
= []() {
   //xinfo2(TSF "make sure longlink connect");
   //STN_WEAK_CALL(MakeSureLongLinkConnect());
   stn_manager->MakesureLonglinkConnected();
};

bool (*LongLinkIsConnected)()
= []() {
    /*bool connected = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected(), connected);
    return connected;*/
    return stn_manager->LongLinkIsConnected();
};
    
bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips)
= [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips){
    
    //return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    return stn_manager->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
};

//void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports)
// {
//	SetLonglinkSvrAddr(host, ports, "");
//};


void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip)
= [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
	/*std::vector<std::string> hosts;
	if (!host.empty()) {
		hosts.push_back(host);
	}
	NetSource::SetLongLink(hosts, ports, debugip);*/
    stn_manager->SetLonglinkSvrAddr(host, ports, debugip);
};

//void SetShortlinkSvrAddr(const uint16_t port)
//{
//	NetSource::SetShortlink(port, "");
//};
    
void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip)
= [](const uint16_t port, const std::string& debugip) {
	//NetSource::SetShortlink(port, debugip);
    stn_manager->SetShortlinkSvrAddr(port, debugip);
};

void (*SetDebugIP)(const std::string& host, const std::string& ip)
= [](const std::string& host, const std::string& ip) {
	//NetSource::SetDebugIP(host, ip);
    stn_manager->SetDebugIP(host, ip);
};
    
void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist)
= [](const std::string& host, const std::vector<std::string>& iplist) {
	//NetSource::SetBackupIPs(host, iplist);
    stn_manager->SetBackupIPs(host, iplist);
};

void (*SetSignallingStrategy)(long _period, long _keepTime)
= [](long _period, long _keepTime) {
    //SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
    stn_manager->SetSignallingStrategy(_period, _keepTime);
};

void (*KeepSignalling)()
= []() {
#ifdef USE_LONG_LINK
    //STN_WEAK_CALL(KeepSignal());
    stn_manager->KeepSignalling();
#endif
};

void (*StopSignalling)()
= []() {
#ifdef USE_LONG_LINK
    //STN_WEAK_CALL(StopSignal());
    stn_manager->StopSignalling();
#endif
};

uint32_t (*getNoopTaskID)()
= []() {
	//return Task::kNoopTaskID;
   return stn_manager->getNoopTaskID();
};

void (*CreateLonglink_ext)(LonglinkConfig& _config)
= [](LonglinkConfig & _config){
    //STN_WEAK_CALL(CreateLongLink(_config));
    stn_manager->CreateLonglink_ext(_config);
};
    
void (*DestroyLonglink_ext)(const std::string& name)
= [](const std::string& name){
    //STN_WEAK_CALL(DestroyLongLink(name));
    stn_manager->DestroyLonglink_ext(name);
};

bool (*LongLinkIsConnected_ext)(const std::string& name)
= [](const std::string& name){
    /*bool res = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected_ext(name),res);
    return res;*/
    return stn_manager->LongLinkIsConnected_ext(name);
};

void (*MarkMainLonglink_ext)(const std::string& name)
= [](const std::string& name){
    //STN_WEAK_CALL(MarkMainLonglink_ext(name));
    stn_manager->MarkMainLonglink_ext(name);
};
    
void (*MakesureLonglinkConnected_ext)(const std::string& name)
= [](const std::string& name){
    //STN_WEAK_CALL(MakeSureLongLinkConnect_ext(name));
    stn_manager->MakesureLonglinkConnected_ext(name);
};

//ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select) {
//    return stn_manager->GetConnectProfile(_taskid, _channel_select);
//}
//void AddServerBan(const std::string& _ip) {
//    stn_manager->AddServerBan(_ip);
//}
//
//void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLink::TDisconnectInternalCode _code) {
//    stn_manager->DisconnectLongLinkByTaskId(_taskid, _code);
//}
void network_export_symbols_0(){}

}
}
