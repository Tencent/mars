//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_STN_MANAGER_H
#define MMNET_STN_MANAGER_H

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "mars/stn/stn.h"

//using namespace mars::boot;

namespace mars {

namespace comm {
class ProxyInfo;
}

namespace stn {

class NetCore;
class NetSource;

class StnCallback {
public:
    virtual ~StnCallback() {}
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id) = 0;

    //流量统计
    virtual void TrafficData(ssize_t _send, ssize_t _recv) = 0;

    //底层询问上层该host对应的ip列表
    virtual std::vector<std::string> OnNewDns(const std::string& host, bool _longlink_host) = 0;
    //网络层收到push消息回调
    virtual void OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) = 0;
    //底层获取task要发送的数据
    virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) = 0;
    //底层回包返回给上层解析
    virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id,  const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;
    //任务执行结束
    virtual int  OnTaskEnd(uint32_t _taskid, void* const _user_context, const std::string& _user_id, int _error_type, int _error_code, const CgiProfile& _profile) = 0;


    //上报网络连接状态
    virtual void ReportConnectStatus(int _status, int _longlink_status) = 0;
    virtual void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {}
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {}

    virtual void OnLongLinkStatusChange(int _status) {}
    //长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
    virtual int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid) = 0;
    //长连信令校验回包
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash) = 0;


    virtual void RequestSync() = 0;

    //验证是否已登录
};

class StnManager {
public:
    void OnInitConfigBeforeOnCreate(int _packer_encoder_version);
public:
    StnManager();
    void SetCallback(StnCallback* const callback);


    //extern void SetLonglinkSvrAddr(const std::string& host, const std::vector<uint16_t> ports);
    //extern void SetShortlinkSvrAddr(const uint16_t port);

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

    // stop and clear all task
    void ClearTasks();

    // the same as ClearTasks(), but also reinitialize network.
    void Reset();

    void ResetAndInitEncoderVersion(int _encoder_version);

    //setting signalling's parameters.
    //if you did not call this function, stn will use default value: period:  5s, keeptime: 20s
    void SetSignallingStrategy(long _period, long _keeptime);

    // used to keep longlink active
    // keep signnaling once 'period' and last 'keeptime'
    void KeepSignalling();

    void StopSignalling();

    // connect quickly if longlink is not connected.
    void MakesureLonglinkConnected();

    bool LongLinkIsConnected();

    bool ProxyIsAvailable(const mars::comm::ProxyInfo& _proxy_info, const std::string& _test_host, const std::vector<std::string>& _hardcode_ips);

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

public:
    NetCore* GetNetCore();
    
private:
    StnCallback* callback_;
    NetCore* net_core_;
};

} // namespace stn
} // namespace mars

#endif // MMNET_STN_MANAGER_H
