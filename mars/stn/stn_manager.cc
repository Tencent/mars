//
// Created by Cpan on 2022/5/27.
//

#include "mars/stn/stn_manager.h"

#include <stdlib.h>

#include <map>
#include <string>

#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseevent.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/boost/config.hpp"
#include "mars/boost/signals2.hpp"
#include "mars/comm/alarm.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/singleton.h"
#include "mars/comm/thread/atomic_oper.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/log/appender.h"
#include "mars/stn/stn.h"
#include "mars/stn/stn_callback_bridge.h"
#include "stn/src/net_core.h"  //一定要放这里，Mac os 编译
#include "stn/src/net_source.h"
#include "stn/src/proxy_test.h"
#include "stn/src/signalling_keeper.h"

#ifdef WIN32
#include <locale>

#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#include "boost/filesystem/path.hpp"
#endif

#include "mars/comm/alarm.h"
#include "mars/comm/thread/mutex.h"

//#ifdef Android
//#include <android/log.h>
//#endif

namespace mars {
namespace stn {

static const std::string kLibName = "stn";
static uint32_t gs_taskid = 1;

StnManager::StnManager(BaseContext* context) : context_(context) {
    //xdebug_function(TSF "mars2 context id %_", context_->GetContextId());
}

StnManager::~StnManager() {
}

void StnManager::Init() {
    //xinfo2(TSF "cpan debug Init");
}

void StnManager::UnInit() {
}

void StnManager::OnInitConfigBeforeOnCreate(const int _packer_encoder_version) {
    //xinfo2(TSF "stn oninit: %_", _packer_encoder_version);
//    __android_log_print(ANDROID_LOG_DEBUG, "mars2", "stn manager OnInitConfigBeforeOnCreate _packer_encoder_version %d", _packer_encoder_version);
    packer_encoder_version_ = _packer_encoder_version;
    // LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);
}

void StnManager::OnCreate() {
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif
    //xinfo2(TSF "stn_manager oncreate");
    ActiveLogic::Instance();
    // NetCore::Singleton::Instance();
    //xinfo2(TSF "cpan debug OnCreate");

    if (!net_core_) {
        //xinfo2(TSF "cpan debug OnCreate 1");
        // net_core_ = std::make_shared<NetCore>(context_, packer_encoder_version_);
        if (context_) {
            xinfo2(TSF "mars2 context is no empty");
        } else {
            xerror2(TSF "mars context is empty.");
        }
        net_core_ = new NetCore(context_, packer_encoder_version_);
    }
}

void StnManager::OnDestroy() {
    //xinfo2(TSF "stn onDestroy");

    // callback_.reset();
    callback_bridge_->SetCallback(nullptr);
    //    callback_bridge_.reset();
    //    net_core_.reset();

    delete callback_;
    delete callback_bridge_;
    delete net_core_;

    // others use activelogic may crash after activelogic release. eg: LongLinkConnectMonitor
    // ActiveLogic::Singleton::Release();
}
void StnManager::OnSingalCrash(int _sig) {
    mars::xlog::appender_close();
}

void StnManager::OnExceptionCrash() {
    mars::xlog::appender_close();
}

void StnManager::OnNetworkChange(void (*pre_change)()) {
    //xinfo2(TSF "cpan debug OnNetworkChange");
    pre_change();
    net_core_->OnNetworkChange();
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
    // callback_ = std::shared_ptr<Callback>(_callback);
    xdebug2(TSF"mars2 SetCallback");
    callback_ = _callback;
}

void StnManager::SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    xdebug2(TSF"mars2 SetStnCallbackBridge");
    if (!callback_bridge_) {
        callback_bridge_ = _callback_bridge;
    } else {
        callback_bridge_->SetCallback(nullptr);
        //        callback_bridge_.reset(_callback_bridge);
        callback_bridge_ = _callback_bridge;
    }
    callback_bridge_->SetCallback(callback_);
}

StnCallbackBridge* StnManager::GetStnCallbackBridge() {
    xdebug2(TSF"mars2 GetStnCallbackBridge");
    if (!callback_bridge_) {
        callback_bridge_ = new StnCallbackBridge();  // std::make_shared<StnCallbackBridge>();
        callback_bridge_->SetCallback(callback_);
    }
    return callback_bridge_;
}

// #################### stn.h callback ####################
bool StnManager::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    xassert2(callback_bridge_ != NULL);
    return callback_bridge_->MakesureAuthed(_host, _user_id);
}

