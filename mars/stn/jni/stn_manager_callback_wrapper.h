//
// Created by Changpeng Pan on 2022/9/14.
//

#ifndef MMNET_STN_MANAGER_CALLBACK_WRAPPER_H
#define MMNET_STN_MANAGER_CALLBACK_WRAPPER_H

#include <jni.h>

#include "mars/stn/stn.h"

namespace mars {
namespace stn {
class StnManagerJniCallback : public Callback {
 public:
    StnManagerJniCallback(JNIEnv* env, jobject callback);
    virtual ~StnManagerJniCallback();

 private:
    jobject callback_inst_;
    jclass callbacks_class_;

 public:
    virtual bool MakesureAuthed(const std::string& _host, const std::string& _user_id) override;
    virtual void TrafficData(ssize_t _send, ssize_t _recv) override;
    virtual std::vector<std::string> OnNewDns(const std::string& host, bool _longlink_host) override;
    virtual void OnPush(const std::string& _channel_id,
                        uint32_t _cmdid,
                        uint32_t _taskid,
                        const AutoBuffer& _body,
                        const AutoBuffer& _extend) override;
    virtual bool Req2Buf(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         AutoBuffer& outbuffer,
                         AutoBuffer& extend,
                         int& error_code,
                         const int channel_select,
                         const std::string& host) override;
    virtual int Buf2Resp(uint32_t _taskid,
                         void* const _user_context,
                         const std::string& _user_id,
                         const AutoBuffer& _inbuffer,
                         const AutoBuffer& _extend,
                         int& _error_code,
                         const int _channel_select) override;
    virtual int OnTaskEnd(uint32_t _taskid,
                          void* const _user_context,
                          const std::string& _user_id,
                          int _error_type,
                          int _error_code,
                          const CgiProfile& _profile) override;
    virtual void ReportConnectStatus(int _status, int _longlink_status) override;
    virtual void ReportConnectNetType(ConnNetType conn_type) override;

    virtual void OnLongLinkNetworkError(ErrCmdType _err_type,
                                        int _err_code,
                                        const std::string& _ip,
                                        uint16_t _port) override;
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type,
                                         int _err_code,
                                         const std::string& _ip,
                                         const std::string& _host,
                                         uint16_t _port) override;
    virtual void OnLongLinkStatusChange(int _status) override;
    virtual int GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                               AutoBuffer& _identify_buffer,
                                               AutoBuffer& _buffer_hash,
                                               int32_t& _cmdid) override;
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                            const AutoBuffer& _response_buffer,
                                            const AutoBuffer& _identify_buffer_hash) override;
    virtual void RequestSync() override;

    virtual void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) override;
    virtual void ReportTaskProfile(const TaskProfile& _task_profile) override;
    virtual void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) override;
    virtual void ReportDnsProfile(const DnsProfile& _dns_profile) override;
};
}  // namespace stn
}  // namespace mars

#endif  // MMNET_STN_MANAGER_CALLBACK_WRAPPER_H
