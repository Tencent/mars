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

#include "mars/comm/thread/atomic_oper.h"
#include "mars/stn/stn_callback_bridge.h"
#include "mars/stn/stn_manager.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";

//mars2
/*
#define STN_WEAK_CALL(func) \
    boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
    if (!stn_ptr) {\
        xwarn2(TSF"stn uncreate");\
        return;\
    }\
    stn_ptr->func
    
#define STN_RETURN_WEAK_CALL(func) \
    boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
    if (!stn_ptr) {\
        xwarn2(TSF"stn uncreate");\
        return false;\
    }\
    stn_ptr->func;\
    return true

#define STN_WEAK_CALL_RETURN(func, ret) \
	boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
    if (stn_ptr) \
    {\
    	ret = stn_ptr->func;\
    }
*/

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    /* mars2
    xinfo2(TSF"stn oninit: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
    */

    //xinfo2(TSF "mars2 onInitConfigBeforeOnCreate _packer_encoder_version:%_", _packer_encoder_version);
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnInitConfigBeforeOnCreate(_packer_encoder_version);
    }
    //xinfo2(TSF"mars2 onInitConfigBeforeOnCreate finish.");
}

static void onCreate() {
    /* mars2
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif
    xinfo2(TSF"stn oncreate");
    ActiveLogic::Instance();
    NetCore::Singleton::Instance();
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnCreate();
    }
}

static void onDestroy() {
    /* mars2
    xinfo2(TSF"stn onDestroy");

    NetCore::Singleton::Release();
    SINGLETON_RELEASE_ALL();

    // others use activelogic may crash after activelogic release. eg: LongLinkConnectMonitor
    // ActiveLogic::Singleton::Release();
    */
    xinfo_function();
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnDestroy();
    }
}

static void onSingalCrash(int _sig) {
    /* mars2
    mars::xlog::appender_close();
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnSingalCrash(_sig);
    }
}

static void onExceptionCrash() {
    /* mars2
    mars::xlog::appender_close();
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    if (stn_manager) {
        stn_manager->OnExceptionCrash();
    }
}

static void onNetworkChange(void (*pre_change)()) {
    /* mars2
    pre_change();
    STN_WEAK_CALL(OnNetworkChange());
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    //may be have no init
    //xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnNetworkChange(pre_change);
    }
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    /* mars2
    if (NULL == _tag || strnlen(_tag, 1024) == 0) {
        xassert2(false);
        return;
    }
    
    if (0 == strcmp(_tag, XLOGGER_TAG)) {
        TrafficData(_send, _recv);
    }
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    //may be have no init
    //xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnNetworkDataChange(_tag, _send, _recv);
    }
}

#ifdef ANDROID
//must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    /* mars2
    Alarm::onAlarmImpl(_id);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    if (stn_manager) {
        stn_manager->OnAlarm(_id);
    }
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
    GetSignalOnDestroy().connect(1, &onDestroy);   //low priority signal func
    GetSignalOnSingalCrash().connect(&onSingalCrash);
    GetSignalOnExceptionCrash().connect(&onExceptionCrash);
    GetSignalOnNetworkChange().connect(5, boost::bind(&onNetworkChange, &mars::comm::OnPlatformNetworkChange));    //define group 5

    
#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif
    
    GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);


bool (*StartTask)(const Task& _task)
= [](const Task& _task) {
    /* mars2
    STN_RETURN_WEAK_CALL(StartTask(_task));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->StartTask(_task);
    }
    return false;
};

void (*StopTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
    /* mars2
    STN_WEAK_CALL(StopTask(_taskid));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->StopTask(_taskid);
    }
};

bool (*HasTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
    /* mars2
	bool has_task = false;
	STN_WEAK_CALL_RETURN(HasTask(_taskid), has_task);
	return has_task;
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->HasTask(_taskid);
    }
    return false;
};

void (*DisableLongLink)()
= []() {
    /* mars2
    NetCore::need_use_longlink_ = false;
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->DisableLongLink();
    }
};

void (*RedoTasks)()
= []() {
   /* mars2
   STN_WEAK_CALL(RedoTasks());
   */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->RedoTasks();
    }
};

void (*TouchTasks)()
= []() {
    /* mars2
   STN_WEAK_CALL(TouchTasks());
   */
   StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
   xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
   if (stn_manager) {
       stn_manager->TouchTasks();
   }
};

void (*ClearTasks)()
= []() {
    /* mars2
   STN_WEAK_CALL(ClearTasks());
   */
   StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
   xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
   if (stn_manager) {
       stn_manager->ClearTasks();
   }
};

void (*Reset)()
= []() {
    /* mars2
	xinfo2(TSF "stn reset");
	NetCore::Singleton::Release();
	NetCore::Singleton::Instance();
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->Reset();
    }
};

void (*ResetAndInitEncoderVersion)(int _packer_encoder_version)
= [](int _packer_encoder_version) {
    /* mars2
	xinfo2(TSF "stn reset, encoder version: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
	NetCore::Singleton::Release();
	NetCore::Singleton::Instance();
    */
    xinfo2(TSF "mars2 Reset stn_logic ResetAndInitEncoderVersion");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->ResetAndInitEncoderVersion(_packer_encoder_version);
    }
};

