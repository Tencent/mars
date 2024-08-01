/*
 * shortlink_with_mmtls.cc
 *
 *  Created on: Jul 15, 2016
 *      Author: elviswu
 */

#include "shortlink_with_mmtls.h"

#include <climits>
#include <functional>
#include <memory>
#include <sstream>

#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/http.h"
#include "mars/comm/platform_comm.h"
#include "mars/comm/socket/block_socket.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/strutil.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/unix/socket/socketselect.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/stn/proto/shortlink_packer.h"
#include "mars/stn/stn.h"

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif

//#include "mars/mm-ext/mmstn_manager.h"
#include "mmtls_lib/client/mmtls_client_channel.h"
#include "mmtls_lib/comm/mmtls_constants.h"
#include "mmtls_lib/comm/mmtls_data_writer.h"
#include "mmtls_lib/comm/mmtls_http_pack.h"
#include "mmtls_lib/comm/mmtls_string.h"
#include "mmtls_lib/comm/mmtls_types.h"
//#include "newdns/newdns_resolver.h"
//#include "weaknet_utils.h"

using namespace mars::stn;
using namespace mars::comm;
using namespace mars::boot;
using namespace http;

// boost::function<void (const std::string& _user_id, std::vector<std::string>& _host_list)>
// ShortLinkWithMMTLS::hosts_filter_; boost::function<void (bool _timeout, struct tcp_info& _info)>
// ShortLinkWithMMTLS::weaknet_report_; boost::function<void (bool _connect_timeout, struct tcp_info& _info)>
// ShortLinkWithMMTLS::add_weaknet_info_;

static unsigned int KBufferSize = 8 * 1024;

static unsigned int kMaxRttEachShortLink = 3;
static const char* kMagicStr = "<html>";

#ifdef WIN32
static bool sg_http_header_host_use_ip = true;
#endif
#define AYNC_HANDLER asyncreg_.Get()

extern void DNSUtilErrorMonitorCB(int _key);

ShortLinkWithMMTLS::ShortLinkWithMMTLS(Context* _context,
                                       MessageQueue::MessageQueue_t _messagequeueid,
                                       std::shared_ptr<NetSource> _netsource,
                                       const Task& _task,
                                       bool _use_proxy,
//                                       std::shared_ptr<NetStat> _net_stat,
                                       std::unique_ptr<SocketOperator> _operator,
                                       std::string tls_group_name)
: ShortLink(_context, _messagequeueid, std::move(_netsource), _task, _use_proxy, std::move(_operator))
, context_(_context)
, mmtls_channel_(nullptr)
, use_mmtls_(context_->GetManager<StnManager>()->IsMMTLSEnabled())
, pack_protocol_(HttpOverMMtls)
, need_all_response_(false)
, debug_host_("")
, timeout_(false)
//, net_stat_(std::move(_net_stat))
, m_tls_group_name_(tls_group_name) {
    dns_util_.GetNewDNS().SetMonitorFunc(boost::bind(&DNSUtilErrorMonitorCB, _1));
    dns_util_.GetDNS().SetMonitorFunc(boost::bind(&DNSUtilErrorMonitorCB, _1));
    memset(mmtls_task_tag_, 0, sizeof(mmtls_task_tag_));
    mmtls_observer_.signal_handshake_completed = [this] {
        OnConnectHandshakeCompleted();
    };
    xdebug2(TSF "use_mmtls_=%_, IsMmtlsEnableFromAuth()=%_",
            use_mmtls_,
            context_->GetManager<StnManager>()->IsMMTLSEnabled());
}

