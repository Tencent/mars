// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#include "stn_callback_bridge.h"

#include "mars/boost/signals2.hpp"
#include "mars/comm/xlogger/xlogger.h"
#include "stn_logic.h"
#ifdef ANDROID
#include "mars/stn/jni/stn_logic_C2Java.h"
#endif

namespace mars {
namespace stn {

static Callback* sg_callback = NULL;
static StnCallbackBridge* sg_callback_bridge = new StnCallbackBridge();
boost::signals2::signal<void (ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port)> SignalOnLongLinkNetworkError;
boost::signals2::signal<void (ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port)> SignalOnShortLinkNetworkError;


void SetCallback(Callback* const callback) {
    sg_callback = callback;
}

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    sg_callback_bridge = _callback_bridge;
}

StnCallbackBridge* GetStnCallbackBridge() {
    return sg_callback_bridge;
}

StnCallbackBridge::~StnCallbackBridge() {
}

bool StnCallbackBridge::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->MakesureAuthed(_host, _user_id);
#else
    return C2Java_MakesureAuthed(_host, _user_id);
#endif
}

void StnCallbackBridge::TrafficData(ssize_t _send, ssize_t _recv) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->TrafficData(_send, _recv);
#else
    C2Java_TrafficData(_send, _recv);
#endif
}

std::vector<std::string> StnCallbackBridge::OnNewDns(const std::string& host) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->OnNewDns(host);
#else
    return C2Java_OnNewDns(host);
#endif
}

void StnCallbackBridge::OnPush(const std::string& _channel_id,
                               uint32_t _cmdid,
                               uint32_t _taskid,
                               const AutoBuffer& _body,
                               const AutoBuffer& _extend) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
#else
    C2Java_OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
#endif
}

bool StnCallbackBridge::Req2Buf(uint32_t _taskid,
                                void* const _user_context,
                                const std::string& _user_id,
                                AutoBuffer& outbuffer,
                                AutoBuffer& extend,
                                int& error_code,
                                const int channel_select,
                                const std::string& host) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->Req2Buf(_taskid, _user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
#else
    return C2Java_Req2Buf(_taskid, _user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
#endif
}

int StnCallbackBridge::Buf2Resp(uint32_t _taskid,
                                void* const _user_context,
                                const std::string& _user_id,
                                const AutoBuffer& _inbuffer,
                                const AutoBuffer& _extend,
                                int& _error_code,
                                const int _channel_select) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->Buf2Resp(_taskid, _user_context, _user_id, _inbuffer, _extend, _error_code, _channel_select);
#else
    return C2Java_Buf2Resp(_taskid, _user_context, _user_id, _inbuffer, _extend, _error_code, _channel_select);
#endif
}

int StnCallbackBridge::OnTaskEnd(uint32_t _taskid, 
                                void* const _user_context, 
                                const std::string& _user_id,
                                int _error_type, 
                                int _error_code) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->OnTaskEnd(_taskid, _user_context, _user_id, _error_type, _error_code);
#else
    return C2Java_OnTaskEnd(_taskid, _user_context, _user_id, _error_type, _error_code);
#endif
}

void StnCallbackBridge::ReportConnectStatus(int _status, int _longlink_status) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->ReportConnectStatus(_status, _longlink_status);
#else
    C2Java_ReportConnectStatus(_status, _longlink_status);
#endif
}

void StnCallbackBridge::OnLongLinkNetworkError(ErrCmdType _err_type,
                                               int _err_code,
                                               const std::string& _ip,
                                               uint16_t _port) {
    SignalOnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
#endif
}

void StnCallbackBridge::OnShortLinkNetworkError(ErrCmdType _err_type,
                                                int _err_code,
                                                const std::string& _ip,
                                                const std::string& _host,
                                                uint16_t _port) {
    SignalOnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
#endif
}

void StnCallbackBridge::OnLongLinkStatusChange(int _status) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->OnLongLinkStatusChange(_status);
#endif
}

int StnCallbackBridge::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                                      AutoBuffer& _identify_buffer,
                                                      AutoBuffer& _buffer_hash,
                                                      int32_t& _cmdid) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->GetLonglinkIdentifyCheckBuffer(_channel_id, _identify_buffer, _buffer_hash, _cmdid);
#else
    return C2Java_GetLonglinkIdentifyCheckBuffer(_channel_id, _identify_buffer, _buffer_hash, _cmdid);
#endif
}

bool StnCallbackBridge::OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                                   const AutoBuffer& _response_buffer,
                                                   const AutoBuffer& _identify_buffer_hash) {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    return sg_callback->OnLonglinkIdentifyResponse(_channel_id, _response_buffer, _identify_buffer_hash);
#else
    return C2Java_OnLonglinkIdentifyResponse(_channel_id, _response_buffer, _identify_buffer_hash);
#endif
}

void StnCallbackBridge::RequestSync() {
#ifndef ANDROID
    xassert2(sg_callback != NULL);
    sg_callback->RequestSync();
#else
    C2Java_RequestSync();
#endif
}

void StnCallbackBridge::RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
#ifndef ANDROID

#else
    C2Java_RequestNetCheckShortLinkHosts(_hostlist);
#endif
}

void StnCallbackBridge::ReportTaskProfile(const TaskProfile& _task_profile) {
#ifndef ANDROID

#else
    C2Java_ReportTaskProfile(_task_profile);
#endif
}

void StnCallbackBridge::ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
}

void StnCallbackBridge::ReportDnsProfile(const DnsProfile& _dns_profile) {
}

//callback functions
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->MakesureAuthed(_host, _user_id);
};

// 流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->TrafficData(_send, _recv);
};

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& host) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->OnNewDns(host);
};

//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
};
//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid,  void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
};
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
};
//任务执行结束
int  OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code);
};

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->ReportConnectStatus(status, longlink_status);
};

void OnLongLinkStatusChange(int _status) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->OnLongLinkStatusChange(_status);
};
void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    xassert2(sg_callback_bridge != NULL);
    SignalOnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
    sg_callback_bridge->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
};

void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    xassert2(sg_callback_bridge != NULL);
    SignalOnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
    sg_callback_bridge->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
};
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
};
//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    xassert2(sg_callback_bridge != NULL);
    return sg_callback_bridge->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
};

void RequestSync() {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->RequestSync();
};

void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->RequestNetCheckShortLinkHosts(_hostlist);
};

void ReportTaskProfile(const TaskProfile& _task_profile) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->ReportTaskProfile(_task_profile);
};

void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->ReportTaskLimited(_check_type, _task, _param);
};

void ReportDnsProfile(const DnsProfile& _dns_profile) {
    xassert2(sg_callback_bridge != NULL);
    sg_callback_bridge->ReportDnsProfile(_dns_profile);
};

}
}
