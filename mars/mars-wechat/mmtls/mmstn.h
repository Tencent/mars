/*
 * mmstn.h
 *
 *  Created on: 2016年6月1日
 *      Author: shockcao
 */

#ifndef MARS_MM_EXT_MMSTN_H_
#define MARS_MM_EXT_MMSTN_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "mars/sdt/netchecker_profile.h"
#include "mars/stn/stn.h"
namespace mars {
namespace stn {

struct IPPortItem;

struct IdcHostInfo {
    IdcHostInfo() : priority(0) {
    }
    std::string origin;
    std::string substitude;
    int priority;
};

typedef enum {
    DetectConnectOK,
    DetectConnectFail,
    DetectTimeout,
    DetectNotStart,
    DetectEnd,
} DetectStatus;
typedef enum {
    DetectLongLinkEnd,
    DetectShortLinkEnd,
    BaseNetDetectEnd,
    AllDetectEnd,
} DetectStage;

enum {
    kEctEnDecodeGzdecompressErr = -10607,
    kEctEnDecodeParseNeedVerifyStr = -10608,
    kEctEnDecodeDoEcdsaVerifyErr = -10609,
};

enum {
    kErrNewGetdns = -3001,
    kErrDisastertoGetNewDns = -3006,
    kErrIDCDisaster = -3002,
    kErrSvrReject = -3003,
};
enum {
    kMMPKGHeadExtFlagFG = 0x1,
    kMMPKGHeadExtFlagVIP = 0x2,
    kMMPKGHeadExtFlagNewDNS = 0x4,
};

enum MMTLSErrCmdType {
    kEctMMTLS = 9527,
};

// mmtls errcode [-3000, -2985]
enum MMTLSErrCmdCode {
    // success
    kEctMMTLSInitSucc = -3000,  // DON'T insert before this item, just insert after it
    kEctMMTLSPskSucc,
    kEctMMTLSEcdhSucc,

    // error need callback
    kEctMMTLSInitError,
    kEctMMTLSInternalStateError,
    kEctMMTLSTooManyRTTError,

    // error no need callback
    kEctMMTLSServerDisasterReject,
    kEctMMTLSEcdhHandshakeError,
    kEctMMTLSPskHandshakeError,
    kEctMMTLSPskOutofDate,

    // for longlink
    kEctMMTLSSendError,
    kEctMMTLSRecvError,

    kEctMMTLSRecvFatalAlert,

    kEctMMTLSDecryptError,

    kEctMMTLSValidFallbackAlert,
    kEctMMTLSInvalidFallbackAlert,  // DON'T append after this item, just insert before it.
};
const char* const kMMTLSErrCmdCodeStr[] = {
    // success
    "kEctMMTLSInitSucc",
    "kEctMMTLSPskSucc",
    "kEctMMTLSEcdhSucc",

    // error need callback
    "kEctMMTLSInitError",
    "kEctMMTLSInternalStateError",
    "kEctMMTLSTooManyRTTError",

    // error no need callback
    "kEctMMTLSServerDisasterReject",
    "kEctMMTLSEcdhHandshakeError",
    "kEctMMTLSPskHandshakeError",
    "kEctMMTLSPskOutofDate",

    // for longlink
    "kEctMMTLSSendError",
    "kEctMMTLSRecvError",

    "kEctMMTLSRecvFatalAlert",

    "kEctMMTLSDecryptError",

    "kEctMMTLSValidFallbackAlert",
    "kEctMMTLSInvalidFallbackAlert",
};

enum CgiPackProtocol {
    LongLinkProtocol = 1,
    HttpOverMMtls = 2,
    MMtlsOverHttp = 4,
    BothAllRight = 6,
};
enum MMtlsEncoderVersion {
    internal = 1,  // 国内用mmtls证书
    external = 2,  // 国外用的mmtls证书
};

const char* const kMMtlsEncoderName[] = {
    "default",
    "ilink",
};

enum MMtlsEncoderName {
    main = 0,
    ilink = 1,
};

enum ActionResult {
    ACTION_ACK_ASYNC = -100,     //.业务层异步处理，通过调用mars::stn::MMAckActionNotify给到svr.
    ACTION_ACK_OK = 0,           //.succeed
    ACTION_ACK_MALFORMED = 1,    // invalid ctx or data
    ACTION_ACK_UNSUPPORTED = 2,  // unsupported code
    ACTION_ACK_FAILED = 3,       // failed
};

struct LiveSpeedTestResult {
    bool ping_rtt_successful = false;
    bool upload_successful = false;
    bool download_successful = false;
    bool only_upload_done = false;
    float avg_ping_rtt_ = 0;
    float ping_jitter_ = 0;
    int avg_down_speed_ = 0;
    int max_down_speed_ = 0;
    int min_down_speed_ = 0;
    int avg_up_speed_ = 0;
    int max_up_speed_ = 0;
    int min_up_speed_ = 0;
    int task_id_ = 0;
};

class LiveSpeedTestCallback {
 public:
    LiveSpeedTestCallback() = default;
    virtual ~LiveSpeedTestCallback() = default;