ShortLinkWithMMTLS::~ShortLinkWithMMTLS() {
    xinfo_function(TSF "delete %_", this);
    if (task_.priority >= 0) {
        xdebug_function(TSF "taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);
    }
    __CancelAndWaitWorkerThread();  // cancel in ~ShortLink()
    if (NULL != mmtls_channel_) {
        delete mmtls_channel_;
        mmtls_channel_ = NULL;
    }
}

void ShortLinkWithMMTLS::SendRequest() {
    xdebug_function();
    is_start_req2buf_thread = true;
    req2buf_thread_ = std::make_shared<comm::Thread>(boost::bind(&ShortLink::__Req2Buf, this),
                                                     __GetTheadName(task_.cgi + ".Req2Buf").c_str());
    if (func_host_filter) {
        func_host_filter(task_.user_id, task_.shortlink_host_list);
    } else {
        xwarn2(TSF "func_host_filter is null.");
    }
    thread_.start();
}

void ShortLinkWithMMTLS::SendRequest(AutoBuffer& _buf_req, AutoBuffer& _buffer_extend) {
    xverbose_function();
    xdebug2(XTHIS)(TSF "bufReq.size:%_", _buf_req.Length());
    send_body_.Attach(_buf_req);

    if (func_host_filter) {
        func_host_filter(task_.user_id, task_.shortlink_host_list);
    } else {
        xwarn2(TSF "func_host_filter is null.");
    }

    send_extend_.Attach(_buffer_extend);
    thread_.start();
}

void ShortLinkWithMMTLS::__Run() {
    xmessage2_define(message, TSF "taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);
    if (task_.priority >= 0) {
        xinfo_function(TSF "%_, net:%_, body:%_, long polling: %_",
                       message.String(),
                       getNetInfo(),
                       send_body_.Length(),
                       task_.long_polling);
    } else {
        xinfo2(TSF "%_, net:%_, body:%_, long polling: %_, realtime: %_",
               message.String(),
               getNetInfo(),
               send_body_.Length(),
               task_.long_polling,
               task_.need_realtime_netinfo);
    }

    if (is_start_req2buf_thread) {
        req2buf_thread_->start();
    }

    ConnectProfile conn_profile;
    conn_profile.start_get_network_lab_time = gettickcount();
    if (task_.need_realtime_netinfo) {
        getRealtimeNetLabel(conn_profile.net_type);
    } else {
        getCurrNetLabel(conn_profile.net_type);
    }
    conn_profile.end_get_network_lab_time = gettickcount();
    xdebug2(TSF "get net label use %_ realtime %_",
            (conn_profile.end_get_network_lab_time - conn_profile.start_get_network_lab_time),
            task_.need_realtime_netinfo);
    conn_profile.start_time = ::gettickcount();
    conn_profile.tid = xlogger_tid();
    conn_profile.link_type = Task::kChannelShort;
    conn_profile.task_id = task_.taskid;
    conn_profile.cgi = task_.cgi;
    __UpdateProfile(conn_profile);

    std::shared_ptr<ShortLinkMMTLSProfile> mmtls_profile(new ShortLinkMMTLSProfile());
    conn_profile.extension_ptr = mmtls_profile;
    conn_profile.req_byte_count = send_body_.Length();

    unsigned int mmtls_rtt_index = 0;
    while (mmtls_rtt_index++ < kMaxRttEachShortLink) {
        if (NULL != mmtls_channel_) {
            delete mmtls_channel_;
            mmtls_channel_ = NULL;
            // xassert2(false);
        } else {
            xinfo2_if(!task_.long_polling && task_.priority >= 0,
                      TSF "mmtls_channel_==NULL, mmtls_rtt_index=%_.",
                      mmtls_rtt_index);
        }
        mmtls_observer_.ResetState();
        mmtls_channel_ = new ClientChannel(context_, mmtls_observer_, m_tls_group_name_);

        memset(mmtls_task_tag_, 0, 64);
        snprintf(mmtls_task_tag_, 64, "mmtls:@%p, mmtls_rtt_index=%d", this, mmtls_rtt_index);
        mmtls_observer_.TaskTag(mmtls_task_tag_);

//        if (net_stat_ != nullptr) {
//            net_stat_->OnConectStatus(MarsLinkTypeShort, MarsLinkConnecting);
//        } else {
//            xerror2("net_stat_ is empty");
//        }
        SOCKET fd_socket = __RunConnect(conn_profile);

        if (is_start_req2buf_thread) {
            req2buf_thread_->join();
        }

        if (INVALID_SOCKET == fd_socket) {
            xdebug2(TSF "connect fialed: %_", conn_profile.conn_errcode);
#ifndef WIN32
            struct tcp_info info;
            __PostSocketTcpInfo(true, info);
#endif
            return;
        }
        if (OnSend) {
            OnSend(this);
        }

//        if (net_stat_ != nullptr) {
//            net_stat_->OnConectStatus(MarsLinkTypeShort,
//                                      INVALID_SOCKET == fd_socket ? MarsLinkConnectFailed : MarsLinkConnected);
//        } else {
//            xerror2("net_stat_ is empty");
//        }

        int errtype = 0;
        int errcode = 0;
        __RunReadWrite(fd_socket, errtype, errcode, conn_profile);
#ifdef WIN32
        if (errtype != kEctOK) {
            sg_http_header_host_use_ip = !sg_http_header_host_use_ip;
        }
#endif
        bool should_return = false;
        __HandleMMTLSErrorCode(errtype, errcode, conn_profile, should_return);

        conn_profile.disconn_signal = ::getSignal(::getNetInfo() == kWifi);
        __UpdateProfile(conn_profile);
        if (!is_keep_alive_ || (kEctMMTLSEcdhSucc == errcode && __PackMMtlsOverHttp())) {
            xdebug2(TSF "close socket: %_", __PackMMtlsOverHttp());
            socket_close(fd_socket);
        } else {
            xinfo2_if(task_.priority >= 0, TSF "keep alive, do not close socket:%_", fd_socket);
        }

        if ((kEctMMTLS == errtype) && (kEctMMTLSInitSucc <= errcode && errcode <= kEctMMTLSInvalidFallbackAlert))
            xdebug2(TSF "should_return=%_, errcode=%_(%_)",
                    should_return,
                    errcode,
                    kMMTLSErrCmdCodeStr[errcode - kEctMMTLSInitSucc]);
        else
            xinfo2(TSF "should_return=%_, errtype=%_, errcode=%_", should_return, errtype, errcode);

        if (should_return) {
            return;
        }
    }
    xerror2(TSF "%_, Must somewhere wrong, mmtls_rtt_index=%_, mmtls_profile:%_",
            mmtls_task_tag_,
            mmtls_rtt_index,
            mmtls_profile->ToDetailString());
    mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSTooManyRTTError);
    __UpdateProfile(conn_profile);
    __RunResponseError(kEctFalse, kEctMMTLSTooManyRTTError, conn_profile);
}

bool ShortLinkWithMMTLS::__MakePayload(int& _err_type,
                                       int& _err_code,
                                       AutoBuffer& _out_buff,
                                       ConnectProfile& _conn_profile) {
    _out_buff.Write(send_body_.Ptr(), send_body_.Length());
    return true;
}

bool ShortLinkWithMMTLS::__MakeTlsPayload(int& _err_type,
                                          int& _err_code,
                                          const AutoBuffer& _in_buff,
                                          ConnectProfile& _conn_profile) {
    std::shared_ptr<ShortLinkMMTLSProfile> mmtls_profile(
        std::dynamic_pointer_cast<ShortLinkMMTLSProfile>(_conn_profile.extension_ptr));
    if (!mmtls_observer_.IsTlvEncoded()) {
        if (!__PackMMtlsOverHttp()) {
            mmtls::HttpHandler http_req_handler((const mmtls::byte*)task_.cgi.c_str(),
                                                task_.cgi.size(),
                                                (const mmtls::byte*)_conn_profile.host.c_str(),
                                                _conn_profile.host.size(),
                                                (const mmtls::byte*)_in_buff.Ptr(),
                                                _in_buff.Length());
            mmtls::String tlv_req_buf;
            mmtls::DataWriter writer(tlv_req_buf);
            http_req_handler.Serialize(writer);
            xdebug2(TSF "%_:req_url=%_, req_host=%_", mmtls_task_tag_, task_.cgi, _conn_profile.host);
            mmtls_observer_.GetSendPlaintxt().Reset();
            mmtls_observer_.GetSendPlaintxt().Write(tlv_req_buf.c_str(), tlv_req_buf.size());
            mmtls_observer_.SetTlvEncodeStatus(true);
        } else {
            mmtls_observer_.GetSendPlaintxt().Reset();
            mmtls_observer_.GetSendPlaintxt().Write(_in_buff.Ptr(), _in_buff.Length());
            mmtls_observer_.SetTlvEncodeStatus(false);
        }
    }

    mmtls::String app_data =
        mmtls::String((const char*)mmtls_observer_.GetSendPlaintxt().Ptr(), mmtls_observer_.GetSendPlaintxt().Length());
    xinfo2_if(!task_.long_polling && task_.priority >= 0, TSF "app data length: %_", app_data.size());
    _conn_profile.start_tls_handshake_time = gettickcount();
    int mmtls_ret = mmtls_channel_->Init(&app_data, true);
    if (!debug_host_.empty()) {
        mmtls_channel_->SetServerUrl(debug_host_.c_str());
    } else {
        mmtls_channel_->SetServerUrl(_conn_profile.host.c_str());
    }
    xdebug2(TSF "%_:SetServerUrl:%_", mmtls_task_tag_, _conn_profile.host);

    if (mmtls::OK == mmtls_ret) {
        xdebug2(TSF "%_: mmtls_ret==OK", mmtls_task_tag_);
#ifndef WIN32
        xassert2(mmtls_observer_.IsCiphertxtReady());  // handshake and app_data
#endif
        mmtls_observer_.SetMMTLSState(kAppDataEncrypt);
        mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSInitSucc);
        __UpdateProfile(_conn_profile);
    } else if (mmtls::ERR_APP_DATA_NOT_SENT == mmtls_ret) {
        xdebug2(TSF "%_: mmtls_ret==ERR_APP_DATA_NOT_SENT", mmtls_task_tag_);
#ifndef WIN32
        xassert2(mmtls_observer_.IsCiphertxtReady());  // handshake data only
#endif
        mmtls_observer_.SetMMTLSState(kHandshakeDataEncrypt);
        mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSInitSucc);
        __UpdateProfile(_conn_profile);
    } else {
        xerror2(TSF "%_: ClientChannl Init failed. mmtls_ret=%_", mmtls_task_tag_, mmtls_ret);
        _err_type = kEctMMTLS;
        _err_code = kEctMMTLSInitError;
        return false;
    }

    return true;
}

