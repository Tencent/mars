//
// Created by Cpan on 2022/5/27.
//

#include "stn_manager.h"

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
#include "mars/stn/stn_callback_bridge.h"
#include "mars/stn/stn.h"
#include "mars/boost/config.hpp"
#include "mars/comm/thread/atomic_oper.h"

#ifdef WIN32
#include <locale>
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#endif

#include "mars/comm/alarm.h"

namespace mars {
namespace stn {

static const uint32_t kReservedTaskIDStart = 0xFFFFFFF0;
static const std::string kLibName = "stn";
static uint32_t gs_taskid = 1;
static Callback* sg_callback = NULL;

#define STN_WEAK_CALL(func) \
    if (!net_core_) {\
        xwarn2(TSF"stn uncreate");\
        return;\
    }\
    net_core_->func
    
#define STN_RETURN_WEAK_CALL(func) \
    if (!net_core_) {\
        xwarn2(TSF"stn uncreate");\
        return false;\
    }\
    net_core_->func;\
    return true

#define STN_WEAK_CALL_RETURN(func, ret) \
    if (net_core_) \
    {\
        ret = net_core_->func;\
    }

#define STN_CALLBACK_WEAK_CALL(func) \
    if (!stn_callback_bridge_) {\
        xwarn2(TSF"callback no set");\
        return;\
    }\
stn_callback_bridge_->func

#define STN_CALLBACK_RETURN_WEAK_CALL(func) \
    if (!stn_callback_bridge_) {\
        xwarn2(TSF"callback no set");\
        return false;\
    }\
stn_callback_bridge_->func;\
    return true

#define STN_CALLBACK_WEAK_CALL_RETURN(func, ret) \
    if (stn_callback_bridge_) {\
        ret = stn_callback_bridge_->func;                 \
    }

StnManager::StnManager(){
    stn_callback_bridge_ = new StnCallbackBridge();
}

StnManager::~StnManager(){
    
}

void StnManager::OnInitConfigBeforeOnCreate(int _packer_encoder_version) {
    xinfo2(TSF"stn oninit: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
}

void StnManager::OnCreate() {
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif
    xinfo2(TSF"stn_manager oncreate");
    ActiveLogic::Instance();
    //NetCore::Singleton::Instance();
    net_core_ = NetCore::Singleton::Instance().get();//new NetCore();
}

void StnManager::OnDestroy() {
    xinfo2(TSF"stn onDestroy");

    //NetCore::Singleton::Release();
    SINGLETON_RELEASE_ALL();
    
    // others use activelogic may crash after activelogic release. eg: LongLinkConnectMonitor
    // ActiveLogic::Singleton::Release();
}
void StnManager::OnSingalCrash(int _sig) {
    mars::xlog::appender_close();
}

void StnManager::OnExceptionCrash() {
    mars::xlog::appender_close();
}

void StnManager::OnNetworkChange() {
    STN_WEAK_CALL(OnNetworkChange());
    
}
void StnManager::OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    if (NULL == _tag || strnlen(_tag, 1024) == 0) {
        xassert2(false);
        return;
    }
    
    if (0 == strcmp(_tag, XLOGGER_TAG)) {
        TrafficData(_send, _recv);
    }
}

#ifdef ANDROID
void StnManager::OnAlarm(int64_t _id) {
    Alarm::onAlarmImpl(_id);
}
#endif


void StnManager::SetCallback(Callback* const _callback) {
    callback_ = _callback;
    stn_callback_bridge_->SetCallback(callback_);
}

void StnManager::SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    stn_callback_bridge_ = _callback_bridge;
    if(callback_) {
        stn_callback_bridge_->SetCallback(callback_);
    }
}

StnCallbackBridge* StnManager::GetStnCallbackBridge() {
    return stn_callback_bridge_;
}

// #################### stn.h ####################
bool StnManager::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->MakesureAuthed(_host, _user_id);
}

