//
// Created by Cpan on 2022/5/27.
//

#include "mars/stn/stn_manager.h"

#include <stdlib.h>

#include <iostream>
#include <map>
#include <random>

#include "mars/baseevent/active_logic.h"
#include "mars/baseevent/baseevent.h"
#include "mars/boost/config.hpp"
#include "mars/comm/alarm.h"
#include "mars/comm/messagequeue/message_queue.h"
#include "mars/comm/thread/atomic_oper.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"
#include "mars/stn/stn_callback_bridge.h"
#include "mars/xlog/appender.h"
#include "stn/src/net_core.h"  //一定要放这里，Mac os 编译
#include "stn/src/proxy_test.h"
#include "stn/src/signalling_keeper.h"

#ifdef WIN32
#include <locale>

#include "boost/filesystem/detail/utf8_codecvt_facet.hpp"
#include "boost/filesystem/path.hpp"
#endif

#include "mars/comm/alarm.h"
#include "mars/comm/thread/mutex.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace stn {

static const std::string kLibName = "stn";
static uint32_t gs_taskid = 1;

static uint32_t gs_sequence_id = 1;
static std::random_device rd;
static std::mt19937 mt_seed(rd());
static std::uniform_int_distribution<int> dist(0, 65535);

StnManager::StnManager(Context* context) : context_(context) {
    ReportDnsProfileFunc = std::bind(&StnManager::__ReportDnsProfile, this, std::placeholders::_1);
    xinfo_function(TSF "mars2");
}

StnManager::~StnManager() {
    xinfo_function(TSF "mars2");
}

std::string StnManager::GetName() {
    return typeid(StnManager).name();
}

void StnManager::OnInitConfigBeforeOnCreate(const int _packer_encoder_version) {
    xdebug2(TSF "mars2 OnInitConfigBeforeOnCreate %_", _packer_encoder_version);
    packer_encoder_version_ = _packer_encoder_version;
}

void StnManager::OnCreate() {
#if !UWP && !defined(WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif
    xinfo_function(TSF "mars2");
    ActiveLogic::Instance();
    if (!net_core_) {
        net_core_ = std::make_shared<NetCore>(context_, packer_encoder_version_, true);
        NetCore::NetCoreCreate()(net_core_);
    }
}

void StnManager::OnDestroy() {
    xinfo_function(TSF "mars2");
    if (nullptr == net_core_) {
        xwarn2(TSF "net core is nullptr. ignore destroy");
        return;
    }
    auto tmp_net_core = net_core_;
    net_core_ = nullptr;
    NetCore::__Release(tmp_net_core);
    NetCore::NetCoreRelease()();
    callback_bridge_->SetCallback(nullptr);
    tmp_net_core.reset();
    callback_ = nullptr;
    delete callback_bridge_;
}
void StnManager::OnSingalCrash(int _sig) {
    mars::xlog::appender_close();
}

void StnManager::OnExceptionCrash() {
    mars::xlog::appender_close();
}

void StnManager::OnNetworkChange(void (*pre_change)()) {
    if (net_core_ && !net_core_->IsAlreadyRelease()) {
        pre_change();
        net_core_->OnNetworkChange();
    }
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
}

void StnManager::SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) {
    xinfo2("mars2 SetStnCallbackBridge, old:%p, new:%p", callback_bridge_, _callback_bridge);
    if (!callback_bridge_) {
        callback_bridge_ = _callback_bridge;
    } else {
        callback_bridge_->SetCallback(nullptr);
        callback_bridge_ = _callback_bridge;
    }
    callback_bridge_->SetCallback(callback_);
}

StnCallbackBridge* StnManager::GetStnCallbackBridge() {
    if (!callback_bridge_) {
        callback_bridge_ = new StnCallbackBridge();  // std::make_shared<StnCallbackBridge>();
        callback_bridge_->SetCallback(callback_);
    }
    return callback_bridge_;
}

std::shared_ptr<LongLink> StnManager::DefaultLongLink() {
    if (net_core_) {
        return net_core_->DefaultLongLink();
    }
    return nullptr;
}

// #################### stn.h callback ####################
bool StnManager::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->MakesureAuthed(_host, _user_id);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return false;
    }
}

// 流量统计
void StnManager::TrafficData(ssize_t _send, ssize_t _recv) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->TrafficData(_send, _recv);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 底层询问上层该host对应的ip列表
std::vector<std::string> StnManager::OnNewDns(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _host_extra_info) {
    std::vector<std::string> ips;
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->OnNewDns(_host, _longlink_host, _host_extra_info);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return std::vector<std::string>();
    }
}