bool ShortLinkWithMMTLS::__MakeHttpPayload(int& _err_type,
                                           int& _err_code,
                                           AutoBuffer& _out_buff,
                                           const AutoBuffer& _in_buf,
                                           ConnectProfile& _conn_profile) {
    char mmtls_fake_url[64] = {0};
    srand((unsigned int)time(NULL));
    snprintf(mmtls_fake_url, 64, "/mmtls/%08llx", (long long)rand());
    std::string complete_url;
    std::map<std::string, std::string> headers;
#ifdef WIN32
    std::string replaceHost = _conn_profile.host;
    if (kIPSourceProxy == _conn_profile.ip_type) {
        complete_url += "http://";
        complete_url += _conn_profile.host;
    } else {
        replaceHost = !_conn_profile.ip.empty() && sg_http_header_host_use_ip ? _conn_profile.ip : _conn_profile.host;
    }
    complete_url += mmtls_fake_url;
    xinfo2_if(task_.priority >= 0, TSF "%_, complete_url=%_ host=%_", mmtls_task_tag_, complete_url, replaceHost);

    headers["Upgrade"] = "mmtls";
    headers[HeaderFields::KStringHost] = replaceHost;
    headers["X-Online-Host"] = replaceHost;
#else
    if (kIPSourceProxy == _conn_profile.ip_type) {
        complete_url += "http://";
        complete_url += _conn_profile.host;
    }
    if (__PackMMtlsOverHttp()) {
        complete_url = task_.cgi;
    } else {
        complete_url += mmtls_fake_url;
    }
    xdebug2(TSF "%_, complete_url=%_", mmtls_task_tag_, complete_url);

    if (!__PackMMtlsOverHttp()) {
        headers["Upgrade"] = "mmtls";
    }
    headers[HeaderFields::KStringHost] = _conn_profile.host;
    if (!debug_host_.empty()) {
        xdebug2(TSF "debug_host: %_", debug_host_);
        headers[HeaderFields::KStringHost] = debug_host_;  // deug
    }
#endif  // WIN32

    // add user headers
    if (task_.headers.size() > 0) {
        auto iter = task_.headers.begin();
        while (iter != task_.headers.end()) {
            headers[iter->first] = iter->second;
            xinfo2_if(!task_.long_polling && task_.priority >= 0,
                      TSF "user headers:(%_, %_)",
                      iter->first,
                      iter->second);
            iter++;
        }
    }

    shortlink_pack(complete_url, headers, _in_buf, send_extend_, _out_buff, tracker_.get());
    return true;
}

bool ShortLinkWithMMTLS::__PackMMtlsOverHttp() {
    return (pack_protocol_ == MMtlsOverHttp) && (use_mmtls_);
}

bool ShortLinkWithMMTLS::__PackHttp(SOCKET _sock,
                                    int& _err_type,
                                    int& _err_code,
                                    AutoBuffer& _out_buff,
                                    ConnectProfile& _conn_profile) {
    bool ret = __MakeTlsPayload(_err_type, _err_code, send_body_, _conn_profile);
    if (!ret) {
        xerror2(TSF "mmtls error :(%_, %_) with sock %_", _err_type, _err_code, _sock);
        return false;
    }

    ret = __MakeHttpPayload(_err_type, _err_code, _out_buff, mmtls_observer_.GetSendCiphertxt(), _conn_profile);
    if (!ret) {
        xerror2(TSF "http payload error :(%_, %_) with sock %_", _err_type, _err_code, _sock);
        return false;
    }

    return true;
}

