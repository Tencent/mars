//
// Created by Changpeng Pan on 2022/12/1.
//

#ifndef MMNET_BASE_STN_MANAGER_H
#define MMNET_BASE_STN_MANAGER_H

#include "base_manager.h"
#include "mars/stn/stn.h"
#include "mars/stn/stn_callback_bridge.h"

namespace mars {
namespace stn {

class BaseStnManager : public boot::BaseManager {
 public:
    virtual ~BaseStnManager() {
    }
    virtual void Init() = 0;
    virtual void UnInit() = 0;

 public:
    virtual void OnInitConfigBeforeOnCreate(const int _packer_encoder_version) = 0;
    virtual void OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void OnSingalCrash(int _sig) = 0;
    virtual void OnExceptionCrash() = 0;
    virtual void OnNetworkChange(void (*pre_change)()) = 0;
    virtual void OnNetworkDataChange(const char* _tag, ssize_t _send, ssize_t _recv) = 0;
#ifdef ANDROID
    virtual void OnAlarm(int64_t _id) = 0;
#endif

 public:
    // 外部接口
    virtual void SetCallback(Callback* const callback) = 0;
    virtual void SetStnCallbackBridge(StnCallbackBridge* _callback_bridge) = 0;
    virtual StnCallbackBridge* GetStnCallbackBridge() = 0;

 public:
    // #################### stn.h Callback ####################
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id) = 0;

    // 流量统计
    virtual void TrafficData(ssize_t _send, ssize_t _recv) = 0;

    // 底层询问上层该host对应的ip列表
    virtual std::vector<std::string> OnNewDns(const std::string& _host, bool _longlink_host) = 0;
    // 网络层收到push消息回调
    virtual void OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend) = 0;
    // 底层获取task要发送的数据
    virtual bool Req2Buf(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         const int channel_select,
                         const std::string& host) = 0;
    // 底层回包返回给上层解析
    virtual int Buf2Resp(uint32_t taskid,
                         void* const user_context,
                         const std::string& _user_id,
                         const AutoBuffer& inbuffer,
                         const AutoBuffer& extend,
                         int& error_code,
                         const int channel_select) = 0;
    // 任务执行结束
    virtual int OnTaskEnd(uint32_t taskid,
                          void* const user_context,
                          const std::string& _user_id,
                          int error_type,
                          int error_code,
                          const ConnectProfile& _profile) = 0;

    // 上报网络连接状态
    virtual void ReportConnectStatus(int status, int longlink_status) = 0;
    virtual void OnLongLinkNetworkError(::mars::stn::ErrCmdType _err_type,
                                        int _err_code,
                                        const std::string& _ip,
                                        uint16_t _port) = 0;
    virtual void OnShortLinkNetworkError(::mars::stn::ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port) = 0;