//流量统计
void StnManager::TrafficData(ssize_t _send, ssize_t _recv) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->TrafficData(_send, _recv);
}

//底层询问上层该host对应的ip列表
std::vector<std::string> StnManager::OnNewDns(const std::string& _host, bool _longlink_host) {
    std::vector<std::string> ips;
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->OnNewDns(_host, _longlink_host);
}

//网络层收到push消息回调
void StnManager::OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

//底层获取task要发送的数据
bool StnManager::Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}

//底层回包返回给上层解析
int StnManager::Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}

//任务执行结束
int StnManager::OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

//上报网络连接状态
void StnManager::ReportConnectStatus(int status, int longlink_status) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->ReportConnectStatus(status, longlink_status);
}

void StnManager::OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void StnManager::OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void StnManager::OnLongLinkStatusChange(int _status) {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->OnLongLinkStatusChange(_status);
}

//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int StnManager::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}

//长连信令校验回包
bool StnManager::OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    xassert2(stn_callback_bridge_ != NULL);
    return stn_callback_bridge_->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void StnManager::RequestSync() {
    xassert2(stn_callback_bridge_ != NULL);
    stn_callback_bridge_->RequestSync();
}

//验证是否已登录

//底层询问上层http网络检查的域名列表
void StnManager::RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    if (!stn_callback_bridge_) {
        return;
    }
    stn_callback_bridge_->RequestNetCheckShortLinkHosts(_hostlist);
}

//底层向上层上报cgi执行结果
void StnManager::ReportTaskProfile(const TaskProfile& _task_profile) {
    if (!stn_callback_bridge_) {
        return;
    }
    stn_callback_bridge_->ReportTaskProfile(_task_profile);
}

//底层通知上层cgi命中限制
void StnManager::ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    if (!stn_callback_bridge_) {
        return;
    }
    stn_callback_bridge_->ReportTaskLimited(_check_type, _task, _param);
}

//底层上报域名dns结果
void StnManager::ReportDnsProfile(const DnsProfile& _dns_profile) {
    if (!stn_callback_bridge_) {
        return;
    }
    stn_callback_bridge_->ReportDnsProfile(_dns_profile);
}

//.生成taskid.
uint32_t StnManager::GenTaskID() {
    if (BOOST_UNLIKELY(atomic_read32(&gs_taskid) >= kReservedTaskIDStart)) {
        atomic_write32(&gs_taskid, 1);
    }
    return atomic_inc32(&gs_taskid);
}

// #################### end stn.h ####################

// #################### stn_logci.h ####################

void StnManager::SetLonglinkSvrAddr(const std::string &host, const std::vector<uint16_t> ports, const std::string &debugip) {
    std::vector<std::string> hosts;
    if (!host.empty()) {
        hosts.push_back(host);
    }
    NetSource::SetLongLink(hosts, ports, debugip);
}

void StnManager::SetShortlinkSvrAddr(const uint16_t port, const std::string &debugip) {
    NetSource::SetShortlink(port, debugip);
}

void StnManager::SetDebugIP(const std::string &host, const std::string &ip) {
    NetSource::SetDebugIP(host, ip);
}

void StnManager::SetBackupIPs(const std::string &host, const std::vector<std::string> &iplist) {
    NetSource::SetBackupIPs(host, iplist);
}

bool StnManager::StartTask(const Task &_task) {
    STN_RETURN_WEAK_CALL(StartTask(_task));
    return true;
}

void StnManager::StopTask(uint32_t _taskid) {
    STN_WEAK_CALL(StopTask(_taskid));
}

bool StnManager::HasTask(uint32_t taskid) {
    bool has_task = false;
    STN_WEAK_CALL_RETURN(HasTask(taskid), has_task);
    return has_task;
}

void StnManager::RedoTasks() {
    STN_WEAK_CALL(RedoTasks());
}

void StnManager::TouchTasks() {
    STN_WEAK_CALL(TouchTasks());
}


void StnManager::DisableLongLink() {
    NetCore::need_use_longlink_ = false;
}

