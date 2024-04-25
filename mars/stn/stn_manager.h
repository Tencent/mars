//
// Created by Cpan on 2022/5/27.
//

#ifndef MMNET_STN_MANAGER_H
#define MMNET_STN_MANAGER_H

#endif  // MMNET_STN_MANAGER_H

#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "mars/stn/stn.h"

namespace mars {

namespace comm {
class ProxyInfo;
}

namespace stn {

class NetCore;
class StnCallbackBridge;

class StnManager : public mars::boot::BaseManager {
 public:
    explicit StnManager(mars::boot::Context* context);
    ~StnManager() override;
    std::string GetName() override;

 private:
    mars::boot::Context* context_;

 public:
    void OnInitConfigBeforeOnCreate(const int _packer_encoder_version);
    void OnCreate();
    void OnDestroy();
    void OnSingalCrash(int _sig);
    void OnExceptionCrash();
    void OnNetworkChange(void (*pre_change)());
    void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv);
#ifdef ANDROID
    void OnAlarm(int64_t _id);
#endif

 public:
    // 外部接口
    void SetCallback(Callback* const callback);
    void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge);
    StnCallbackBridge* GetStnCallbackBridge();

 public:
    // #################### stn.h Callback ####################
    bool MakesureAuthed(const std::string& _host, const std::string& _user_id);

    // 流量统计
    void TrafficData(ssize_t _send, ssize_t _recv);

    // 底层询问上层该host对应的ip列表
    std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host);
    // 网络层收到push消息回调
    void OnPush(const std::string& _channel_id,
                uint32_t _cmdid,
                uint32_t _taskid,
                const AutoBuffer& _body,
                const AutoBuffer& _extend);
    // 底层获取task要发送的数据
    bool Req2Buf(uint32_t taskid,
                 void* const user_context,
                 const std::string& _user_id,
                 AutoBuffer& outbuffer,
                 AutoBuffer& extend,
                 int& error_code,
                 int channel_select,
                 const std::string& host);
    // 底层回包返回给上层解析
    int Buf2Resp(uint32_t taskid,
                 void* const user_context,
                 const std::string& _user_id,
                 const AutoBuffer& inbuffer,
                 const AutoBuffer& extend,
                 int& error_code,
                 int channel_select);
    // 任务执行结束
    int OnTaskEnd(uint32_t taskid,
                  void* const user_context,
                  const std::string& _user_id,
                  int error_type,
                  int error_code,
                  const ConnectProfile& _profile);

    // 上报网络连接状态
    void ReportConnectStatus(int status, int longlink_status);
    void ReportConnectNetType(ConnNetType conn_type);

    void OnLongLinkNetworkError(::mars::stn::ErrCmdType _err_type,
                                int _err_code,
                                const std::string& _ip,
                                uint16_t _port);
    void OnShortLinkNetworkError(::mars::stn::ErrCmdType _err_type,
                                 int _err_code,
                                 const std::string& _ip,
                                 const std::string& _host,
                                 uint16_t _port);

    void OnLongLinkStatusChange(int _status);
    // 长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
    int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                       AutoBuffer& identify_buffer,
                                       AutoBuffer& buffer_hash,
                                       int32_t& cmdid);
    // 长连信令校验回包
    bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                    const AutoBuffer& response_buffer,
                                    const AutoBuffer& identify_buffer_hash);

    void RequestSync();
    // 验证是否已登录

    // 底层询问上层http网络检查的域名列表
    void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);
    // 底层向上层上报cgi执行结果
    void ReportTaskProfile(const ::mars::stn::TaskProfile& _task_profile);
    // 底层通知上层cgi命中限制
    void ReportTaskLimited(int _check_type, const ::mars::stn::Task& _task, unsigned int& _param);

    //.生成taskid.
    uint32_t GenTaskID();

 public:
    std::function<void(const ::mars::stn::DnsProfile& _dns_profile)> ReportDnsProfileFunc;

 private:
    // 底层上报域名dns结果
    void __ReportDnsProfile(const ::mars::stn::DnsProfile& _dns_profile);
    // #################### end stn.h ####################

    // #################### end stn_logci.h ####################

 public:
    // extern void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports) ;
    // extern void SetShortlinkSvrAddr(const uint16_t port) ;

    // 'host' will be ignored when 'debugip' is not empty.
    void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports, const std::string& debugip);

    // 'task.host' will be ignored when 'debugip' is not empty.
    void SetShortlinkSvrAddr(const uint16_t port, const std::string& debugip);

    // setting debug ip address for the corresponding host
    void SetDebugIP(const std::string& host, const std::string& ip);

    // setting backup iplist for the corresponding host
    // if debugip is not empty, iplist will be ignored.
    // iplist will be used when newdns/dns ip is not available.
    void SetBackupIPs(const std::string& host, const std::vector<std::string>& iplist);

    // async function.
    bool StartTask(const Task& task);

    // sync function
    void StopTask(uint32_t taskid);

    // check whether task's list has the task or not.
    bool HasTask(uint32_t taskid);

    // reconnect longlink and redo all task
    // when you change svr ip, you must call this function.
    void RedoTasks();

    // touch tasks loop. Generally, invoke it after autoauth successfully.
    void TouchTasks();

    //    need longlink channel
    void DisableLongLink();

    // stop and clear all task
    void ClearTasks();

    // the same as ClearTasks(), but also reinitialize network.
    void Reset();

    void ResetAndInitEncoderVersion(int _encoder_version);

    // setting signalling's parameters.
    // if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    void SetSignallingStrategy(long _period, long _keeptime);

    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    void KeepSignalling();

    void StopSignalling();

    // connect quickly if longlink is not connected.
    void MakesureLonglinkConnected();

    bool LongLinkIsConnected();

    bool ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info,
                          const std::string& _test_host,
                          const std::vector<std::string>& _hardcode_ips);

    // noop is used to keep longlink conected
    // get noop taskid
    uint32_t getNoopTaskID();

    //===----------------------------------------------------------------------===//
    ///
    /// Support multi longlinks for mars
    /// these APIs are subject to change in developing
    ///
    //===----------------------------------------------------------------------===//
    void CreateLonglink_ext(LonglinkConfig& _config);
    void DestroyLonglink_ext(const std::string& name);
    std::vector<std::string> GetAllLonglink_ext();
    void MarkMainLonglink_ext(const std::string& name);

    bool LongLinkIsConnected_ext(const std::string& name);
    void MakesureLonglinkConnected_ext(const std::string& name);

    // #################### end stn_logci.h ####################

    const std::vector<std::string>& GetLongLinkHosts();
    void SetLongLink(const std::vector<std::string>& _hosts,
                     const std::vector<uint16_t>& _ports,
                     const std::string& _debugip);
    void SetShortlink(const uint16_t _port, const std::string& _debugip);

    // Add for Android
    //   ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select) ;
    //   void AddServerBan(const std::string& _ip) ;
    //   void DisconnectLongLinkByTaskId(uint32_t _taskid, LongLinkErrCode::TDisconnectInternalCode _code) ;
    // end Add for Android

 private:
    Callback* callback_ = NULL;
    StnCallbackBridge* callback_bridge_ = NULL;
    std::shared_ptr<NetCore> net_core_ = NULL;
    int packer_encoder_version_;
};  // StnManager

}  // namespace stn
}  // namespace mars
