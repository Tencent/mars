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
//  stn_logic.h
//  network
//
//  Created by yanguoyue on 16/2/18.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef MARS_STN_LOGIC_H_
#define MARS_STN_LOGIC_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"

namespace mars {

namespace comm {
class ProxyInfo;
}

namespace stn {
class StnCallbackBridge;
class LongLink;
// callback interface
// mars2
/*
class Callback
{
public:
    virtual ~Callback() {}
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id) = 0;

    //流量统计
    virtual void TrafficData(ssize_t _send, ssize_t _recv) = 0;

    //底层询问上层该host对应的ip列表
    virtual std::vector<std::string> OnNewDns(const std::string& host, bool _longlink_host) = 0;
    //网络层收到push消息回调
    virtual void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body,
const AutoBuffer& _extend) = 0;
    //底层获取task要发送的数据
    virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer&
outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) = 0;
    //底层回包返回给上层解析
    virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id,  const AutoBuffer&
_inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;
    //任务执行结束
    virtual int  OnTaskEnd(uint32_t _taskid, void* const _user_context, const std::string& _user_id, int _error_type,
int _error_code, const CgiProfile& _profile) = 0;


    //上报网络连接状态
    virtual void ReportConnectStatus(int _status, int _longlink_status) = 0;
    virtual void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {}
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string&
_host, uint16_t _port) {}

    virtual void OnLongLinkStatusChange(int _status) {}
    //长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
    virtual int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer,
AutoBuffer& _buffer_hash, int32_t& _cmdid) = 0;
    //长连信令校验回包
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const
AutoBuffer& _identify_buffer_hash) = 0;


    virtual void RequestSync() = 0;

    //验证是否已登录
};
*/

void SetCallback(Callback* const callback);

//    extern void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports);
//    extern void SetShortlinkSvrAddr(const uint16_t port);

// 'host' will be ignored when 'debugip' is not empty.
extern void (*SetLonglinkSvrAddr)(const std::string& host,
                                  const std::vector<uint16_t> ports,
                                  const std::string& debugip);

// 'task.host' will be ignored when 'debugip' is not empty.
extern void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip);

// setting debug ip address for the corresponding host
extern void (*SetDebugIP)(const std::string& host, const std::string& ip);

// setting backup iplist for the corresponding host
// if debugip is not empty, iplist will be ignored.
// iplist will be used when newdns/dns ip is not available.
extern void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist);

// async function.
extern bool (*StartTask)(const Task& task);

// sync function
extern void (*StopTask)(uint32_t taskid);

// check whether task's list has the task or not.
extern bool (*HasTask)(uint32_t taskid);

// reconnect longlink and redo all task
// when you change svr ip, you must call this function.
extern void (*RedoTasks)();

// touch tasks loop. Generally, invoke it after autoauth successfully.
extern void (*TouchTasks)();

// need longlink channel
extern void (*DisableLongLink)();

// stop and clear all task
extern void (*ClearTasks)();

// the same as ClearTasks(), but also reinitialize network.
extern void (*Reset)();

extern void (*ResetAndInitEncoderVersion)(int _encoder_version);

// setting signalling's parameters.
// if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
extern void (*SetSignallingStrategy)(long period, long keeptime);

// used to keep longlink active
// keep signnaling once 'period' and last 'keeptime'
extern void (*KeepSignalling)();

extern void (*StopSignalling)();

// connect quickly if longlink is not connected.
extern void (*MakesureLonglinkConnected)();

extern bool (*LongLinkIsConnected)();

extern bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info,
                                const std::string& _test_host,
                                const std::vector<std::string>& _hardcode_ips);

std::shared_ptr<LongLink> DefaultLongLink();

// noop is used to keep longlink conected
// get noop taskid
extern uint32_t (*getNoopTaskID)();

//===----------------------------------------------------------------------===//
///
/// Support multi longlinks for mars
/// these APIs are subject to change in developing
///
//===----------------------------------------------------------------------===//
extern void (*CreateLonglink_ext)(LonglinkConfig& _config);
extern void (*DestroyLonglink_ext)(const std::string& name);
extern std::vector<std::string> (*GetAllLonglink_ext)();
extern void (*MarkMainLonglink_ext)(const std::string& name);

extern bool (*LongLinkIsConnected_ext)(const std::string& name);
extern void (*MakesureLonglinkConnected_ext)(const std::string& name);
// mars2

void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge);
StnCallbackBridge* GetStnCallbackBridge();

// callback
extern bool MakesureAuthed(const std::string& _host, const std::string& _user_id);

//流量统计
extern void TrafficData(ssize_t _send, ssize_t _recv);

//底层询问上层该host对应的ip列表
extern std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _extra_info);
//网络层收到push消息回调
extern void OnPush(const std::string& _channel_id,
                   uint32_t _cmdid,
                   uint32_t _taskid,
                   const AutoBuffer& _body,
                   const AutoBuffer& _extend);
//底层获取task要发送的数据
extern bool Req2Buf(uint32_t taskid,
                    void* const user_context,
                    const std::string& _user_id,
                    AutoBuffer& outbuffer,
                    AutoBuffer& extend,
                    int& error_code,
                    const int channel_select,
                    const std::string& host,
                    const unsigned short client_sequence_id);
//底层回包返回给上层解析
extern int Buf2Resp(uint32_t taskid,
                    void* const user_context,
                    const std::string& _user_id,
                    const AutoBuffer& inbuffer,
                    const AutoBuffer& extend,
                    int& error_code,
                    const int channel_select,
                    unsigned short& server_sequence_id);
//任务执行结束
extern int OnTaskEnd(uint32_t taskid,
                     void* const user_context,
                     const std::string& _user_id,
                     int error_type,
                     int error_code,
                     const ConnectProfile& _profile);

//上报网络连接状态
extern void ReportConnectStatus(int status, int longlink_status);

extern void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
extern void OnShortLinkNetworkError(ErrCmdType _err_type,
                                    int _err_code,
                                    const std::string& _ip,
                                    const std::string& _host,
                                    uint16_t _port);

extern void OnLongLinkStatusChange(int _status);
//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
extern int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                          AutoBuffer& identify_buffer,
                                          AutoBuffer& buffer_hash,
                                          int32_t& cmdid);
//长连信令校验回包
extern bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                       const AutoBuffer& response_buffer,
                                       const AutoBuffer& identify_buffer_hash);

extern void RequestSync();
//验证是否已登录

//底层询问上层http网络检查的域名列表
extern void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);
//底层向上层上报cgi执行结果
extern void ReportTaskProfile(const TaskProfile& _task_profile);
//底层通知上层cgi命中限制
extern void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param);
//底层上报域名dns结果
extern void ReportDnsProfile(const DnsProfile& _dns_profile);
//.生成taskid.
extern uint32_t GenTaskID();
extern unsigned short GenSequenceId();
// end callback
}  // namespace stn
}  // namespace mars

#endif /* MARS_STN_LOGIC_H_ */