void (*MakesureLonglinkConnected)()
= []() {
    /* mars2
    xinfo2(TSF "make sure longlink connect");
   STN_WEAK_CALL(MakeSureLongLinkConnect());
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->MakesureLonglinkConnected();
    }
};

bool (*LongLinkIsConnected)()
= []() {
    /* mars2
    bool connected = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected(), connected);
    return connected;
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->LongLinkIsConnected();
    }
    return false;
};
    
bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips)
= [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips){
    /* mars2
    return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    }
    return false;
};

//void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports)
// {
//	SetLonglinkSvrAddr(host, ports, "");
//};


void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip)
= [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
    /* mars2
	std::vector<std::string> hosts;
	if (!host.empty()) {
		hosts.push_back(host);
	}
	NetSource::SetLongLink(hosts, ports, debugip);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetLonglinkSvrAddr(host, ports, debugip);
    }
};

//void SetShortlinkSvrAddr(const uint16_t port)
//{
//	NetSource::SetShortlink(port, "");
//};
    
void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip)
= [](const uint16_t port, const std::string& debugip) {
    /* mars2
	NetSource::SetShortlink(port, debugip);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetShortlinkSvrAddr(port, debugip);
    }
};

void (*SetDebugIP)(const std::string& host, const std::string& ip)
= [](const std::string& host, const std::string& ip) {
    /* mars2
	NetSource::SetDebugIP(host, ip);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetDebugIP(host, ip);
    }
};
    
void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist)
= [](const std::string& host, const std::vector<std::string>& iplist) {
    /* mars2
	NetSource::SetBackupIPs(host, iplist);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetBackupIPs(host, iplist);
    }
};

void (*SetSignallingStrategy)(long _period, long _keepTime)
= [](long _period, long _keepTime) {
    /* mars2
    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetSignallingStrategy(_period, _keepTime);
    }
};

void (*KeepSignalling)()
= []() {
#ifdef USE_LONG_LINK
    /* mars2
    STN_WEAK_CALL(KeepSignal());
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->KeepSignalling();
    }
#endif
};

void (*StopSignalling)()
= []() {
#ifdef USE_LONG_LINK
    /* mars2
    STN_WEAK_CALL(StopSignal());
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->StopSignalling();
    }
#endif
};

uint32_t (*getNoopTaskID)()
= []() {
    /* mars2
	return Task::kNoopTaskID;
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->getNoopTaskID();
    }
    return Task::kNoopTaskID;
};

void (*CreateLonglink_ext)(LonglinkConfig& _config)
= [](LonglinkConfig & _config){
    /* mars2
    STN_WEAK_CALL(CreateLongLink(_config));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->CreateLonglink_ext(_config);
    }
};
    
void (*DestroyLonglink_ext)(const std::string& name)
= [](const std::string& name){
    /* mars2
    STN_WEAK_CALL(DestroyLongLink(name));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->DestroyLonglink_ext(name);
    }
};

bool (*LongLinkIsConnected_ext)(const std::string& name)
= [](const std::string& name){
    /* mars2
    bool res = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected_ext(name),res);
    return res;
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->LongLinkIsConnected_ext(name);
    }
    return false;
};

void (*MarkMainLonglink_ext)(const std::string& name)
= [](const std::string& name){
    /* mars2
    STN_WEAK_CALL(MarkMainLonglink_ext(name));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->MarkMainLonglink_ext(name);
    }
};
    
void (*MakesureLonglinkConnected_ext)(const std::string& name)
= [](const std::string& name){
    /* mars2
    STN_WEAK_CALL(MakeSureLongLinkConnect_ext(name));
    */
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->MakesureLonglinkConnected_ext(name);
    }
};

// callback
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->MakesureAuthed(_host, _user_id);
    }
    return false;
}

//流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->TrafficData(_send, _recv);
    }
}

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->OnNewDns(_host, _longlink_host);
    }
    return std::vector<std::string>();
}
//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    xdebug2(TSF"mars2 OnPush");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
    }
}
//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    xdebug2(TSF"mars2 Req2Buf");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
    }
    return false;
}
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    xdebug2(TSF"mars2 Buf2Resp");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
    }
    return 0;
}
//任务执行结束
int OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    xdebug2(TSF"mars2 OnTaskEnd");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
    }
    return 0;
}

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    xverbose_function(TSF"mars2");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->ReportConnectStatus(status, longlink_status);
    }
}

void ReportConnectNetType(ConnNetType conn_type) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->ReportConnectNetType(conn_type);
    }
}

void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
    }
}

void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
    }
}

void OnLongLinkStatusChange(int _status) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->OnLongLinkStatusChange(_status);
    }
}
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
    }
    return 0;
}
//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
    }
    return false;
}

void RequestSync() {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->RequestSync();
    }
}
//验证是否已登录

//底层询问上层http网络检查的域名列表
void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->RequestNetCheckShortLinkHosts(_hostlist);
    }
}
//底层向上层上报cgi执行结果
void ReportTaskProfile(const TaskProfile& _task_profile) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->ReportTaskProfile(_task_profile);
    }
}
//底层通知上层cgi命中限制
void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->ReportTaskLimited(_check_type, _task, _param);
    }
}
//底层上报域名dns结果
void ReportDnsProfile(const DnsProfile& _dns_profile) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager && stn_manager->ReportDnsProfileFunc) {
        stn_manager->ReportDnsProfileFunc(_dns_profile);
    }
}

//.生成taskid.
uint32_t GenTaskID() {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->GenTaskID();
    }
    return 0;
}

void SetCallback(Callback* const callback) {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetCallback(callback);
    }
}

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    xdebug2(TSF"mars2 SetStnCallbackBridge");
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        stn_manager->SetStnCallbackBridge(_callback_bridge);
    }
}

StnCallbackBridge* GetStnCallbackBridge() {
    StnManager* stn_manager = Context::CreateContext("default")->GetManager<StnManager>();
    xassert2(NULL != stn_manager, "mars2 stn_manager is empty.");
    if (stn_manager) {
        return stn_manager->GetStnCallbackBridge();
    }
    return nullptr;
}
void network_export_symbols_0(){}

}
}
