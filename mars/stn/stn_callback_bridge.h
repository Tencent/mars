// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STN_CALLBACK_BRIDGE_H_
#define STN_CALLBACK_BRIDGE_H_

#include <string>
#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"

namespace mars {
namespace stn {

class StnCallbackBridge {
 public:
    virtual ~StnCallbackBridge();
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id);

    virtual void TrafficData(ssize_t _send, ssize_t _recv);

    virtual std::vector<std::string> OnNewDns(const std::string& host);
    virtual void OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend);
    virtual bool Req2Buf(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         const int channel_select,
                         const std::string& host);
    virtual int Buf2Resp(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         const AutoBuffer& _inbuffer,
                         const AutoBuffer& _extend,
                         int& _error_code,
                         const int _channel_select);
    virtual int OnTaskEnd(uint32_t _taskid, 
                        void* const _user_context, 
                        const std::string& _user_id,
                        int _error_type, 
                        int _error_code);

    virtual void ReportConnectStatus(int _status, int _longlink_status);
    virtual void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port);

    virtual void OnLongLinkStatusChange(int _status);
    //长连信令校验 ECHECK_NOW, ECHECK_NEXT = 1, ECHECK_NEVER = 2 
    virtual int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& _identify_buffer,
                                               AutoBuffer& _buffer_hash,
                                               int32_t& _cmdid);
    //长连信令校验回包 
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& _response_buffer,
                                            const AutoBuffer& _identify_buffer_hash);

    virtual void RequestSync();

    virtual void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);
    virtual void ReportTaskProfile(const TaskProfile& _task_profile);
    virtual void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param);
    virtual void ReportDnsProfile(const DnsProfile& _dns_profile);
};

// You must get `StnCallbackBridge` object through `GetStnCallbackBridge` firstly before invoke this function,
// then store it or release it.
void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge);
StnCallbackBridge* GetStnCallbackBridge();

}
}

#endif //STN_CALLBACK_BRIDGE_H_
