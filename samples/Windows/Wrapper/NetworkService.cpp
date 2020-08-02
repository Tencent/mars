// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
*  NetworkService.cpp
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#include "NetworkService.h"
#include "mars/comm/windows/projdef.h"
#include "mars/boost/bind.hpp"
#include "mars/baseevent/base_logic.h"
#include "mars/stn/stn_logic.h"
#include "PublicComponentV2/stnproto_logic.h"
#include "PublicComponentV2/stn_callback.h"
#include "PublicComponentV2/app_callback.h"

#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/log/appender.h"

#include "proto/generate/main.pb.h"
#include <windows.h>
using namespace std;

NetworkService& NetworkService::Instance()
{
	static NetworkService instance_;
	return instance_;
}

NetworkService::NetworkService()
{
	__Openlog();
	__Init();
}

NetworkService::~NetworkService()
{
	__Cleanup();
	__Closelog();
}

void NetworkService::setClientVersion(uint32_t _client_version)
{
	mars::stn::SetClientVersion(_client_version);
}

void NetworkService::setShortLinkDebugIP(const std::string& _ip, unsigned short _port)
{
	mars::stn::SetShortlinkSvrAddr(_port, _ip);
}

void NetworkService::setShortLinkPort(unsigned short _port)
{
	mars::stn::SetShortlinkSvrAddr(_port, "");
}

void NetworkService::setLongLinkAddress(const std::string& _ip, unsigned short _port, const std::string& _debug_ip)
{
	vector<uint16_t> ports;
	ports.push_back(_port);
	mars::stn::SetLonglinkSvrAddr(_ip, ports, _debug_ip);
}

void NetworkService::start()
{
	mars::baseevent::OnForeground(true);
	mars::stn::MakesureLonglinkConnected();
}

void NetworkService::__Openlog()
{
#if _DEBUG
	xlogger_SetLevel(kLevelDebug);
	appender_set_console_log(true);
	extern std::function<void(char* _log)> g_console_log_fun;
	g_console_log_fun = [](char* _log) {
		::OutputDebugStringA(_log);
	};
#else
	xlogger_SetLevel(kLevelInfo);
	appender_set_console_log(false);
#endif

	XLogConfig config;
	config.logdir_ = "Log"; //use your log path
	config.nameprefix_ = "Sample";
	config.pub_key_ = ""; //use your pubkey for log encrypt
	config.cachedir_ = ""; //use your cache path
	appender_open(config);
}

void NetworkService::__Closelog()
{
	appender_close();
}

void NetworkService::__Init()
{
	mars::stn::SetCallback(mars::stn::StnCallBack::Instance());
	mars::app::SetCallback(mars::app::AppCallBack::Instance());
	mars::baseevent::OnCreate();
}

void NetworkService::__Cleanup()
{
	mars::baseevent::OnDestroy();
}

int NetworkService::startTask(CGITask* task)
{
	mars::stn::Task ctask;
	ctask.cmdid = task->cmdid_;
	ctask.channel_select = task->channel_select_;
	ctask.shortlink_host_list.push_back(task->host_);
	ctask.cgi = task->cgi_;
	ctask.user_context = (void*)task;
	mars::stn::StartTask(ctask);
	map_task_[ctask.taskid] = task;
	return ctask.taskid;
}

bool NetworkService::Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select, const std::string& _host)
{
	auto it = map_task_.find(_taskid);
	if (it == map_task_.end())return false;
	return it->second->Req2Buf(_taskid, _user_context, _user_id, _outbuffer, _extend, _error_code, _channel_select, _host);
}

int NetworkService::Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	auto it = map_task_.find(_taskid);
	if (it == map_task_.end())return mars::stn::kTaskFailHandleDefault;
	return it->second->Buf2Resp(_taskid, _user_context, _user_id, _inbuffer, _extend, _error_code, _channel_select);
}

int NetworkService::OnTaskEnd(uint32_t _taskid, void* const _user_context, const std::string& _user_id, int _error_type, int _error_code)
{
	auto it = map_task_.find(_taskid);
	if (it != map_task_.end())
	{
		delete it->second;
		map_task_.erase(it);
	}
	return 0;
}

void NetworkService::OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend)
{
	auto it = map_push_observer_.find(_cmdid);
	if (it != map_push_observer_.end() && it->second)
		it->second->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

void NetworkService::setPushObserver(uint32_t _cmdid, PushObserver* _observer)
{
	map_push_observer_[_cmdid] = _observer;
}