void StnManager::ClearTasks() {
    STN_WEAK_CALL(ClearTasks());
}

void StnManager::Reset() {
    xinfo2(TSF "stn reset");
    //NetCore::Singleton::Release();
    //NetCore::Singleton::Instance();
    delete net_core_;
    net_core_ = new NetCore();
}

void StnManager::ResetAndInitEncoderVersion(int _packer_encoder_version) {
    xinfo2(TSF "stn reset, encoder version: %_", _packer_encoder_version);
    LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
    //NetCore::Singleton::Release();
    //NetCore::Singleton::Instance();
    delete net_core_;
    net_core_ = new NetCore();
}

void StnManager::SetSignallingStrategy(long _period, long _keepTime) {
    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
}

void StnManager::KeepSignalling() {
    
#ifdef USE_LONG_LINK
    STN_WEAK_CALL(KeepSignal());
#endif
    
}

void StnManager::StopSignalling() {

#ifdef USE_LONG_LINK
    STN_WEAK_CALL(StopSignal());

#endif

}

void StnManager::MakesureLonglinkConnected() {
    STN_WEAK_CALL(MakeSureLongLinkConnect());
}

bool StnManager::LongLinkIsConnected() {
    return false;
}

uint32_t StnManager::getNoopTaskID() {
    return Task::kNoopTaskID;
}

void StnManager::CreateLonglink_ext(LonglinkConfig &_config) {
    STN_WEAK_CALL(CreateLongLink(_config));
}

void StnManager::DestroyLonglink_ext(const std::string &name) {
    STN_WEAK_CALL(DestroyLongLink(name));
}

std::vector<std::string> StnManager::GetAllLonglink_ext() {
    return std::vector<std::string>();
}

void StnManager::MarkMainLonglink_ext(const std::string &name) {
    STN_WEAK_CALL(MarkMainLonglink_ext(name));
}

bool StnManager::LongLinkIsConnected_ext(const std::string &name) {
    bool res = false;
    STN_WEAK_CALL_RETURN(LongLinkIsConnected_ext(name),res);
    return res;
}

bool StnManager::ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
    return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
}

void StnManager::MakesureLonglinkConnected_ext(const std::string &name) {
    STN_WEAK_CALL(MakeSureLongLinkConnect_ext(name));
}

// #################### end stn_logci.h ####################