bool ShortLinkWithMMTLS::__PackMMtls(SOCKET _sock,
                                     int& _err_type,
                                     int& _err_code,
                                     AutoBuffer& _out_buff,
                                     ConnectProfile& _conn_profile) {
    AutoBuffer httpPayload;
    bool ret = __MakeHttpPayload(_err_type, _err_code, httpPayload, send_body_, _conn_profile);
    if (!ret) {
        xerror2(TSF "http payload error :(%_, %_) with sock %_", _err_type, _err_code, _sock);
        return false;
    }

    ret = __MakeTlsPayload(_err_type, _err_code, httpPayload, _conn_profile);
    if (!ret) {
        xerror2(TSF "mmtls error :(%_, %_) with sock %_", _err_type, _err_code, _sock);
        return false;
    }

    _out_buff.Write(mmtls_observer_.GetSendCiphertxt());
    xdebug2(TSF "send length: %_, %_, %_", httpPayload.Length(), send_body_.Length(), _out_buff.Length());
    return true;
}

void ShortLinkWithMMTLS::__RunReadWrite(SOCKET _sock, int& _err_type, int& _err_code, ConnectProfile& _conn_profile) {
    xmessage2_define(message)(TSF "taskid:%_, cgi:%_, pack protocol:%_,%_ @%_, ",
                              task_.taskid,
                              task_.cgi,
                              task_.protocol,
                              pack_protocol_,
                              this);
    // mmtls
    bool isReused = _conn_profile.is_reused_fd;
    if (isReused) {
        xinfo2_if(task_.priority >= 0, TSF "is:%_", isReused);
    } else {
        _conn_profile.socket_fd = _sock;
    }

    AutoBuffer out_buff;
    bool packRet = false;
    _conn_profile.start_pack_mmtls_time = gettickcount();
    if (__PackMMtlsOverHttp()) {
        _conn_profile.is_pack_mmtls = true;
        xinfo2_if(task_.priority >= 0, TSF "use pack mmtls over http way");
        packRet = __PackMMtls(_sock, _err_type, _err_code, out_buff, _conn_profile);
    } else {
        _conn_profile.is_pack_mmtls = false;
        packRet = __PackHttp(_sock, _err_type, _err_code, out_buff, _conn_profile);
    }
    _conn_profile.end_pack_mmtls_time = gettickcount();
    xdebug2(TSF "pack http/mmtls pack ret %_ time %_",
            packRet,
            (_conn_profile.end_pack_mmtls_time - _conn_profile.start_pack_mmtls_time));
    if (!packRet) {
        xerror2(TSF "pack http error");
        return;
    }

    xverbose2(TSF "out_buf=%_", xlogger_memory_dump(out_buff.Ptr(), out_buff.Length()));
    //~mmtls

    // send request
    xgroup2_define(group_send);
    xinfo2(TSF "task socket send sock:%_, %_ http len:%_, ", _sock, message.String(), out_buff.Length()) >> group_send;
    uint64_t time_now = gettickcount();
    _conn_profile.start_send_packet_time = time_now;
    int send_ret =
        socketOperator_->Send(_sock, (const unsigned char*)out_buff.Ptr(), (unsigned int)out_buff.Length(), _err_code);

//    if (net_stat_) {
//        net_stat_->OnSend(MarsLinkTypeShort, send_ret);
//    }

    _conn_profile.send_request_cost = gettickspan(time_now);
    __UpdateProfile(_conn_profile);

    if (send_ret < 0) {
        xerror2(TSF "Send Request Error, ret:%0, errno:%1, nread:%_, nwrite:%_",
                send_ret,
                socketOperator_->ErrorDesc(_err_code),
                socket_nread(_sock),
                socket_nwrite(_sock))
            >> group_send;
        __RunResponseError(kEctSocket,
                           (_err_code == 0) ? kEctSocketWritenWithNonBlock : _err_code,
                           _conn_profile,
                           true);
        _err_type = kEctSocket;
        _err_code = (_err_code == 0) ? kEctSocketWritenWithNonBlock : _err_code;
        return;
    }

    GetSignalOnNetworkDataChange()(XLOGGER_TAG, (ssize_t)send_ret, 0);

    if (socketOperator_->Breaker().IsBreak()) {
        xwarn2(TSF "Send Request break, sent:%_ nread:%_, nwrite:%_",
               send_ret,
               socket_nread(_sock),
               socket_nwrite(_sock))
            >> group_send;
        _conn_profile.disconn_errtype = kEctCanceld;
        __UpdateProfile(_conn_profile);
        _err_type = kEctCanceld;
        return;
    }

    // mmtls
    bool is_app_data_sent = true;
    // send success
    mmtls_observer_.ClearSendCiphertxt();
    if (kAppDataEncrypt == mmtls_observer_.GetMMTLSState()) {
        mmtls_observer_.SetMMTLSState(kAppDataSent);
        // mmtls_observer_.ClearSendPlaintxt(); maybe psk out of date
    } else if (kHandshakeDataEncrypt == mmtls_observer_.GetMMTLSState()) {
        mmtls_observer_.SetMMTLSState(kHandshakeDataSent);
    } else {
        xerror2(TSF "%_: mmtls_state=%_", mmtls_task_tag_, (int)mmtls_observer_.GetMMTLSState());
        _err_type = kEctMMTLS;
        _err_code = kEctMMTLSInternalStateError;
        xassert2(false);
        return;
    }
    is_app_data_sent = (kAppDataSent == mmtls_observer_.GetMMTLSState());
    xdebug2(TSF "%_:is_app_data_sent=%_, mmtls_state=%_",
            mmtls_task_tag_,
            is_app_data_sent,
            mmtls_observer_.GetMMTLSState());
    //~mmtls

    task_.priority >= 0 ? (xgroup2() << group_send) : (group_send.Clear());

    xgroup2_define(group_close);
    xgroup2_define(group_recv);

    xinfo2(TSF "task socket close sock:%_, ", _sock) >> group_close;
    xinfo2(TSF "task socket recv sock:%_, %_, ", _sock, message.String()) >> group_recv;

    time_now = gettickcount();
    // recv response
    AutoBuffer body;
    AutoBuffer recv_buf;
    AutoBuffer extension;
    int status_code = -1;
    off_t recv_pos = 0;
    MemoryBodyReceiver* receiver = new MemoryBodyReceiver(body);
    http::Parser parser(receiver, true);

    int read_timeout = 5000;
    if (task_.long_polling) {
        xdebug2(TSF "this is long-polling task, read time %_", task_.long_polling_timeout);
        read_timeout = task_.long_polling_timeout;
    }

    struct tcp_info _info;
    bool getsockinfo_successful = false;
    _conn_profile.start_read_packet_time = 0;
    while (true) {
        int recv_ret = socketOperator_->Recv(_sock, recv_buf, KBufferSize, _err_code, read_timeout);
        if (_conn_profile.start_read_packet_time == 0) {
            _conn_profile.start_read_packet_time = gettickcount();
            if (task_.need_realtime_netinfo) {
                getRealtimeNetLabel(_conn_profile.net_type);
            } else {
                getCurrNetLabel(_conn_profile.net_type);
            }
            __UpdateProfile(_conn_profile);
        }

//        if (net_stat_) {
//            net_stat_->OnRecved(MarsLinkTypeShort, recv_ret);
//        }

        read_timeout = 5000;
        if (recv_ret < 0) {
            xerror2(TSF "read block socket return false, error:%0, nread:%_, nwrite:%_",
                    socketOperator_->ErrorDesc(_err_code),
                    socket_nread(_sock),
                    socket_nwrite(_sock))
                >> group_close;
            __RunResponseError(kEctSocket, (_err_code == 0) ? kEctSocketReadOnce : _err_code, _conn_profile, true);
            _err_type = kEctSocket;
            _err_code = (_err_code == 0) ? kEctSocketReadOnce : _err_code;
            break;
        }

        if (socketOperator_->Breaker().IsBreak()) {
            xinfo2(TSF "user cancel, nread:%_, nwrite:%_", socket_nread(_sock), socket_nwrite(_sock)) >> group_close;
            _conn_profile.disconn_errtype = kEctCanceld;
            __UpdateProfile(_conn_profile);
            _err_type = kEctCanceld;
            _err_code = kEctSocketUserBreak;
            break;
        }

        if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == _err_code) {
            xerror2(TSF "read timeout error:(%_,%_), nread:%_, nwrite:%_",
                    _err_code,
                    socketOperator_->ErrorDesc(_err_code),
                    socket_nread(_sock),
                    socket_nwrite(_sock))
                >> group_close;
            //			__OnResponse(kEctSocket, kEctSocketRecvErr, buf_body_, _conn_profile,
            // socket_nwrite(_sock)
            //== 0); 			break;
            continue;
        }
        if (recv_ret == 0) {
            xerror2(TSF "remote disconnect:(%_,%_), nread:%_, nwrite:%_",
                    _err_code,
                    socketOperator_->ErrorDesc(_err_code),
                    socket_nread(_sock),
                    socket_nwrite(_sock))
                >> group_close;
            __RunResponseError(kEctSocket, kEctSocketShutdown, _conn_profile, true);
            _err_type = kEctSocket;
            _err_code = kEctSocketShutdown;
            break;
        }

        if (recv_ret > 0) {
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, 0, (ssize_t)recv_ret);

            xinfo2(TSF "recv len:%_ ", recv_ret) >> group_recv;
            if (kAppDataSent == mmtls_observer_.GetMMTLSState()) {
                if (OnRecv) {
                    OnRecv(this, (unsigned int)(recv_buf.Length() - recv_pos), (unsigned int)recv_buf.Length());
                }
            } else {
                if (OnRecv) {
                    OnRecv(this, 0, 0);
                }
            }
            recv_pos = recv_buf.Pos();
        }

        Parser::TRecvStatus parse_status;
        AutoBuffer decrypted_buf;
        if (__PackMMtlsOverHttp()) {
            int error = 0;
            __HandleMMTLSRecvBuf((const char*)recv_buf.Ptr(recv_buf.Length() - recv_ret),
                                 recv_ret,
                                 _conn_profile,
                                 error,
                                 decrypted_buf);
            xinfo2(TSF "decrype length:%_, buf length:%_, error:%_", decrypted_buf.Length(), recv_ret, error)
                >> group_recv;
            if (error == 0) {
                continue;
            }
            _err_code = error;
            if (error != kEctMMTLSPskSucc || decrypted_buf.Length() == 0) {
                xerror2(TSF "decode mmtls error");
                _err_type = kEctMMTLS;
                break;
            }
            parse_status = parser.Recv(decrypted_buf.Ptr(0), decrypted_buf.Length());
        } else {
            parse_status = parser.Recv(recv_buf.Ptr(recv_buf.Length() - recv_ret), recv_ret);
        }

        if (parser.FirstLineReady()) {
            status_code = parser.Status().StatusCode();
        }

        if (parse_status == http::Parser::kFirstLineError) {
            xerror2(TSF "http head not receive yet,but socket closed, length:%0, nread:%_, nwrite:%_ ",
                    recv_buf.Length(),
                    socket_nread(_sock),
                    socket_nwrite(_sock))
                >> group_close;
            __RunResponseError(kEctHttp, kEctHttpParseStatusLine, _conn_profile, true);
            _err_type = kEctHttp;
            _err_code = kEctHttpParseStatusLine;
            break;
        } else if (parse_status == http::Parser::kHeaderFieldsError) {
            xerror2(TSF "parse http head failed, but socket closed, length:%0, nread:%_, nwrite:%_ ",
                    recv_buf.Length(),
                    socket_nread(_sock),
                    socket_nwrite(_sock))
                >> group_close;
            __RunResponseError(kEctHttp, kEctHttpSplitHttpHeadAndBody, _conn_profile, true);
            _err_type = kEctHttp;
            _err_code = kEctHttpSplitHttpHeadAndBody;
            break;
        } else if (parse_status == http::Parser::kBodyError) {
            xerror2(TSF "content_length_ != buf_body_.Length(), Head:%0, http dump:%1 \n headers size:%2",
                    parser.Fields().ContentLength(),
                    xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()),
                    parser.Fields().GetHeaders().size())
                >> group_close;
            __RunResponseError(kEctHttp, kEctHttpSplitHttpHeadAndBody, _conn_profile, true);
            _err_type = kEctHttp;
            _err_code = kEctHttpSplitHttpHeadAndBody;
            break;
        } else if (parse_status == http::Parser::kEnd) {
            if (is_keep_alive_) {  // parse server keep-alive config
                bool isClose = parser.Fields().IsConnectionClose();
                xwarn2_if(isClose && !task_.long_polling, "request keep-alive, but server return close");
                if (!isClose) {
                    uint32_t timeout = parser.Fields().KeepAliveTimeout();
                    _conn_profile.keepalive_timeout = timeout;
                    _conn_profile.socket_fd = _sock;
                    __UpdateProfile(_conn_profile);
                } else {
                    is_keep_alive_ = false;
                }
            }

            if (status_code != 200) {
                xerror2(TSF "@%0, status_code_ != 200, code:%1, http dump:%2 \n headers size:%3",
                        this,
                        status_code,
                        xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()),
                        parser.Fields().GetHeaders().size())
                    >> group_close;
                __RunResponseError(kEctHttp, status_code, _conn_profile, true);
                _err_type = kEctHttp;
                _err_code = status_code;
            } else {
                xinfo2(TSF "@%0, headers size:%_, ", this, parser.Fields().GetHeaders().size()) >> group_recv;
                if (body.Length() >= strlen(kMagicStr)) {
                    if (0 == memcmp(body.Ptr(), kMagicStr, strlen(kMagicStr))) {
                        xinfo2("http redirect, but http statusCode is 200");
                        xerror2(TSF "@%_, status code:%_, buf size:%_, http dump:%_ \n line:%_, headers:%_",
                                this,
                                status_code,
                                recv_buf.Length(),
                                xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()),
                                parser.Status().ToString(),
                                parser.Fields().ToString());
                        __RunResponseError(kEctHttp, 302, _conn_profile, true);  // treat as 302
                        _err_type = kEctHttp;
                        _err_code = 302;
                        break;
                    }
                } else {
                    xwarn2(TSF "body_length(%_) < strlen(%_)", body.Length(), kMagicStr);
                    xerror2(TSF "@%_, status code:%_, http dump:%_ \n headers size:%_",
                            this,
                            status_code,
                            xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()),
                            parser.Fields().GetHeaders().size());
                }
                _err_type = kEctMMTLS;
                _conn_profile.recv_reponse_cost = gettickspan(time_now);
                _conn_profile.read_packet_finished_time = gettickcount();
                __UpdateProfile(_conn_profile);
                xdebug2(TSF "%_, _err_code=%_, need all byte:%_", mmtls_task_tag_, _err_code, need_all_response_);