// 网络层收到push消息回调
void StnManager::OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 底层获取task要发送的数据
bool StnManager::Req2Buf(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         int channel_select,
                         const std::string& host,
                         const unsigned short client_sequence_id) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->Req2Buf(taskid,
                                         user_context,
                                         _user_id,
                                         outbuffer,
                                         extend,
                                         error_code,
                                         channel_select,
                                         host,
                                         client_sequence_id);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return false;
    }
}

// 底层回包返回给上层解析
int StnManager::Buf2Resp(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         const AutoBuffer& inbuffer,
                         const AutoBuffer& extend,
                         int& error_code,
                         int channel_select,
                         unsigned short& server_sequence_id,
                         const std::map<std::string, std::string>& _task_extra_info) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->Buf2Resp(taskid,
                                          user_context,
                                          _user_id,
                                          inbuffer,
                                          extend,
                                          error_code,
                                          channel_select,
                                          server_sequence_id,
                                          _task_extra_info);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return 0;
    }
}

// 任务执行结束
int StnManager::OnTaskEnd(uint32_t taskid,
                          void* const user_context,
                          const std::string& _user_id,
                          int error_type,
                          int error_code,
                          const ConnectProfile& _profile) {
    xassert2(callback_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return 0;
    }
}

// 上报网络连接状态
void StnManager::ReportConnectStatus(int status, int longlink_status) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->ReportConnectStatus(status, longlink_status);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

void StnManager::OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

void StnManager::OnShortLinkNetworkError(ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

void StnManager::OnLongLinkStatusChange(int _status) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->OnLongLinkStatusChange(_status);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int StnManager::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& identify_buffer,
                                               AutoBuffer& buffer_hash,
                                               int32_t& cmdid) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return 0;
    }
}

// 长连信令校验回包
bool StnManager::OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& response_buffer,
                                            const AutoBuffer& identify_buffer_hash) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        return callback_bridge_->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
        return false;
    }
}

