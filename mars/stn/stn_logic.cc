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

#include "stn/src/net_core.h"//一定要放这里，Mac os 编译
#include "stn/src/net_source.h"
#include "stn/src/signalling_keeper.h"
#include "stn/src/proxy_test.h"

namespace mars {
namespace stn {

static Callback* sg_callback = NULL;
static const std::string kLibName = "stn";

#define STN_WEAK_CALL(func) \
    boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
    if (!stn_ptr) {\
        xwarn2(TSF"stn uncreate");\
        return;\
    }\
    stn_ptr->func

#define STN_WEAK_CALL_RETURN(func, ret) \
	boost::shared_ptr<NetCore> stn_ptr = NetCore::Singleton::Instance_Weak().lock();\
    if (stn_ptr) \
    {\
    	ret = stn_ptr->func;\
    }

static void onCreate() {
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

    xinfo2(TSF"stn oncreate");
    ActiveLogic::Singleton::Instance();
    NetCore::Singleton::Instance();

}

static void onDestroy() {
    xinfo2(TSF"stn onDestroy");

    NetCore::Singleton::Release();
    SINGLETON_RELEASE_ALL();
    
    // others use activelogic may crash after activelogic release. eg: LongLinkConnectMonitor
    // ActiveLogic::Singleton::Release();
}

static void onSingalCrash(int _sig) {
    appender_close();
}

static void onExceptionCrash() {
    appender_close();
}

static void onNetworkChange() {

    STN_WEAK_CALL(OnNetworkChange());
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    
    if (NULL == _tag || strnlen(_tag, 1024) == 0) {
        xassert2(false);
        return;
    }
    
    if (NULL != XLOGGER_TAG && 0 == strcmp(_tag, XLOGGER_TAG)) {
        TrafficData(_send, _recv);
    }
}


static void __initbind_baseprjevent() {

#ifdef ANDROID
	mars::baseevent::addLoadModule(kLibName);
#endif
    GetSignalOnCreate().connect(&onCreate);
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
    
void SetCallback(Callback* const callback) {
	sg_callback = callback;
}

void (*StartTask)(const Task& _task)
= [](const Task& _task) {
    STN_WEAK_CALL(StartTask(_task));
};

void (*StopTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
    STN_WEAK_CALL(StopTask(_taskid));
};

bool (*HasTask)(uint32_t _taskid)
= [](uint32_t _taskid) {
	bool has_task = false;
	STN_WEAK_CALL_RETURN(HasTask(_taskid), has_task);
	return has_task;
};

void (*RedoTasks)()
= []() {
   STN_WEAK_CALL(RedoTasks());
};

void (*ClearTasks)()
= []() {
   STN_WEAK_CALL(ClearTasks());
};

void (*Reset)()
= []() {
	xinfo2(TSF "stn reset");
	NetCore::Singleton::Release();
	NetCore::Singleton::Instance();
};

void (*MakesureLonglinkConnected)()
= []() {
    xinfo2(TSF "make sure longlink connect");
   STN_WEAK_CALL(MakeSureLongLinkConnect());
};

bool (*LongLinkIsConnected)()
= []() {
    bool connected = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected(), connected);
    return connected;
};
    
bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips)
= [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips){
    
    return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
};

//void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports)
// {
//	SetLonglinkSvrAddr(host, ports, "");
//};


void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip)
= [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
	std::vector<std::string> hosts;
	if (!host.empty()) {
		hosts.push_back(host);
	}
	NetSource::SetLongLink(hosts, ports, debugip);
};

//void SetShortlinkSvrAddr(const uint16_t port)
//{
//	NetSource::SetShortlink(port, "");
//};
    
void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip)
= [](const uint16_t port, const std::string& debugip) {
	NetSource::SetShortlink(port, debugip);
};

void (*SetDebugIP)(const std::string& host, const std::string& ip)
= [](const std::string& host, const std::string& ip) {
	NetSource::SetDebugIP(host, ip);
};
    
void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist)
= [](const std::string& host, const std::vector<std::string>& iplist) {
	NetSource::SetBackupIPs(host, iplist);
};