#if defined(__ANDROID__) || defined(__APPLE__)
                if (getsocktcpinfo(_sock, &_info) == 0) {
                    getsockinfo_successful = true;
#ifdef __APPLE__
                    _conn_profile.retrans_byte_count = _info.tcpi_txretransmitbytes;
                    _conn_profile.rtt_by_socket = _info.tcpi_rttcur / 1000;
#else
                    _conn_profile.retrans_byte_count = _info.tcpi_total_retrans;
                    _conn_profile.rtt_by_socket = _info.tcpi_rtt / 1000;
#endif
                }
#endif
                if (__PackMMtlsOverHttp()) {
                    xinfo2(TSF "response mmtls over http") >> group_recv;
                    if (kEctMMTLSPskSucc == _err_code) {
                        if (need_all_response_) {
                            __OnResponse(kEctOK, status_code, decrypted_buf, extension, _conn_profile, true);
                        } else {
                            __OnResponse(kEctOK, status_code, body, extension, _conn_profile, true);
                        }
                    }
                } else {
                    AutoBuffer decrypted_buf;
                    uint64_t start_unpack_mmtls_time = gettickcount();
                    __HandleMMTLSRecvBuf((const char*)body.Ptr(),
                                         body.Length(),
                                         _conn_profile,
                                         _err_code,
                                         decrypted_buf);
                    uint64_t end_unpack_mmtls_time = gettickcount();
                    xdebug2(TSF "handle mmtls recv buf time %_", end_unpack_mmtls_time - start_unpack_mmtls_time);
                    if (kEctMMTLSPskSucc == _err_code)
                        __OnResponse(kEctOK, status_code, decrypted_buf, extension, _conn_profile, true);
                }
            }
            break;
        } else {
            xdebug2(TSF "http parser status:%_ ", parse_status);
        }
    }

    if (_conn_profile.local_net_stack == ELocalIPStack_IPv6 && _conn_profile.ip.find(".") != std::string::npos) {
        _conn_profile.ipv6only_but_v4_successful = true;
    }
    if (_conn_profile.local_net_stack == ELocalIPStack_Dual && _conn_profile.ip.find(".") != std::string::npos) {
        _conn_profile.dual_stack_but_use_v4 = true;
    }

    if (_conn_profile.ipv6_connect_failed && _conn_profile.ip.find(".") != std::string::npos) {
        _conn_profile.ipv6_failed_but_v4_successful = true;
    }

    xdebug2(TSF "read with nonblock socket http response, length:%_, ", recv_buf.Length()) >> group_recv;

    task_.priority >= 0 ? (xgroup2() << group_recv) : (group_recv.Clear());
