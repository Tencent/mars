// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 ============================================================================
 Name       : stn.h
 Author     : yerungui
 Created on : 2012-7-18
 ============================================================================
 */

#ifndef NETWORK_SRC_NET_COMM_H_
#define NETWORK_SRC_NET_COMM_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "mars/comm/autobuffer.h"
#include "mars/comm/projdef.h"

namespace mars{
    namespace stn{

#define DEFAULT_LONGLINK_NAME "default-longlink"
#define DEFAULT_LONGLINK_GROUP "default-group"
#define RUNON_MAIN_LONGLINK_NAME "!run-on-main!"
struct TaskProfile;
struct DnsProfile;
struct ConnectProfile;
class LongLinkEncoder;

static const uint32_t kReservedTaskIDStart = 0xFFFFFFF0;

enum PackerEncoderVersion {
    kOld = 1,
    kNew = 2,
};

enum HostRedirectType {
    kFull   = 0,
    kFuzzy  = 1,   //通过 fuzzy
    kPrefix = 2,  //通过 isprefixmatch (这个很可能已经废弃)
    kOther  = 3
};

struct Task {
public:
    //channel type
    static const int kChannelShort = 0x1;
    static const int kChannelLong = 0x2;
    static const int kChannelBoth = 0x3;
    static const int kChannelMinorLong = 0x4;
    static const int kChannelNormal = 0x5;
    static const int kChannelAll = 0x7;

    static const int kChannelNormalStrategy = 0;
    static const int kChannelFastStrategy = 1;
    static const int kChannelDisasterRecoveryStategy = 2;

    static const int kTransportProtocolDefault = 0; // TCP
    static const int kTransportProtocolTCP = 1;     // TCP
    static const int kTransportProtocolQUIC = 2;    // QUIC
    static const int kTransportProtocolMixed = 3;   // TCP or QUIC

    static const int kTaskPriorityHighest = 0;
    static const int kTaskPriority0 = 0;
    static const int kTaskPriority1 = 1;
    static const int kTaskPriority2 = 2;
    static const int kTaskPriority3 = 3;
    static const int kTaskPriorityNormal = 3;
    static const int kTaskPriority4 = 4;
    static const int kTaskPriority5 = 5;
    static const int kTaskPriorityLowest = 5;

    static const uint32_t kInvalidTaskID = 0;
    static const uint32_t kNoopTaskID = 0xFFFFFFFF;
    static const uint32_t kLongLinkIdentifyCheckerTaskID = 0xFFFFFFFE;
    static const uint32_t kSignallingKeeperTaskID = 0xFFFFFFFD;
    static const uint32_t kMinorLonglinkCmdMask = 0xFF000000;


    Task();
    Task(uint32_t _taskid);

    //require
    uint32_t       taskid;
    uint32_t       cmdid;
    uint64_t       channel_id;      // not used
    int32_t        channel_select;
    int32_t        transport_protocol;  // see kTransportProtocol...
    std::string    cgi;    // user

    //optional
    bool    send_only;  // user
    bool    need_authed;  // user
    bool    limit_flow;  // user
    bool    limit_frequency;  // user

    bool        network_status_sensitive;  // user
    int32_t     channel_strategy;
    int32_t     priority;  // user

    int32_t     retry_count;  // user
    int32_t     server_process_cost;  // user
    int32_t     total_timeout;  // user ms
    bool        long_polling;
    int32_t     long_polling_timeout;

    void*       user_context;  // user
    std::string report_arg;  // use for cgi report
    std::string channel_name;   //longlink channel id
    std::string group_name;     //use for select decode method
    std::string user_id;        //use for identify multi users
    int protocol;

    std::map<std::string, std::string> headers;
    std::vector<std::string> shortlink_host_list;   // current using hosts, may be quic host or tcp host
    std::vector<std::string> shortlink_fallback_hostlist;   // for fallback
    std::vector<std::string> longlink_host_list;
    std::vector<std::string> minorlong_host_list;
    std::vector<std::string> quic_host_list;
    int32_t max_minorlinks;