//
//// 'host' will be ignored when 'debugip' is not empty.
//void (*SetLonglinkSvrAddr)(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) = [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
//    std::vector<std::string> hosts;
//    if (!host.empty()) {
//        hosts.push_back(host);
//    }
//    NetSource::SetLongLink(hosts, ports, debugip);
//};
//
//// 'task.host' will be ignored when 'debugip' is not empty.
//void (*SetShortlinkSvrAddr)(const uint16_t port, const std::string& debugip)  = [](const uint16_t port, const std::string& debugip) {
//    NetSource::SetShortlink(port, debugip);
//};
//
//// setting debug ip address for the corresponding host
//void (*SetDebugIP)(const std::string& host, const std::string& ip) = [](const std::string& host, const std::string& ip) {
//    NetSource::SetDebugIP(host, ip);
//};
//
//// setting backup iplist for the corresponding host
//// if debugip is not empty, iplist will be ignored.
//// iplist will be used when newdns/dns ip is not available.
//void (*SetBackupIPs)(const std::string& host, const std::vector<std::string>& iplist) = [](const std::string& host, const std::vector<std::string>& iplist) {
//    NetSource::SetBackupIPs(host, iplist);
//};
//
//// async function.
//bool (*StartTask)(const Task& task) = [](const Task& _task) {
//    STN_RETURN_WEAK_CALL(StartTask(_task));
//};
//
//// sync function
//void (*StopTask)(uint32_t taskid) = [](uint32_t _taskid) {
//    STN_WEAK_CALL(StopTask(_taskid));
//};
//
//// check whether task's list has the task or not.
//bool (*HasTask)(uint32_t taskid) = [](uint32_t _taskid) {
//    bool has_task = false;
//    STN_WEAK_CALL_RETURN(HasTask(_taskid), has_task);
//    return has_task;
//};
//
//// reconnect longlink and redo all task
//// when you change svr ip, you must call this function.
//void (*RedoTasks)() = []() {
//    STN_WEAK_CALL(RedoTasks());
//};
//
//// touch tasks loop. Generally, invoke it after autoauth successfully.
//void (*TouchTasks)() = []() {
//    STN_WEAK_CALL(TouchTasks());
//};
//
////need longlink channel
//void (*DisableLongLink)() = []() {
//    NetCore::need_use_longlink_ = false;
//};
//
//// stop and clear all task
//void (*ClearTasks)() = []() {
//    STN_WEAK_CALL(ClearTasks());
//};
//
//// the same as ClearTasks(), but also reinitialize network.
//void (*Reset)() = []() {
//    xinfo2(TSF "stn reset");
//    NetCore::Singleton::Release();
//    NetCore::Singleton::Instance();
//};
//
//void (*ResetAndInitEncoderVersion)(int _encoder_version) = [](int _encoder_version) {
//    xinfo2(TSF "stn reset, encoder version: %_", _encoder_version);
//    LongLinkEncoder::SetEncoderVersion(_encoder_version);
//    NetCore::Singleton::Release();
//    NetCore::Singleton::Instance();
//};
//
////setting signalling's parameters.
////if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
//void (*SetSignallingStrategy)(long _period, long _keepTime) = [](long _period, long _keepTime) {
//    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
//};
//
//// used to keep longlink active
//// keep signnaling once 'period' and last 'keeptime'
//void (*KeepSignalling)() = []() {
//#ifdef USE_LONG_LINK
//    STN_WEAK_CALL(KeepSignal());
//#endif
//};
//
//void (*StopSignalling)() = []() {
//#ifdef USE_LONG_LINK
//    STN_WEAK_CALL(StopSignal());
//#endif
//};
//
//// connect quickly if longlink is not connected.
//void (*MakesureLonglinkConnected)() = []() {
//};
//
//bool (*LongLinkIsConnected)() = []() {
//    bool connected = false;
//    STN_WEAK_CALL_RETURN(LongLinkIsConnected(), connected);
//    return connected;
//};
//
//bool (*ProxyIsAvailable)(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) = [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
//    return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
//};
//
//// noop is used to keep longlink conected
//// get noop taskid
//uint32_t (*getNoopTaskID)() = []() {
//    return Task::kNoopTaskID;
//};
//
////===----------------------------------------------------------------------===//
/////
///// Support multi longlinks for mars
///// these APIs are subject to change in developing
/////
////===----------------------------------------------------------------------===//
//void (*CreateLonglink_ext)(LonglinkConfig& _config) = [](LonglinkConfig& _config) {
//    STN_WEAK_CALL(CreateLongLink(_config));
//};
//
//void (*DestroyLonglink_ext)(const std::string& name) = [](const std::string& name) {
//    STN_WEAK_CALL(DestroyLongLink(name));
//};
//
////std::vector<std::string> (*GetAllLonglink_ext)() = []() {
////};
//
//void (*MarkMainLonglink_ext)(const std::string& name) = [](const std::string& name) {
//    STN_WEAK_CALL(MarkMainLonglink_ext(name));
//};
//
//bool (*LongLinkIsConnected_ext)(const std::string& name) = [](const std::string& name) {
//    bool res = false;
//    STN_WEAK_CALL_RETURN(LongLinkIsConnected_ext(name),res);
//    return res;
//};
//
//void (*MakesureLonglinkConnected_ext)(const std::string& name) = [](const std::string& name) {
//    STN_WEAK_CALL(MakeSureLongLinkConnect_ext(name));
//};

NetCore* StnManager::GetNetCore() {
    return net_core_;
}

}//end stn
}//end mars