#if defined(__ANDROID__) || defined(__APPLE__)
    if (getsockinfo_successful) {
        char tcp_info_str[1024] = {0};
        xinfo2(TSF "task socket close getsocktcpinfo:%_", tcpinfo2str(&_info, tcp_info_str, sizeof(tcp_info_str)))
            >> group_close;
        __PostSocketTcpInfo(timeout_, _info);
    }
#endif
    xgroup2() << group_close;
}

void ShortLinkWithMMTLS::__HandleMMTLSRecvBuf(const char* _mmtls_recv_buf,
                                              size_t _mmtls_buf_length,
                                              ConnectProfile& _conn_profile,
                                              int& _err_code,
                                              AutoBuffer& _decrypted_recv_buf) {
    // std::shared_ptr<MMTLSProfile> mmtls_profile((MMTLSProfile*)_conn_profile.extension_ptr.get());
    std::shared_ptr<ShortLinkMMTLSProfile> mmtls_profile(
        std::dynamic_pointer_cast<ShortLinkMMTLSProfile>(_conn_profile.extension_ptr));
    size_t size_hint = 0;
    int mmtls_ret = mmtls_channel_->Receive(_mmtls_recv_buf, _mmtls_buf_length, size_hint);
    if (__PackMMtlsOverHttp()) {
        if (mmtls::OK == mmtls_ret && size_hint > 0) {
            return;
        }
    }
    if (mmtls::OK == mmtls_ret && 0 == size_hint) {
        if (kHandshakeDataSent == mmtls_observer_.GetMMTLSState()) {
            mmtls_observer_.SetMMTLSState(kHandshakeDataRecv);
#ifndef WIN32
            xassert2(!mmtls_observer_.IsRevPlaintxtReady());
#endif
            _err_code = kEctMMTLSEcdhSucc;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSEcdhSucc);
            xdebug2(TSF "mmtls handshake success");
        } else if (kAppDataSent == mmtls_observer_.GetMMTLSState()) {
            mmtls_observer_.SetMMTLSState(kAppDataRecv);
#ifndef WIN32
            xassert2(mmtls_observer_.IsRevPlaintxtReady());
#endif
            xdebug2(TSF "%_:mmpkg size=%_, mmpkg=%_",
                    mmtls_task_tag_,
                    mmtls_observer_.GetRecvPlaintxt().Length(),
                    mmtls::StrToHex(mmtls::String((const char*)mmtls_observer_.GetRecvPlaintxt().Ptr(),
                                                  200 > mmtls_observer_.GetRecvPlaintxt().Length()
                                                      ? mmtls_observer_.GetRecvPlaintxt().Length()
                                                      : 200))
                        .c_str());
            xinfo2_if(task_.priority >= 0,
                      TSF "%_:(contain mmtls handshake data)content length=%_ --> real content length=%_",
                      mmtls_task_tag_,
                      _mmtls_buf_length,
                      mmtls_observer_.GetRecvPlaintxt().Length());
            mmtls_observer_.ClearSendPlaintxt();
            _decrypted_recv_buf.Attach(mmtls_observer_.GetRecvPlaintxt());
            _err_code = kEctMMTLSPskSucc;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSPskSucc);
        } else {
            xerror2(TSF "%_: mmtls_state=%_", mmtls_task_tag_, (int)mmtls_observer_.GetMMTLSState());
            _err_code = kEctMMTLSInternalStateError;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSInternalStateError);
            xassert2(false);
            return;
        }
    } else {
        xerror2(TSF "%_: ClientChannl Recv failed. mmtls_ret=%_, size_hint=%_", mmtls_task_tag_, mmtls_ret, size_hint);
        if ((mmtls::ERR_RECEIVE_FATAL_ALERT == mmtls_ret)
            && (mmtls::ALERT_ACCESS_DENIED == mmtls_observer_.AlertType())) {
            xwarn2(TSF "%_: server disaster, reject ecdh hanshake.", mmtls_task_tag_);
            _err_code = kEctMMTLSServerDisasterReject;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSServerDisasterReject);
            return;
        }
        if (kHandshakeDataSent == mmtls_observer_.GetMMTLSState()) {
            _err_code = kEctMMTLSEcdhHandshakeError;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSEcdhHandshakeError);
        } else if (kAppDataSent == mmtls_observer_.GetMMTLSState()) {
            _err_code = kEctMMTLSPskHandshakeError;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSEcdhHandshakeError);
        } else {
            xerror2(TSF "%_: mmtls_state=%_", mmtls_task_tag_, (int)mmtls_observer_.GetMMTLSState());
            xassert2(false);
            _err_code = kEctMMTLSInternalStateError;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSInternalStateError);
            return;
        }

        if (mmtls::ERR_RECEIVE_FATAL_ALERT == mmtls_ret) {
            if (mmtls_channel_->Stat().no_mmtls()) {
                // DispatchMmtlsCtrlInfo(false);
                std::vector<std::string> urls = mmtls_channel_->GetFallbackUrlsVec();
                if (mmtls_channel_->HasCertRegion()) {
                    uint32_t region = mmtls_channel_->GetCertRegion();
                    xinfo2(TSF "receive region from alert: %_", region);
                    __HandleReceivedMMtlsRegion();
                }
                context_->GetManager<StnManager>()->ForbidMMtlsHost(urls);
                use_mmtls_ = context_->GetManager<StnManager>()->IsMMTLSEnabled();
                xwarn2(TSF "%_: receive ALERT_FALLBACK_NO_MMTLS, use_mmtls_=%_, fallback url: %_",
                       mmtls_task_tag_,
                       use_mmtls_,
                       urls.size());
                _err_code = kEctMMTLSValidFallbackAlert;
                mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSValidFallbackAlert);
                return;

            } else if (mmtls::ALERT_FALLBACK_NO_MMTLS == mmtls_observer_.AlertType()) {
                xerror2(TSF "%_: receive ALERT_FALLBACK_NO_MMTLS, but verify error", mmtls_task_tag_);
                _err_code = kEctMMTLSInvalidFallbackAlert;
                mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSInvalidFallbackAlert);
                return;
            }
            if (mmtls::ALERT_UNKNOWN_PSK_IDENTITY == mmtls_observer_.AlertType()) {
#ifndef WIN32
                xassert2(mmtls::ALERT_NONE != mmtls_observer_.AlertType());
                xassert2(kAppDataSent == mmtls_observer_.GetMMTLSState());
#endif
                xwarn2(TSF "%_: server tell access psk out of date", mmtls_task_tag_);
                _err_code = kEctMMTLSPskOutofDate;
                mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSPskOutofDate);
            } else {
                // include ALERT_FALLBACK_NO_MMTLS
                xwarn2(TSF "%_: client recv fatal alert, AlertType=%_", mmtls_task_tag_, mmtls_observer_.AlertType());
                _err_code = kEctMMTLSRecvFatalAlert;
                mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSRecvFatalAlert);
            }
        }

        if (0 != size_hint) {
            xassert2(false);
            _err_code = kEctMMTLSDecryptError;
            mmtls_profile->UpdateMMTLSStateSet(kEctMMTLSDecryptError);
            return;
        }
    }
}
void ShortLinkWithMMTLS::__HandleMMTLSErrorCode(const int& _err_type,
                                                const int& _err_code,
                                                ConnectProfile& _conn_profile,
                                                bool& _should_return) {
    _should_return = false;
    if (kEctMMTLS == _err_type) {
        switch (_err_code) {
            // success
            case kEctMMTLSPskSucc: {
                // already callback in __RunReadWrite()
                _should_return = true;
                break;
            }
            case kEctMMTLSEcdhSucc: {
                // internal state, no need process
                _should_return = false;
                break;
            }

                // error need callback
            case kEctMMTLSInitError:
            case kEctMMTLSValidFallbackAlert:
            case kEctMMTLSInternalStateError: {
                __RunResponseError(kEctFalse, _err_code, _conn_profile);
                _should_return = true;
                break;
            }

            // error no need callback
            case kEctMMTLSServerDisasterReject:
            case kEctMMTLSEcdhHandshakeError:
            case kEctMMTLSPskHandshakeError:
            case kEctMMTLSPskOutofDate:
            case kEctMMTLSRecvFatalAlert:
            case kEctMMTLSDecryptError:
            case kEctMMTLSInvalidFallbackAlert: {
                _should_return = false;
                break;
            }
            default:
                if (kEctMMTLSInitSucc <= _err_code && _err_code <= kEctMMTLSInvalidFallbackAlert)
                    xwarn2(TSF "unexpected _err_code=%_(%_)",
                           _err_code,
                           kMMTLSErrCmdCodeStr[_err_code - kEctMMTLSInitSucc]);
                else
                    xerror2(TSF "unexpected _err_code=%_", _err_code);
                xassert2(false);
                break;
        }  // end switch
    } else {
        xassert2(kEctCanceld == _err_type || kEctSocket == _err_type || kEctHttp == _err_type,
                 TSF "_err_type=%_",
                 _err_type);
        // already callback in __RunReadWrite()
        _should_return = true;
    }  // end if
}

