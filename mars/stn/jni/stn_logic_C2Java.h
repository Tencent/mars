// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STN_JNI_STN_LOGIC_C2JAVA_H_
#define STN_JNI_STN_LOGIC_C2JAVA_H_

#include <vector>
#include "mars/comm/autobuffer.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

int C2Java_OnTaskEnd(uint32_t _taskid, 
                    void* const _user_context, 
                    const std::string& _user_id, 
                    int _error_type, 
                    int _error_code);

void C2Java_OnPush(const std::string& _channel_id,
                   uint32_t _cmdid,
                   uint32_t _taskid,
                   const AutoBuffer& _body,
                   const AutoBuffer& _extend);

std::vector<std::string> C2Java_OnNewDns(const std::string& _host);

bool C2Java_Req2Buf(uint32_t _taskid,
                    void* const _user_context,
                    const std::string& _user_id,
                    AutoBuffer& _outbuffer,
                    AutoBuffer& _extend,
                    int& _error_code,
                    const int _channel_select,
                    const std::string& _host);

int C2Java_Buf2Resp(uint32_t _taskid,
                    void* const _user_context,
                    const std::string& _user_id,
                    const AutoBuffer& _inbuffer,
                    const AutoBuffer& _extend,
                    int& _error_code,
                    const int _channel_select);

bool C2Java_MakesureAuthed(const std::string& _host, const std::string& _user_id);

int C2Java_GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid);

bool C2Java_OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash);

void C2Java_TrafficData(ssize_t _send, ssize_t _recv);

void C2Java_ReportConnectStatus(int _all_connstatus, int _longlink_connstatus);

void C2Java_RequestSync();

void C2Java_RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);

void C2Java_ReportTaskProfile(const TaskProfile& _task_profile);
}
}

#endif //STN_JNI_STN_LOGIC_C2JAVA_H_
