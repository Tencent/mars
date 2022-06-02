//
// Created by Cpan on 2022/5/27.
//

#ifndef MMNET_STN_LOGIC_BRIDGE_H
#define MMNET_STN_LOGIC_BRIDGE_H

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

#include "mars/comm/autobuffer.h"
#include "mars/comm/comm_data.h"
//#include "mars/comm/xlogger/xlogger.h"
#include "stn_manager.h"

namespace mars {
namespace stn {

    static StnManager* stn_manager_ = NULL;

    extern void SetCallback(Callback* const callback);
    extern void SetStnCallbackBridge(StnCallbackBridge* const _callback_bridge);
    extern StnCallbackBridge* GetStnCallbackBridge();

    // #################### stn.h ####################
    extern bool MakesureAuthed(const std::string& _host, const std::string& _user_id);

    //流量统计
    extern void TrafficData(ssize_t _send, ssize_t _recv);

    //底层询问上层该host对应的ip列表
    extern std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host);
    //网络层收到push消息回调
    extern void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend);
    //底层获取task要发送的数据
    extern bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host);
    //底层回包返回给上层解析
    extern int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select);
    //任务执行结束
    extern int  OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile);

    //上报网络连接状态
    extern void ReportConnectStatus(int status, int longlink_status);
    extern void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);
    extern void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port);

    extern void OnLongLinkStatusChange(int _status);
    //长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
    extern int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid);
    //长连信令校验回包
    extern bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash);

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

    // #################### stn.h ####################

// **
    // #################### stn_logic.h ####################
    // 'host' will be ignored when 'debugip' is not empty.
    extern void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip);

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

    //need longlink channel
    extern void (*DisableLongLink)();

    // stop and clear all task
    extern void (*ClearTasks)();

    // the same as ClearTasks(), but also reinitialize network.
    extern void (*Reset)();

    extern void (*ResetAndInitEncoderVersion)(int _encoder_version);

    //setting signalling's parameters.
    //if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    extern void (*SetSignallingStrategy)(long period, long keeptime);

    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    extern void (*KeepSignalling)();

    extern void (*StopSignalling)();

    // connect quickly if longlink is not connected.
    extern void (*MakesureLonglinkConnected)();

    extern bool (*LongLinkIsConnected)();

    extern bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips);

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
    //std::vector<std::string> (*GetAllLonglink_ext)();
    extern void (*MarkMainLonglink_ext)(const std::string& name);

    extern bool (*LongLinkIsConnected_ext)(const std::string& name);
    extern void (*MakesureLonglinkConnected_ext)(const std::string& name);

// #################### stn_logic.h ####################
// */

/**

// #################### stn_logic.h ####################
// 'host' will be ignored when 'debugip' is not empty.
void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) = [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetLonglinkSvrAddr(host, ports, debugip);
};

// 'task.host' will be ignored when 'debugip' is not empty.
void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip)  = [](const uint16_t port, const std::string& debugip) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetShortlinkSvrAddr(port, debugip);
};

// setting debug ip address for the corresponding host
void (*SetDebugIP)(const std::string& host, const std::string& ip) = [](const std::string& host, const std::string& ip) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetDebugIP(host, ip);
};

// setting backup iplist for the corresponding host
// if debugip is not empty, iplist will be ignored.
// iplist will be used when newdns/dns ip is not available.
void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist) = [](const std::string& host, const std::vector<std::string>& iplist) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetBackupIPs(host, iplist);
};


// async function.
bool (*StartTask)(const Task& task) = [](const Task& _task) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->StartTask(_task);
};

// sync function
void (*StopTask)(uint32_t taskid) = [](uint32_t _taskid) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->StopTask(_taskid);
};


// check whether task's list has the task or not.
bool (*HasTask)(uint32_t taskid) = [](uint32_t _taskid) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->HasTask(_taskid);
};

// reconnect longlink and redo all task
// when you change svr ip, you must call this function.
void (*RedoTasks)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->RedoTasks();
};

// touch tasks loop. Generally, invoke it after autoauth successfully.
void (*TouchTasks)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->TouchTasks();
};

//need longlink channel
void (*DisableLongLink)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->DisableLongLink();
};

// stop and clear all task
void (*ClearTasks)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ClearTasks();
};

// the same as ClearTasks(), but also reinitialize network.
void (*Reset)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->Reset();
};

void (*ResetAndInitEncoderVersion)(int _encoder_version) = [](int _encoder_version) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ResetAndInitEncoderVersion(_encoder_version);
};

//setting signalling's parameters.
//if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
void (*SetSignallingStrategy)(long period, long keeptime) = [](long _period, long _keepTime) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetSignallingStrategy(_period,_keepTime);
};


// used to keep longlink active
// keep signnaling once 'period' and last 'keeptime'
void (*KeepSignalling)() = []() {
#ifdef USE_LONG_LINK
    xassert2(stn_manager_ != NULL);
    stn_manager_->KeepSignalling();
#endif
};

void (*StopSignalling)() = []() {
#ifdef USE_LONG_LINK
    xassert2(stn_manager_ != NULL);
    stn_manager_->StopSignalling();
#endif
};

// connect quickly if longlink is not connected.
void (*MakesureLonglinkConnected)() = []() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->MakesureLonglinkConnected();
};

bool (*LongLinkIsConnected)() = []() {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->LongLinkIsConnected();
};

bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) = [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
};


// noop is used to keep longlink conected
// get noop taskid
uint32_t (*getNoopTaskID)() = []() {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->getNoopTaskID();
};

//===----------------------------------------------------------------------===//
///
/// Support multi longlinks for mars
/// these APIs are subject to change in developing
///
//===----------------------------------------------------------------------===//
void (*CreateLonglink_ext)(LonglinkConfig& _config)  = [](LonglinkConfig& _config) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->CreateLonglink_ext(_config);
};

void (*DestroyLonglink_ext)(const std::string& name) = [](const std::string& name) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->DestroyLonglink_ext(name);
};

//std::vector<std::string> (*GetAllLonglink_ext)();
void (*MarkMainLonglink_ext)(const std::string& name) = [](const std::string& name) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->MarkMainLonglink_ext(name);
};

bool (*LongLinkIsConnected_ext)(const std::string& name) = [](const std::string& name) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->LongLinkIsConnected_ext(name);
};
void (*MakesureLonglinkConnected_ext)(const std::string& name) = [](const std::string& name) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->MakesureLonglinkConnected_ext(name);
};

 */
 
}
}

#endif //MMNET_STN_LOGIC_BRIDGE_H
