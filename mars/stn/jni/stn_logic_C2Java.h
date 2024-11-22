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

#include <memory>
#include <vector>

#include "mars/comm/autobuffer.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

int C2Java_OnTaskEnd(uint32_t _taskid,
                     void* const _user_context,
                     const std::string& _user_id,
                     int _error_type,
                     int _error_code,
                     const ConnectProfile& _profile);

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

int C2Java_GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                          AutoBuffer& _identify_buffer,
                                          AutoBuffer& _buffer_hash,
                                          int32_t& _cmdid);

bool C2Java_OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                       const AutoBuffer& _response_buffer,
                                       const AutoBuffer& _identify_buffer_hash);

void C2Java_TrafficData(ssize_t _send, ssize_t _recv);

void C2Java_ReportConnectStatus(int _all_connstatus, int _longlink_connstatus);

void C2Java_RequestSync();

void C2Java_RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);

void C2Java_ReportTaskProfile(const TaskProfile& _task_profile);

#ifdef NATIVE_CALLBACK
class StnNativeCallback {
 public:
    StnNativeCallback() = default;
    virtual ~StnNativeCallback() = default;
    virtual int OnTaskEnd(uint32_t _taskid,
                          void* const _user_context,
                          const std::string& _user_id,
                          int _error_type,
                          int _error_code) {
        return -1;
    }

    virtual void OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend) {
    }

    virtual std::vector<std::string> OnNewDns(const std::string& _host) {
        return std::vector<std::string>();
    }

    virtual bool Req2Buf(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         AutoBuffer& _outbuffer,
                         AutoBuffer& _extend,
                         int& _error_code,
                         const int _channel_select,
                         const std::string& _host) {
        return false;
    }

    virtual int Buf2Resp(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         const AutoBuffer& _inbuffer,
                         const AutoBuffer& _extend,
                         int& _error_code,
                         const int _channel_select) {
        return -1;
    }
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
        return false;
    }
    virtual int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& _identify_buffer,
                                               AutoBuffer& _buffer_hash,
                                               int32_t& _cmdid) {
        return -1;
    }
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& _response_buffer,
                                            const AutoBuffer& _identify_buffer_hash) {
        return false;
    }
    virtual void TrafficData(int64_t _send, int64_t _recv) {
    }
    virtual void ReportConnectStatus(int _all_connstatus, int _longlink_connstatus) {
    }
    virtual void RequestSync() {
    }
    virtual void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    }
    virtual void ReportTaskProfile(const TaskProfile& _task_profile) {
    }
};
extern void SetStnNativeCallback(std::shared_ptr<StnNativeCallback> _cb);

#endif

}  // namespace stn
}  // namespace mars

#endif  // STN_JNI_STN_LOGIC_C2JAVA_H_