void ShortLinkWithMMTLS::__HandleReceivedMMtlsRegion() {
    uint32_t old_region = context_->GetManager<StnManager>()->GetMMTlsRegion();
    uint32_t current_region = mmtls_channel_->GetCertRegion();
    xinfo2(TSF "on handshake completed: %_, current: %_", old_region, current_region);

    if (OnHandshakeCompleted) {
        OnHandshakeCompleted(current_region, mars::stn::TlsHandshakeFrom::kFromShortLink);
    }
    ConnectProfile profile;
    if (current_region > 0) {
        profile.tls_handshake_success = true;
    }
    if (old_region != current_region) {
        xwarn2(TSF "mmtls region mismatch");
        profile.tls_handshake_mismatch = true;
        __RunResponseError(kEctFalse, kEctHandshakeMisunderstand, profile, false);
    }
    xinfo2(TSF "handshake mode %_", mmtls_channel_->Stat().handshake_mode());
    if (mmtls_channel_->Stat().handshake_mode() != mmtls::HS_MODE_ZERO_RTT_PSK) {
        profile.tls_handshake_successful_time = gettickcount();
        __UpdateProfile(profile);
        __UpdateMMtlsHandshakeProfile(profile);
    } else {
        profile.start_tls_handshake_time = 0;
        profile.tls_handshake_successful_time = 0;
        __UpdateMMtlsHandshakeProfile(profile);
    }
}