    std::string function;
    std::string cgi_prefix;
    HostRedirectType redirect_type;
};

struct CgiProfile {
    uint64_t start_time = 0;
    uint64_t start_connect_time = 0;
    uint64_t connect_successful_time = 0;
    uint64_t start_tls_handshake_time = 0;
    uint64_t tls_handshake_successful_time = 0;
    uint64_t start_send_packet_time = 0;
    uint64_t start_read_packet_time = 0;
    uint64_t read_packet_finished_time = 0;
    int channel_type = 0;
    int transport_protocol = 0;
    int rtt = 0;
};

struct LonglinkConfig {
public:
    LonglinkConfig(const std::string& _name, const std::string& _group = DEFAULT_LONGLINK_GROUP, bool _isMain = false)
        :name(_name),is_keep_alive(false), group(_group), longlink_encoder(nullptr), isMain(_isMain), dns_func(nullptr), need_tls(true) {}
    bool IsMain() const {
        return isMain;
    }
    std::string     name;   //channel_id
    std::vector<std::string> host_list;
    bool            is_keep_alive;     //if false, reconnect trig by task
    std::string     group;
    LongLinkEncoder* longlink_encoder;
    bool            isMain;
	int             link_type = Task::kChannelLong;
    int             packer_encoder_version = PackerEncoderVersion::kOld;
    std::vector<std::string> (*dns_func)(const std::string& _host, bool _longlink_host);
    bool            need_tls;
};

struct QuicParameters{
    bool enable_0rtt = true;
    std::string alpn;
    std::string hostname;
};
struct ShortlinkConfig {
public:
    ShortlinkConfig(bool _use_proxy, bool _use_tls) : use_proxy(_use_proxy), use_tls(_use_tls){}
    bool use_proxy = false;
    bool use_tls = true;
    bool use_quic = false;
    QuicParameters quic;
};

enum TaskFailHandleType {
	kTaskFailHandleNormal = 0,
	kTaskFailHandleNoError = 0,

	kTaskFailHandleDefault = -1,
	kTaskFailHandleRetryAllTasks = -12,
	kTaskFailHandleSessionTimeout = -13,

	kTaskFailHandleTaskEnd = -14,
	kTaskFailHandleTaskTimeout = -15,
    kTaskSlientHandleTaskEnd = -16,
};

//error type
enum ErrCmdType {
	kEctOK = 0,
	kEctFalse = 1,
	kEctDial = 2,
	kEctDns = 3,
	kEctSocket = 4,
	kEctHttp = 5,
	kEctNetMsgXP = 6,
	kEctEnDecode = 7,
	kEctServer = 8,
	kEctLocal = 9,
        kEctCanceld = 10,
};

//error code
enum {
	kEctLocalTaskTimeout = -1,
    kEctLocalTaskRetry = -2,
	kEctLocalStartTaskFail = -3,
	kEctLocalAntiAvalanche = -4,
	kEctLocalChannelSelect = -5,
	kEctLocalNoNet = -6,
    kEctLocalCancel = -7,
    kEctLocalClear = -8,
    kEctLocalReset = -9,
	kEctLocalTaskParam = -12,
	kEctLocalCgiFrequcencyLimit = -13,
	kEctLocalChannelID = -14,
    kEctLocalLongLinkReleased = -15,
    kEctLocalLongLinkUnAvailable = -16,
};

// -600 ~ -500
enum {
    kEctLongFirstPkgTimeout = -500,
    kEctLongPkgPkgTimeout = -501,
    kEctLongReadWriteTimeout = -502,
    kEctLongTaskTimeout = -503,
};

// -600 ~ -500
enum {
    kEctHttpFirstPkgTimeout = -500,
    kEctHttpPkgPkgTimeout = -501,
    kEctHttpReadWriteTimeout = -502,
    kEctHttpLongPollingTimeout = -503,
  //  kEctHttpTaskTimeout = -503,
};

// -20000 ~ -10000
enum {
    kEctSocketNetworkChange = -10086,
    kEctSocketMakeSocketPrepared = -10087,
    kEctSocketWritenWithNonBlock = -10088,
    kEctSocketReadOnce = -10089,
    kEctSocketShutdown = -10090,
    kEctSocketRecvErr = -10091,
    kEctSocketSendErr = -10092,
    kEctSocketNoopTimeout = -10093,
    kEctSocketNoopAlarmTooLate = -10094,
    kEctSocketUserBreak = -10095,
    kEctHandshakeMisunderstand = -10096,

