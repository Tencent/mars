//
// Created by Cpan on 2022/5/27.
//

#include "stn_logic_bridge.h"
#include "mars/comm/xlogger/xlogger.h"

#include "mars/comm/bootrun.h"
#include "stn/src/net_core.h"//一定要放这里，Mac os 编译
#include "mars/comm/xlogger/xlogger.h"
#include "mars/baseevent/baseprjevent.h"

namespace mars {
namespace stn {

void SetCallback(Callback* const callback) {
    xassert2(stn_manager_ != NULL);
    stn_manager_ ->SetCallback(callback);
}

void SetStnCallbackBridge(StnCallbackBridge* const _callback_bridge) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->SetStnCallbackBridge(_callback_bridge);
}

StnCallbackBridge* GetStnCallbackBridge() {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->GetStnCallbackBridge();
}


// #################### stn.h ####################
bool MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->MakesureAuthed(_host, _user_id);
}

//流量统计
void TrafficData(ssize_t _send, ssize_t _recv) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->TrafficData(_send, _recv);
}

//底层询问上层该host对应的ip列表
std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->OnNewDns(_host, _longlink_host);
}

//网络层收到push消息回调
void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->OnPush(_channel_id, _cmdid, _taskid, _body, _extend);
}

//底层获取task要发送的数据
bool Req2Buf(uint32_t taskid, void* const user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->Req2Buf(taskid, user_context, _user_id, outbuffer, extend, error_code, channel_select, host);
}
//底层回包返回给上层解析
int Buf2Resp(uint32_t taskid, void* const user_context, const std::string& _user_id, const AutoBuffer& inbuffer, const AutoBuffer& extend, int& error_code, const int channel_select) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->Buf2Resp(taskid, user_context, _user_id, inbuffer, extend, error_code, channel_select);
}
//任务执行结束
int  OnTaskEnd(uint32_t taskid, void* const user_context, const std::string& _user_id, int error_type, int error_code, const ConnectProfile& _profile) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->OnTaskEnd(taskid, user_context, _user_id, error_type, error_code, _profile);
}

//上报网络连接状态
void ReportConnectStatus(int status, int longlink_status) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ReportConnectStatus(status, longlink_status);
}

void OnLongLinkNetworkError(::mars::stn::ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->OnLongLinkNetworkError(_err_type, _err_code, _ip, _port);
}

void OnShortLinkNetworkError(::mars::stn::ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->OnShortLinkNetworkError(_err_type, _err_code, _ip, _host, _port);
}

void OnLongLinkStatusChange(int _status) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->OnLongLinkStatusChange(_status);
}

//长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& identify_buffer, AutoBuffer& buffer_hash, int32_t& cmdid) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->GetLonglinkIdentifyCheckBuffer(_channel_id, identify_buffer, buffer_hash, cmdid);
}

//长连信令校验回包
bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& response_buffer, const AutoBuffer& identify_buffer_hash) {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->OnLonglinkIdentifyResponse(_channel_id, response_buffer, identify_buffer_hash);
}

void RequestSync() {
    xassert2(stn_manager_ != NULL);
    stn_manager_->RequestSync();
}
//验证是否已登录

//底层询问上层http网络检查的域名列表
void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->RequestNetCheckShortLinkHosts(_hostlist);
}

//底层向上层上报cgi执行结果
void ReportTaskProfile(const TaskProfile& _task_profile) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ReportTaskProfile(_task_profile);
}

//底层通知上层cgi命中限制
void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ReportTaskLimited(_check_type, _task, _param);
}

//底层上报域名dns结果
void ReportDnsProfile(const DnsProfile& _dns_profile) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->ReportDnsProfile(_dns_profile);
}

//.生成taskid.
uint32_t GenTaskID() {
    xassert2(stn_manager_ != NULL);
    return stn_manager_->GenTaskID();
}

// #################### stn.h ####################


// #################### stn_logic.h ####################
// 'host' will be ignored when 'debugip' is not empty.