    virtual void OnLongLinkStatusChange(int _status) = 0;
    // 长连信令校验 ECHECK_NOW = 0, ECHECK_NEVER = 1, ECHECK_NEXT = 2
    virtual int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& identify_buffer,
                                               AutoBuffer& buffer_hash,
                                               int32_t& cmdid) = 0;
    // 长连信令校验回包
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& response_buffer,
                                            const AutoBuffer& identify_buffer_hash) = 0;

    virtual void RequestSync() = 0;
    // 验证是否已登录

    // 底层询问上层http网络检查的域名列表
    virtual void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) = 0;
    // 底层向上层上报cgi执行结果
    virtual void ReportTaskProfile(const ::mars::stn::TaskProfile& _task_profile) = 0;
    // 底层通知上层cgi命中限制
    virtual void ReportTaskLimited(int _check_type, const ::mars::stn::Task& _task, unsigned int& _param) = 0;
    // 底层上报域名dns结果
    virtual void ReportDnsProfile(const ::mars::stn::DnsProfile& _dns_profile) = 0;
    //.生成taskid.
    virtual uint32_t GenTaskID() = 0;

    // #################### end stn.h ####################

    // #################### end stn_logci.h ####################

 public:
    // extern void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports) = 0;
    // extern void SetShortlinkSvrAddr(const uint16_t port) = 0;

    // 'host' will be ignored when 'debugip' is not empty.
    virtual void SetLonglinkSvrAddr(const std::string& host,
                                    const std::vector<uint16_t> ports,
                                    const std::string& debugip) = 0;

    // 'task.host' will be ignored when 'debugip' is not empty.
    virtual void SetShortlinkSvrAddr(const uint16_t port, const std::string& debugip) = 0;

    // setting debug ip address for the corresponding host
    virtual void SetDebugIP(const std::string& host, const std::string& ip) = 0;

    // setting backup iplist for the corresponding host
    // if debugip is not empty, iplist will be ignored.
    // iplist will be used when newdns/dns ip is not available.
    virtual void SetBackupIPs(const std::string& host, const std::vector<std::string>& iplist) = 0;

    // async function.
    virtual bool StartTask(const Task& task) = 0;

    // sync function
    virtual void StopTask(uint32_t taskid) = 0;

    // check whether task's list has the task or not.
    virtual bool HasTask(uint32_t taskid) = 0;

    // reconnect longlink and redo all task
    // when you change svr ip, you must call this function.
    virtual void RedoTasks() = 0;

    // touch tasks loop. Generally, invoke it after autoauth successfully.
    virtual void TouchTasks() = 0;

    //    need longlink channel
    virtual void DisableLongLink() = 0;

    // stop and clear all task
    virtual void ClearTasks() = 0;

    // the same as ClearTasks(), but also reinitialize network.
    virtual void Reset() = 0;

    virtual void ResetAndInitEncoderVersion(int _encoder_version) = 0;

    // setting signalling's parameters.
    // if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    virtual void SetSignallingStrategy(long _period, long _keeptime) = 0;

    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    virtual void KeepSignalling() = 0;

    virtual void StopSignalling() = 0;

    // connect quickly if longlink is not connected.
    virtual void MakesureLonglinkConnected() = 0;

    virtual bool LongLinkIsConnected() = 0;

    virtual bool ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info,
                                  const std::string& _test_host,
                                  const std::vector<std::string>& _hardcode_ips) = 0;

    // noop is used to keep longlink conected
    // get noop taskid
    virtual uint32_t getNoopTaskID() = 0;

    //===----------------------------------------------------------------------===//
    ///
    /// Support multi longlinks for mars
    /// these APIs are subject to change in developing
    ///
    //===----------------------------------------------------------------------===//
    virtual void CreateLonglink_ext(LonglinkConfig& _config) = 0;
    virtual void DestroyLonglink_ext(const std::string& name) = 0;
    virtual std::vector<std::string> GetAllLonglink_ext() = 0;
    virtual void MarkMainLonglink_ext(const std::string& name) = 0;

    virtual bool LongLinkIsConnected_ext(const std::string& name) = 0;
    virtual void MakesureLonglinkConnected_ext(const std::string& name) = 0;

    // #################### end stn_logci.h ####################

    virtual const std::vector<std::string>& GetLongLinkHosts() = 0;
    virtual void SetLongLink(const std::vector<std::string>& _hosts,
                             const std::vector<uint16_t>& _ports,
                             const std::string& _debugip) = 0;
    virtual void SetShortlink(const uint16_t _port, const std::string& _debugip) = 0;
    // Add for Android
    //   virtual ConnectProfile GetConnectProfile(uint32_t _taskid, int _channel_select) = 0;
    //   virtual void AddServerBan(const std::string& _ip) = 0;
    //   virtual void DisconnectLongLinkByTaskId(uint32_t _taskid, longlink::TDisconnectInternalCode _code) = 0;
    // end Add for Android
};

}  // namespace stn
}  // namespace mars
#endif  // MMNET_BASE_STN_MANAGER_H