void ShortLinkWithMMTLS::OnConnectHandshakeCompleted() {
    __HandleReceivedMMtlsRegion();
}

void ShortLinkWithMMTLS::__UpdateMMtlsHandshakeProfile(ConnectProfile _conn_profile) {
    SYNC2ASYNC_FUNC(std::bind(&ShortLinkWithMMTLS::__UpdateMMtlsHandshakeProfile, this, _conn_profile));
    conn_profile_.tls_handshake_successful_time = _conn_profile.tls_handshake_successful_time;
    conn_profile_.tls_handshake_mismatch = _conn_profile.tls_handshake_mismatch;
    conn_profile_.tls_handshake_success = _conn_profile.tls_handshake_success;
}

void ShortLinkWithMMTLS::__PostSocketTcpInfo(bool _timeout, struct tcp_info _info) {
    xdebug2("__PostSocketTcpInfo 111");
    if (func_add_weak_net_info) {
        func_add_weak_net_info(_timeout, _info);
    } else {
        xwarn2(TSF "mars2 add weaknet info function is empty.");
    }
    if (func_weak_net_report) {
        func_weak_net_report(_timeout, _info);
    }
}

void ShortLinkWithMMTLS::OnNetTimeout() {
    xdebug2("OnNetTimeout");
    timeout_ = true;
}
