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

using namespace mars::comm;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";


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

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    xinfo2(TSF"stn oninit: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
}

static void onCreate() {
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif
    xinfo2(TSF"stn oncreate");
    ActiveLogic::Instance();
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
    mars::xlog::appender_close();
}

static void onExceptionCrash() {
    mars::xlog::appender_close();
}

static void onNetworkChange() {

    STN_WEAK_CALL(OnNetworkChange());
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    
    if (NULL == _tag || strnlen(_tag, 1024) == 0) {
        xassert2(false);
        return;
    }
    
    if (0 == strcmp(_tag, XLOGGER_TAG)) {
        TrafficData(_send, _recv);
    }
}

#ifdef ANDROID
//must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    Alarm::onAlarmImpl(_id);
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


bool (*StartTask)(const Task& _task)
= [](const Task& _task) {
    STN_RETURN_WEAK_CALL(StartTask(_task));
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

void (*DisableLongLink)()
= []() {
    NetCore::need_use_longlink_ = false;
};

void (*RedoTasks)()
= []() {
   STN_WEAK_CALL(RedoTasks());
};

void (*TouchTasks)()
= []() {
   STN_WEAK_CALL(TouchTasks());
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

void (*ResetAndInitEncoderVersion)(int _packer_encoder_version)
= [](int _packer_encoder_version) {
	xinfo2(TSF "stn reset, encoder version: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
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

void (*CreateLonglink_ext)(LonglinkConfig& _config)
= [](LonglinkConfig & _config){
    STN_WEAK_CALL(CreateLongLink(_config));
};
    
void (*DestroyLonglink_ext)(const std::string& name)
= [](const std::string& name){
    STN_WEAK_CALL(DestroyLongLink(name));
};

bool (*LongLinkIsConnected_ext)(const std::string& name)
= [](const std::string& name){
    bool res = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected_ext(name),res);
    return res;
};

void (*MarkMainLonglink_ext)(const std::string& name)
= [](const std::string& name){
    STN_WEAK_CALL(MarkMainLonglink_ext(name));
};
    
void (*MakesureLonglinkConnected_ext)(const std::string& name)
= [](const std::string& name){
    STN_WEAK_CALL(MakeSureLongLinkConnect_ext(name));
};

void network_export_symbols_0(){}

}
}