void StnManager::RequestSync() {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->RequestSync();
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

////底层询问上层http网络检查的域名列表
void StnManager::RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->RequestNetCheckShortLinkHosts(_hostlist);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 底层向上层上报cgi执行结果
void StnManager::ReportTaskProfile(const TaskProfile& _task_profile) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->ReportTaskProfile(_task_profile);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 底层通知上层cgi命中限制
void StnManager::ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->ReportTaskLimited(_check_type, _task, _param);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

// 底层上报域名dns结果
void StnManager::__ReportDnsProfile(const DnsProfile& _dns_profile) {
    xassert2(callback_bridge_ != NULL);
    if (callback_bridge_) {
        callback_bridge_->ReportDnsProfile(_dns_profile);
    } else {
        xwarn2(TSF "mars2 callback_bridget is null.");
    }
}

//.生成taskid.
uint32_t StnManager::GenTaskID() {
    if (BOOST_UNLIKELY(atomic_read32(&gs_taskid) >= kReservedTaskIDStart)) {
        atomic_write32(&gs_taskid, 1);
    }
    return atomic_inc32(&gs_taskid);
}

unsigned short StnManager::GenSequenceId() {
    unsigned short sequence = dist(mt_seed);
    xdebug2(TSF "sequence %_", sequence);
    return sequence;
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
    if (net_core_) {
        net_core_->GetNetSource()->SetLongLink(hosts, ports, debugip);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::SetShortlinkSvrAddr(const uint16_t port, const std::string& debugip) {
    if (net_core_) {
        net_core_->GetNetSource()->SetShortlink(port, debugip);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::SetDebugIP(const std::string& host, const std::string& ip) {
    if (net_core_) {
        net_core_->GetNetSource()->SetDebugIP(host, ip);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::SetBackupIPs(const std::string& host, const std::vector<std::string>& iplist) {
    if (net_core_) {
        net_core_->GetNetSource()->SetBackupIPs(host, iplist);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

bool StnManager::StartTask(const Task& _task) {
    if (net_core_) {
        net_core_->StartTask(_task);
        return true;
    } else {
        xwarn2(TSF "net core is empty.");
    }
    return false;
}

void StnManager::StopTask(uint32_t _taskid) {
    if (net_core_) {
        net_core_->StopTask(_taskid);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

bool StnManager::HasTask(uint32_t taskid) {
    if (net_core_) {
        return net_core_->HasTask(taskid);
    } else {
        xwarn2(TSF "net core is empty.");
    }
    return false;
}

void StnManager::RedoTasks() {
    if (net_core_) {
        net_core_->RedoTasks();
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::TouchTasks() {
    if (net_core_) {
        net_core_->TouchTasks();
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::DisableLongLink() {
    if (net_core_) {
        net_core_->SetNeedUseLongLink(false);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::ClearTasks() {
    if (net_core_) {
        net_core_->ClearTasks();
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::Reset() {
    xinfo2(TSF "mars2");
    auto tmp_net_core = net_core_;
    net_core_ = nullptr;
    NetCore::__Release(tmp_net_core);
    NetCore::NetCoreRelease()();
    tmp_net_core.reset();
    net_core_ = std::make_shared<NetCore>(context_, packer_encoder_version_, true);
    NetCore::NetCoreCreate()(net_core_);
}

void StnManager::ResetAndInitEncoderVersion(int _packer_encoder_version) {
    xinfo_function(TSF "mars2 packer_encoder_version:%_", _packer_encoder_version);
    packer_encoder_version_ = _packer_encoder_version;
    auto tmp_net_core = net_core_;
    net_core_ = nullptr;
    NetCore::__Release(tmp_net_core);
    NetCore::NetCoreRelease()();
    tmp_net_core.reset();
    net_core_ = std::make_shared<NetCore>(context_, packer_encoder_version_, true);
    NetCore::NetCoreCreate()(net_core_);
}

void StnManager::SetSignallingStrategy(long _period, long _keepTime) {
    SignallingKeeper::SetStrategy((unsigned int)_period, (unsigned int)_keepTime);
}

void StnManager::KeepSignalling() {
#ifdef USE_LONG_LINK
    if (net_core_) {
        net_core_->KeepSignal();
    } else {
        xwarn2(TSF "net core is empty.");
    }
#endif
}

void StnManager::StopSignalling() {
#ifdef USE_LONG_LINK
    if (net_core_) {
        net_core_->StopSignal();
    } else {
        xwarn2(TSF "net core is empty.");
    }
#endif
}

void StnManager::MakesureLonglinkConnected() {
    if (net_core_) {
        net_core_->MakeSureLongLinkConnect();
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

bool StnManager::LongLinkIsConnected() {
    if (net_core_) {
        return net_core_->LongLinkIsConnected();
    } else {
        xwarn2(TSF "net core is empty.");
    }
    return false;
}

uint32_t StnManager::getNoopTaskID() {
    return Task::kNoopTaskID;
}

void StnManager::CreateLonglink_ext(LonglinkConfig& _config) {
    if (net_core_) {
        net_core_->CreateLongLink(_config);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::DestroyLonglink_ext(const std::string& name) {
    if (net_core_) {
        net_core_->DestroyLongLink(name);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

std::vector<std::string> StnManager::GetAllLonglink_ext() {
    return std::vector<std::string>();
}

void StnManager::MarkMainLonglink_ext(const std::string& name) {
    if (net_core_) {
        net_core_->MarkMainLonglink_ext(name);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

bool StnManager::LongLinkIsConnected_ext(const std::string& name) {
    if (net_core_) {
        return net_core_->LongLinkIsConnected_ext(name);
    } else {
        xwarn2(TSF "net core is empty.");
    }
    return false;
}

bool StnManager::ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info,
                                  const std::string& _test_host,
                                  const std::vector<std::string>& _hardcode_ips) {
    ProxyTest* proxy_test_ = new ProxyTest();
    return proxy_test_->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
}

void StnManager::MakesureLonglinkConnected_ext(const std::string& name) {
    if (net_core_) {
        net_core_->MakeSureLongLinkConnect_ext(name);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

// #################### end stn_logci.h ####################

const std::vector<std::string>& StnManager::GetLongLinkHosts() {
    if (net_core_) {
        return net_core_->GetNetSource()->GetLongLinkHosts();
    } else {
        xwarn2(TSF "net core is empty.");
    }
    return empty_longlink_hosts;
}

void StnManager::SetLongLink(const std::vector<std::string>& _hosts,
                             const std::vector<uint16_t>& _ports,
                             const std::string& _debugip) {
    if (net_core_) {
        net_core_->GetNetSource()->SetLongLink(_hosts, _ports, _debugip);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

void StnManager::SetShortlink(const uint16_t _port, const std::string& _debugip) {
    if (net_core_) {
        net_core_->GetNetSource()->SetShortlink(_port, _debugip);
    } else {
        xwarn2(TSF "net core is empty.");
    }
}

}  // namespace stn
}  // namespace mars