    kEctHttpSplitHttpHeadAndBody = -10194,
    kEctHttpParseStatusLine = -10195,

    kEctNetMsgXPHandleBufferErr = -10504,

    kEctDnsMakeSocketPrepared = -10606,
};

enum NetStatus {
    kNetworkUnkown = -1,
    kNetworkUnavailable = 0,
    kGateWayFailed = 1,
    kServerFailed = 2,
    kConnecting = 3,
    kConnected = 4,
    kServerDown = 5
};

enum IdentifyMode {
    kCheckNow = 0,
    kCheckNext,
    kCheckNever
};

enum IPSourceType {
    kIPSourceNULL = 0,
    kIPSourceDebug,
    kIPSourceDNS,
    kIPSourceNewDns,
    kIPSourceProxy,
    kIPSourceBackup,
};

enum TlsHandshakeFrom {
    kNoHandshaking = -1,
    kFromSetting = 0,
    kFromLongLink  = 1,
    kFromShortLink = 2,
};

enum TimeoutSource {
    kClientDefault = 0,
    kServerDefault = 1,
    kCgiSpecial = 2,
};

//move form longlink

// Note: Never Delete Item!!!Just Add!!!
class LongLinkErrCode {
public:
enum TDisconnectInternalCode {
    kNone = 0,
    kReset = 10000,        // no use
    kRemoteClosed = 10001,
    kUnknownErr = 10002,
    kNoopTimeout = 10003,
    kDecodeError = 10004,
    kUnknownRead = 10005,
    kUnknownWrite = 10006,
    kDecodeErr = 10007,
    kTaskTimeout = 10008,
    kNetworkChange = 10009,
    kIDCChange = 10010,
    kNetworkLost = 10011,
    kSelectError = 10012,
    kPipeError = 10013,
    kHasNewDnsIP = 10014,
    kSelectException = 10015,
    kLinkCheckTimeout = 10016,
    kForceNewGetDns = 10017,
    kLinkCheckError = 10018,
    kTimeCheckSucc = 10019,
    kObjectDestruct = 10020,
    kLinkDetectEnd = 10021,
};
};
//move form longlink

/**网络连接的类型*/
enum ConnNetType {
    kConnNetUnknown = 0,   //unknown
    kConnNetWifi    = 1,   //wifi
    kConnNetCellular= 2,   // cellular network
    kConnNetBindCellular = 3,  //bind cellular network when wifi no conn or weak singel
};

const char* const IPSourceTypeString[] = {
    "NullIP",
    "DebugIP",
    "DNSIP",
    "NewDNSIP",
    "ProxyIP",
    "BackupIP",
};

const char* const ChannelTypeString[] = {
    "",
    "Short",
    "Long",
    "",
    "MinorLong",
    "",
    "",
    "",
};

struct IPPortItem {
    std::string		str_ip;
    uint16_t 		port;
    IPSourceType 	source_type;
    std::string 	str_host;
    int transport_protocol = Task::kTransportProtocolTCP; // tcp or quic?
    bool is_bind_cellular_network;
};

/* mars2

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
*/

//mars2
class Callback
{
 public:
    virtual ~Callback() {}
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
    virtual void ReportConnectNetType(ConnNetType conn_type) = 0;

    virtual void OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {}
    virtual void OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {}

    virtual void OnLongLinkStatusChange(int _status) {}
    //长连信令校验 ECHECK_NOW = 0, ECHECK_NEXT = 1, ECHECK_NEVER = 2
    virtual int  GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid) = 0;
    //长连信令校验回包
    virtual bool OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash) = 0;


    virtual void RequestSync() = 0;

    virtual void RequestNetCheckShortLinkHosts(std::vector<std::string>& _hostlist) = 0;
    virtual void ReportTaskProfile(const TaskProfile& _task_profile) = 0;
    virtual void ReportTaskLimited(int _check_type, const Task& _task, unsigned int& _param) = 0;
    virtual void ReportDnsProfile(const DnsProfile& _dns_profile)= 0;
};

}}
#endif // NETWORK_SRC_NET_COMM_H_
