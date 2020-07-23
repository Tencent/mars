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
*  CGITask.h
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#ifndef _CGI_TASK_H_
#define _CGI_TASK_H_
#include <map>
#include <string>

#include "mars/comm/autobuffer.h"

enum ChannelType 
{
	ChannelType_ShortConn = 1,
	ChannelType_LongConn = 2,
	ChannelType_All = 3
} ;
class CGITask
{
public:
	virtual ~CGITask() {};
	// 底层获取task要发送的数据
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select, const std::string& _host) = 0;
	// 底层回包返回给上层解析 
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;

	uint32_t taskid_;
	ChannelType channel_select_;
	uint32_t cmdid_;
	std::string cgi_;
	std::string host_;
};

#endif