static void __InitFuncPtr() {
    
    stn_manager_ = new StnManager();
    
    SetLonglinkSvrAddr = [](const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->SetLonglinkSvrAddr(host, ports, debugip);
    };

    // 'task.host' will be ignored when 'debugip' is not empty.
    SetShortlinkSvrAddr = [](const uint16_t port, const std::string& debugip) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->SetShortlinkSvrAddr(port, debugip);
    };

    // setting debug ip address for the corresponding host
    SetDebugIP = [](const std::string& host, const std::string& ip) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->SetDebugIP(host, ip);
    };

    // setting backup iplist for the corresponding host
    // if debugip is not empty, iplist will be ignored.
    // iplist will be used when newdns/dns ip is not available.
    SetBackupIPs = [](const std::string& host, const std::vector<std::string>& iplist) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->SetBackupIPs(host, iplist);
    };


    // async function.
    StartTask = [](const Task& _task) {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->StartTask(_task);
    };

    // sync function
    StopTask = [](uint32_t _taskid) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->StopTask(_taskid);
    };


    // check whether task's list has the task or not.
    HasTask = [](uint32_t _taskid) {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->HasTask(_taskid);
    };

    // reconnect longlink and redo all task
    // when you change svr ip, you must call this function.
    RedoTasks = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->RedoTasks();
    };

    // touch tasks loop. Generally, invoke it after autoauth successfully.
    TouchTasks = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->TouchTasks();
    };

    //need longlink channel
    DisableLongLink = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->DisableLongLink();
    };

    // stop and clear all task
    ClearTasks = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->ClearTasks();
    };

    // the same as ClearTasks(), but also reinitialize network.
    Reset = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->Reset();
    };

    ResetAndInitEncoderVersion = [](int _encoder_version) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->ResetAndInitEncoderVersion(_encoder_version);
    };

    //setting signalling's parameters.
    //if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    SetSignallingStrategy = [](long _period, long _keepTime) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->SetSignallingStrategy(_period,_keepTime);
    };


    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    KeepSignalling = []() {
    #ifdef USE_LONG_LINK
        xassert2(stn_manager_ != NULL);
        stn_manager_->KeepSignalling();
    #endif
    };

    StopSignalling = []() {
    #ifdef USE_LONG_LINK
        xassert2(stn_manager_ != NULL);
        stn_manager_->StopSignalling();
    #endif
    };

    // connect quickly if longlink is not connected.
    MakesureLonglinkConnected = []() {
        xassert2(stn_manager_ != NULL);
        stn_manager_->MakesureLonglinkConnected();
    };

    LongLinkIsConnected = []() {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->LongLinkIsConnected();
    };

    ProxyIsAvailable = [](const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips) {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->ProxyIsAvailable(_proxy_info, _test_host, _hardcode_ips);
    };


    // noop is used to keep longlink conected
    // get noop taskid
    getNoopTaskID = []() {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->getNoopTaskID();
    };

    //===----------------------------------------------------------------------===//
    ///
    /// Support multi longlinks for mars
    /// these APIs are subject to change in developing
    ///
    //===----------------------------------------------------------------------===//
    CreateLonglink_ext = [](LonglinkConfig& _config) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->CreateLonglink_ext(_config);
    };

    DestroyLonglink_ext = [](const std::string& name) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->DestroyLonglink_ext(name);
    };

    //std::vector<std::string> (*GetAllLonglink_ext)();
    MarkMainLonglink_ext = [](const std::string& name) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->MarkMainLonglink_ext(name);
    };

    LongLinkIsConnected_ext = [](const std::string& name) {
        xassert2(stn_manager_ != NULL);
        return stn_manager_->LongLinkIsConnected_ext(name);
    };
    
    MakesureLonglinkConnected_ext = [](const std::string& name) {
        xassert2(stn_manager_ != NULL);
        stn_manager_->MakesureLonglinkConnected_ext(name);
    };

} //__InitFunc

static void onInitConfigBeforeOnCreate(int _packer_encoder_version) {
    stn_manager_->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

static void onCreate() {
    stn_manager_->OnCreate();
}

static void onDestroy() {
    stn_manager_->OnDestroy();
}

static void onSingalCrash(int _sig) {
    stn_manager_->OnSingalCrash(_sig);
}

static void onExceptionCrash() {
    stn_manager_->OnExceptionCrash();
}

static void onNetworkChange() {
    stn_manager_->OnNetworkChange();
}
    
static void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) {
    stn_manager_->OnNetworkDataChange(_tag, _send, _recv);
}

#ifdef ANDROID
//must dipatch by function in stn_logic.cc, to avoid static member bug
static void onAlarm(int64_t _id) {
    stn_manager_->OnAlarm(_id);
}
#endif

static void __initbind_baseprjevent() {
    
    __InitFuncPtr();
    
#ifdef WIN32
    boost::filesystem::path::imbue(std::locale(std::locale(), new boost::filesystem::detail::utf8_codecvt_facet));
#endif

#ifdef ANDROID
    mars::baseevent::addLoadModule(kLibName);
    GetSignalOnAlarm().connect(&onAlarm);
#endif
    GetSignalOnCreate().connect(&onCreate);
    GetSignalOnInitBeforeOnCreate().connect(boost::bind(&onInitConfigBeforeOnCreate, _1));
    GetSignalOnDestroy().connect(&onDestroy);   //low priority signal func
    GetSignalOnSingalCrash().connect(&onSingalCrash);
    GetSignalOnExceptionCrash().connect(&onExceptionCrash);
    GetSignalOnNetworkChange().connect(5, &onNetworkChange);    //define group 5

    
#ifndef XLOGGER_TAG
#error "not define XLOGGER_TAG"
#endif
    
    GetSignalOnNetworkDataChange().connect(&OnNetworkDataChange);
}

BOOT_RUN_STARTUP(__initbind_baseprjevent);



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
    stn_manager_->CreateLonglink_ext(_config);// CreateLongLink(_config);
};

void (*DestroyLonglink_ext)(const std::string& name) = [](const std::string& name) {
    xassert2(stn_manager_ != NULL);
    stn_manager_->DestroyLonglink_ext(name);// DestroyLongLink(name);
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
    stn_manager_->MakesureLonglinkConnected_ext(name);// MakeSureLongLinkConnect_ext(name);
};


// #################### stn_logic.h ####################
}
}