// 流量统计
void StnManager::TrafficData(ssize_t _send, ssize_t _recv) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->TrafficData(_send, _recv);
}

// 底层询问上层该host对应的ip列表
std::vector<std::string> StnManager::OnNewDns(const std::string& _host, bool _longlink_host) {
    std::vector<std::string> ips;
    xassert2(callback_bridge_ != NULL);
    return callback_bridge_->OnNewDns(_host, _longlink_host);
}

// 网络层收到push消息回调
void StnManager::OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

// 底层获取task要发送的数据
bool StnManager::Req2Buf(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         const int channel_select,
                         const std::string& host) {
    xassert2(callback_bridge_ != NULL);
    return callback_bridge_
        ->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}

// 底层回包返回给上层解析
int StnManager::Buf2Resp(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         const AutoBuffer& inbuffer,
                         const AutoBuffer& extend,
                         int& error_code,
                         const int channel_select) {
    xdebug2(TSF"mars2 Buf2Resp");
    xassert2(callback_bridge_ != NULL);
    xdebug2(TSF"mars2 Buf2Resp no null.");
    return callback_bridge_->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}

// 任务执行结束
int StnManager::OnTaskEnd(uint32_t taskid,
                          void* const user_context,
                          const std::string& _user_id,
                          int error_type,
                          int error_code,
                          const ConnectProfile& _profile) {
    xassert2(callback_ != NULL);
    return callback_bridge_->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

// 上报网络连接状态
void StnManager::ReportConnectStatus(int status, int longlink_status) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->ReportConnectStatus(status, longlink_status);
}

void StnManager::OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void StnManager::OnShortLinkNetworkError(ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void StnManager::OnLongLinkStatusChange(int _status) {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->OnLongLinkStatusChange(_status);
}

// 长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int StnManager::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& identify_buffer,
                                               AutoBuffer& buffer_hash,
                                               int32_t& cmdid) {
    xassert2(callback_bridge_ != NULL);
    return callback_bridge_->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}

// 长连信令校验回包
bool StnManager::OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& response_buffer,
                                            const AutoBuffer& identify_buffer_hash) {
    xassert2(callback_bridge_ != NULL);
    return callback_bridge_->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void StnManager::RequestSync() {
    xassert2(callback_bridge_ != NULL);
    callback_bridge_->RequestSync();
}

// 验证是否已登录

////底层询问上层http网络检查的域名列表
void StnManager::RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    if (!callback_bridge_) {
        return;
    }
    callback_bridge_->RequestNetCheckShortLinkHosts(_hostlist);
}

// 底层向上层上报cgi执行结果
void StnManager::ReportTaskProfile(const TaskProfile& _task_profile) {
    if (!callback_bridge_) {
        return;
    }
    callback_bridge_->ReportTaskProfile(_task_profile);
}

// 底层通知上层cgi命中限制
void StnManager::ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    if (!callback_bridge_) {
        return;
    }
    callback_bridge_->ReportTaskLimited(_check_type, _task, _param);
}

// 底层上报域名dns结果
void StnManager::ReportDnsProfile(const DnsProfile& _dns_profile) {
    if (!callback_bridge_) {
        return;
    }
    callback_bridge_->ReportDnsProfile(_dns_profile);
}

//.生成taskid.
uint32_t StnManager::GenTaskID() {
    if (BOOST_UNLIKELY(atomic_read32(&gs_taskid) >= kReservedTaskIDStart)) {
        atomic_write32(&gs_taskid, 1);
    }
    return atomic_inc32(&gs_taskid);
}
// #################### end stn.h / callback ####################

// #################### stn_logci.h ####################

void StnManager::SetLonglinkSvrAddr(const std::string& host,
                                    const std::vector<uint16_t> ports,
                                    const std::string& debugip) {
    std::vector<std::string> hosts;
    if (!host.empty()) {
        hosts.push_back(host);
    }
    net_core_->GetNetSource()->SetLongLink(hosts, ports, debugip);
}

void StnManager::SetShortlinkSvrAddr(const uint16_t port, const std::string& debugip) {
    net_core_->GetNetSource()->SetShortlink(port, debugip);
}

void StnManager::SetDebugIP(const std::string& host, const std::string& ip) {
    net_core_->GetNetSource()->SetDebugIP(host, ip);
}

void StnManager::SetBackupIPs(const std::string& host, const std::vector<std::string>& iplist) {
    net_core_->GetNetSource()->SetBackupIPs(host, iplist);
}

bool StnManager::StartTask(const Task& _task) {
    net_core_->StartTask(_task);
    return true;
}