 public:
    virtual void OnSpeedTestDone(LiveSpeedTestResult _result) {
    }
    virtual void OnUploadDone(LiveSpeedTestResult _result) {
    }

#ifdef __ANDROID__
 public:
    void AddCallback(void* _cb) {
        jni_callback_ = _cb;
    }
    void* JniCallbackInstance() {
        return jni_callback_;
    }

 private:
    void* jni_callback_;
#endif
};

/*
extern bool IsMMTLSEnabled();

extern std::string GetUserIDCLocale();

extern std::string GetCurLanguage();

extern int64_t GetIntegerExpt(const std::string& exptKey, int def);

extern void MMTrafficData(ssize_t _send, ssize_t _recv);

extern std::vector<std::string> MMOnNewDns(const std::string& _host, bool _longlink_host);

extern void MMRequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist);

extern void MMReportTaskProfile(const TaskProfile& _task_profile);

extern void MMReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param);

extern void MMReportNetCheckResult(const std::vector<mars::sdt::CheckResultProfile>& _check_results);

extern void MMReportDnsProfile(const DnsProfile& _dns_profile);

extern int MMBuf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id, const AutoBuffer&
_inbuffer, const AutoBuffer& extend, int& _error_code, const int _channel_select);

extern void MMOnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port);

extern void MMOnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string&
_host, uint16_t _port);

extern int GetLogIdSubType(unsigned int _logid, const std::string& _extrainfo);
extern void ReportKV(unsigned int _logid, const char* _value, bool _important, bool _reportnow, int type=0);
extern void ReportIDKey( unsigned int _id, unsigned _key, unsigned int _value, bool _important);
extern void ReportGroupIDKey(const std::vector<std::vector<unsigned int> >& _idkey_infos, bool _important);

extern void ReportFlow(int32_t wifi_recv, int32_t wifi_send, int32_t mobile_recv, int32_t mobile_send);

extern void NetworkAnalysisCallBack(DetectStatus _status, DetectStage _stage, bool _is_detect_end, const std::string&
_report_kv_info);

extern void NotifyDisasterInfo(const std::string& _info);

extern void NotifyDisasterNode(const std::string& _disaster_node);

extern int GetXAgreementId();

typedef void (*TaskSwitchChannelNotifyFuncType)(int _from, ErrCmdType _err_type, int _err_code, int task_id, int
_remain_retry_count);

//.网络测速上报.
extern void (*NotifySpeedTestReport)(const std::string _cookie, const uint32_t& _conn_time, const uint32_t&
_conn_retcode, const uint32_t& _trans_time, const int& _trans_retcode, const std::string& _cli_ip, const int _cli_port,
                                     const std::string& _svr_ip, const int _svr_port, const int _local_stack);

extern void MMCreateLonglink_ext(LonglinkConfig& _config);

extern void MMMarkMainLonglink_ext(const std::string& _name);

extern void MMDestroyLonglink_ext(const std::string& _name);
extern void notifyStnWeaknet(bool _weak);
// .网络层收到action notify消息回调, 必须返回 ActionResult 之一.
// .如果返回ACTION_ACK_ASYNC表示业务层异步处理，需要后续调用mars::stn::MMAckActionNotify接口进行回包.
extern int OnActionNotify(const std::string& uuid, uint32_t code, const std::string& data, const std::string& ctx, const
std::string& extradata, uint32_t taskid); extern void DoNetIdRequest(); extern bool IfUseNewNetId(); extern bool
MaybeChinaUser(); extern void NotifyWeakNet(bool _weak_net, uint32_t _current_rtt); #ifdef __ANDROID__ extern int
doCertificateVerify(const std::string& hostname, const std::vector<std::string>& certschain); #endif
*/

// mars2
class MMCallback {
 public:
    virtual ~MMCallback() {
    }