void (*SetSignallingStrategy)(long _period, long _keepTime)
= [](long _period, long _keepTime) {
    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
};

void (*KeepSignalling)()
= []() {
#ifdef USE_LONG_LINK
    STN_WEAK_CALL(KeepSignal());
#endif
};

void (*StopSignalling)()
= []() {
#ifdef USE_LONG_LINK
    STN_WEAK_CALL(StopSignal());
#endif
};

uint32_t (*getNoopTaskID)()
= []() {
	return Task::kNoopTaskID;
};

void network_export_symbols_0(){}

#ifndef ANDROID
	//callback functions
bool (*MakesureAuthed)()
= []() {
	xassert2(sg_callback != NULL);
	return sg_callback->MakesureAuthed();
};

// 流量统计 
void (*TrafficData)(ssize_t _send, ssize_t _recv)
= [](ssize_t _send, ssize_t _recv) {
    xassert2(sg_callback != NULL);
    return sg_callback->TrafficData(_send, _recv);
};

//底层询问上层该host对应的ip列表 
std::vector<std::string> (*OnNewDns)(const std::string& host)
= [](const std::string& host) {
	xassert2(sg_callback != NULL);
	return sg_callback->OnNewDns(host);
};

//网络层收到push消息回调 
void (*OnPush)(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend)
= [](uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
	xassert2(sg_callback != NULL);
	sg_callback->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
};
//底层获取task要发送的数据 
bool (*Req2Buf)(uint32_t taskid,  void* const user_context, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select)
= [](uint32_t taskid,  void* const user_context, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select) {
	xassert2(sg_callback != NULL);
	return sg_callback->Req2Buf(taskid, user_context, outbuffer, extend, error_code, channel_select);
};
//底层回包返回给上层解析 
int (*Buf2Resp)(uint32_t taskid, void* const user_context, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select)
= [](uint32_t taskid, void* const user_context, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
	xassert2(sg_callback != NULL);
	return sg_callback->Buf2Resp(taskid, user_context, inbuffer, extend, error_code, channel_select);
};
//任务执行结束 
int  (*OnTaskEnd)(uint32_t taskid, void* const user_context, int error_type, int error_code)
= [](uint32_t taskid, void* const user_context, int error_type, int error_code) {
	xassert2(sg_callback != NULL);
	return sg_callback->OnTaskEnd(taskid, user_context, error_type, error_code);
 };

//上报网络连接状态 
void (*ReportConnectStatus)(int status, int longlink_status)
= [](int status, int longlink_status) {
	xassert2(sg_callback != NULL);
	sg_callback->ReportConnectStatus(status, longlink_status);
};
    
void (*OnLongLinkNetworkError)(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port)
= [](ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {

};
    
void (*OnShortLinkNetworkError)(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port)
= [](ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {

};

//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int  (*GetLonglinkIdentifyCheckBuffer)(AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid)
= [](AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
	xassert2(sg_callback != NULL);
	return sg_callback->GetLonglinkIdentifyCheckBuffer(identify_buffer, buffer_hash, cmdid);
};
//长连信令校验回包
bool (*OnLonglinkIdentifyResponse)(const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash)
= [](const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
	xassert2(sg_callback != NULL);
	return sg_callback->OnLonglinkIdentifyResponse(response_buffer, identify_buffer_hash);
};

void (*RequestSync)() 
= []() {
	xassert2(sg_callback != NULL);
	sg_callback->RequestSync();
};

void (*RequestNetCheckShortLinkHosts)(std::vector<std::string>& _hostlist)
= [](std::vector<std::string>& _hostlist) {
};

void (*ReportTaskProfile)(const TaskProfile& _task_profile)
= [](const TaskProfile& _task_profile) {
};

void (*ReportTaskLimited)(int _check_type, const Task& _task, unsigned int& _param)
= [](int _check_type, const Task& _task, unsigned int& _param) {
};

void (*ReportDnsProfile)(const DnsProfile& _dns_profile)
= [](const DnsProfile& _dns_profile) {
};
#endif

}
}