void StnManager::StopTask(uint32_t _taskid) {
    net_core_->StopTask(_taskid);
}

bool StnManager::HasTask(uint32_t taskid) {
    return net_core_->HasTask(taskid);
}

void StnManager::RedoTasks() {
    net_core_->RedoTasks();
}

void StnManager::TouchTasks() {
    net_core_->TouchTasks();
}

void StnManager::DisableLongLink() {
    NetCore::need_use_longlink_ = false;
}

void StnManager::ClearTasks() {
    net_core_->ClearTasks();
}

void StnManager::Reset() {
    //xinfo2(TSF "stn reset");
    // NetCore::Singleton::Release();
    // NetCore::Singleton::Instance();
    //    net_core_.reset(new NetCore());
    //xinfo2(TSF "cpan debug Reset");
    // net_core_.reset(new NetCore(context_, packer_encoder_version_));

    xdebug_function(TSF"mars2");
    delete net_core_;
    net_core_ = new NetCore(context_, packer_encoder_version_);
}

void StnManager::ResetAndInitEncoderVersion(int _packer_encoder_version) {
    xdebug_function(TSF"mars2 packer_encoder_version:%_", _packer_encoder_version);
    //xinfo2(TSF "stn reset, encoder version: %_", _packer_encoder_version);
    packer_encoder_version_ = _packer_encoder_version;
    // LongLinkEncoder::SetEncoderVersion(_packer_encoder_version);

    // NetCore::Singleton::Release();
    // NetCore::Singleton::Instance();
    //    net_core_.reset(new NetCore());
    //xinfo2(TSF "cpan debug ResetAndInitEncoderVersion");
    // net_core_.reset(new NetCore(context_, packer_encoder_version_));

    //TODO cpan mars2 TODOTDOO TODOTDOOTODOTDOOTODOTDOOTODOTDOOTODOTDOOTODOTDOO
    delete net_core_;
    net_core_ = new NetCore(context_, packer_encoder_version_);
}

void StnManager::SetSignallingStrategy(long _period, long _keepTime) {
    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
}

void StnManager::KeepSignalling() {
#ifdef USE_LONG_LINK
    net_core_->KeepSignal();
#endif
}

void StnManager::StopSignalling() {
#ifdef USE_LONG_LINK
    net_core_->StopSignal();

#endif
}

void StnManager::MakesureLonglinkConnected() {
    //xinfo2(TSF "make sure longlink connect");
    net_core_->MakeSureLongLinkConnect();
}

bool StnManager::LongLinkIsConnected() {
    return false;
}

uint32_t StnManager::getNoopTaskID() {
    return Task::kNoopTaskID;
}

void StnManager::CreateLonglink_ext(LonglinkConfig& _config) {
    net_core_->CreateLongLink(_config);
}

void StnManager::DestroyLonglink_ext(const std::string& name) {
    net_core_->DestroyLongLink(name);
}

std::vector<std::string> StnManager::GetAllLonglink_ext() {
    return std::vector<std::string>();
}

void StnManager::MarkMainLonglink_ext(const std::string& name) {
    net_core_->MarkMainLonglink_ext(name);
}

bool StnManager::LongLinkIsConnected_ext(const std::string& name) {
    return net_core_->LongLinkIsConnected_ext(name);
}

bool StnManager::ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info,
                                  const std::string& _test_host,
                                  const std::vector<std::string>& _hardcode_ips) {
    //    return ProxyTest::Singleton::Instance()->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    ProxyTest* proxy_test_ = new ProxyTest();
    return proxy_test_->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
}

void StnManager::MakesureLonglinkConnected_ext(const std::string& name) {
    net_core_->MakeSureLongLinkConnect_ext(name);
}

// #################### end stn_logci.h ####################

const std::vector<std::string>& StnManager::GetLongLinkHosts() {
    return net_core_->GetNetSourceRef().GetLongLinkHosts();
}

void StnManager::SetLongLink(const std::vector<std::string>& _hosts,
                             const std::vector<uint16_t>& _ports,
                             const std::string& _debugip) {
    net_core_->GetNetSourceRef().SetLongLink(_hosts, _ports, _debugip);
}

void StnManager::SetShortlink(const uint16_t _port, const std::string& _debugip) {
    net_core_->GetNetSourceRef().SetShortlink(_port, _debugip);
}

NetCore* StnManager::GetNetCore() {
    return net_core_;
}

}  // namespace stn
}  // namespace mars