    virtual std::string GetUserLocale() = 0;

    virtual std::string GetCurLanguage() = 0;

    virtual int64_t GetIntegerExpt(const std::string& exptKey, int def) = 0;

    virtual void ReportFlow(int32_t wifi_recv, int32_t wifi_send, int32_t mobile_recv, int32_t mobile_send) = 0;

    virtual int GetLogIdSubType(unsigned int _logid, const std::string& _extrainfo) = 0;

    virtual void ReportKV(unsigned int _logid,
                          const char* _value,
                          bool _important,
                          bool _reportnow,
                          int type,
                          bool _ignore_freq_check) = 0;

    virtual void ReportIDKey(unsigned int _id, unsigned _key, unsigned int _value, bool _important) = 0;

    virtual void ReportGroupIDKey(const std::vector<std::vector<unsigned int> >& _idkey_infos,
                                  bool _important,
                                  bool _ignore_freq_check) = 0;

    virtual int MMBuf2Resp(uint32_t _taskid,
                           void* const _user_context,
                           const std::string& _user_id,
                           const AutoBuffer& _inbuffer,
                           const AutoBuffer& extend,
                           int& _error_code,
                           uint64_t& _flags,
                           const int _channel_select,
                           unsigned short& server_sequence_id) = 0;

    virtual void NetworkAnalysisCallBack(DetectStatus _status,
                                         DetectStage _stage,
                                         bool _is_detect_end,
                                         const std::string& _report_kv_info) = 0;
    virtual void NotifyDisasterInfo(const std::string& _info) = 0;

    //.网络测速上报.
    virtual void NotifySpeedTestReport(const std::string _cookie,
                                       const uint32_t& _conn_time,
                                       const uint32_t& _conn_retcode,
                                       const uint32_t& _trans_time,
                                       const int& _trans_retcode,
                                       const std::string& _cli_ip,
                                       const int _cli_port,
                                       const std::string& _svr_ip,
                                       const int _svr_port,
                                       const int _local_stack) = 0;
    virtual void NotifyDisasterNode(const std::string& _disaster_node) = 0;

    virtual int GetXAgreementId() = 0;

    //网络层收到action notify消息，必须返回ActionResult 值之一
    virtual int OnActionNotify(const std::string& uuid,
                               uint32_t code,
                               const std::string& data,
                               const std::string& ctx,
                               const std::string& extradata,
                               uint32_t taskid) = 0;

    virtual void NotifyDoGetNetId() = 0;
    virtual bool UseNewNetId() = 0;
    virtual bool MaybeChinaUser() = 0;
    virtual void NotifyWeakNet(bool _weak_net, uint32_t _current_rtt) = 0;
    virtual int doCertificateVerify(const std::string& hostname, const std::vector<std::string>& certschain) = 0;
};

typedef void (*TaskSwitchChannelNotifyFuncType)(int _from,
                                                ErrCmdType _err_type,
                                                int _err_code,
                                                int task_id,
                                                int _remain_retry_count);

}  // namespace stn
}  // namespace mars

#endif /* MARS_MM_EXT_MMSTN_H_ */
