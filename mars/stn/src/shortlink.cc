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
 * shortlink.cc
 *
 *  Created on: 2012-8-22
 *      Author: zhouzhijie
 */

#include "shortlink.h"

#include <tuple>

#include "boost/bind.hpp"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/complexconnect.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/socket/block_socket.h"
#include "mars/comm/strutil.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/http.h"
#include "mars/comm/platform_comm.h"
#include "mars/app/app.h"
#include "mars/comm/crypt/ibase64.h"
#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/move_wrapper.h"

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif
#include "mars/stn/proto/shortlink_packer.h"

#include "weak_network_logic.h"
#include "tcp_socket_operator.h"
#include "mars/app/app_manager.h"

#define AYNC_HANDLER asyncreg_.Get()
#define STATIC_RETURN_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

using namespace mars::stn;
using namespace mars::app;
using namespace mars::comm;
using namespace http;

static unsigned int KBufferSize = 8 * 1024;

namespace mars{
namespace stn{

bool CheckKeepAlive(const Task& _task) {
    auto iter = _task.headers.begin();
    while(iter != _task.headers.end()) {
        if(iter->first == http::HeaderFields::KStringConnection && iter->second == http::HeaderFields::KStringKeepalive)
            return true;
        iter++;
    }
    return false;
}

class ShortLinkConnectObserver : public MComplexConnect {
  public:
    ShortLinkConnectObserver(ShortLink& _shortlink): shortlink_(_shortlink), rtt_(0), last_err_(-1) {
        memset(ConnectingIndex, 0, sizeof(ConnectingIndex));
    };

    virtual void OnCreated(unsigned int _index, const socket_address& _addr, SOCKET _socket) {}
    virtual void OnConnect(unsigned int _index, const socket_address& _addr, SOCKET _socket) {
        ConnectingIndex[_index] = 1;
    }
    virtual void OnConnected(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _error, int _rtt) {
        ConnectingIndex[_index] = 0;

        if (0 != _error) {
//            xassert2(shortlink_.func_network_report);

            if (_index < shortlink_.Profile().ip_items.size() && shortlink_.func_network_report)
                shortlink_.func_network_report(__LINE__, kEctSocket, _error, _addr.ip(), shortlink_.Profile().ip_items[_index].str_host, _addr.port());
        }

        if (last_err_ != 0) {
            last_err_ = _error;
            rtt_ = _rtt;
        }
    }

    int LastErrorCode() const {return last_err_;}
    int Rtt() const {return rtt_;}

    char ConnectingIndex[32];

  private:
    ShortLinkConnectObserver(const ShortLinkConnectObserver&);
    ShortLinkConnectObserver& operator=(const ShortLinkConnectObserver&);

  private:
    ShortLink& shortlink_;
    int rtt_;
    int last_err_;
};

}}

namespace internal{
std::string threadName(const std::string& fullcgi){
    auto pos = fullcgi.find_last_of("/");
    if (pos != std::string::npos){
        return fullcgi.substr(pos + 1, 32) + "@shortlink";
    }
    return fullcgi + "@shortlink";
}
};

///////////////////////////////////////////////////////////////////////////////////////
ShortLink::ShortLink(boot::Context* _context, MessageQueue::MessageQueue_t _messagequeueid, std::shared_ptr<NetSource> _netsource, const Task& _task, bool _use_proxy, std::unique_ptr<SocketOperator> _operator)
    : context_(_context)
        , asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
	, net_source_(_netsource)
	, socketOperator_(_operator == nullptr ? std::make_unique<TcpSocketOperator>(std::make_shared<ShortLinkConnectObserver>(*this)) : std::move(_operator))
	, task_(_task)
	, thread_(boost::bind(&ShortLink::__Run, this), internal::threadName(_task.cgi).c_str())
    , dns_util_(context_)
    , use_proxy_(_use_proxy)
    , tracker_(shortlink_tracker::Create())
    , is_keep_alive_(CheckKeepAlive(_task))
    {
    xinfo2(TSF"%_, handler:(%_,%_), long polling: %_ ",this, asyncreg_.Get().queue, asyncreg_.Get().seq, _task.long_polling);
}

ShortLink::~ShortLink() {
    xinfo2("delete %p", this);
    if (task_.priority >= 0) {
        xdebug_function(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);
    }
    __CancelAndWaitWorkerThread();
    asyncreg_.CancelAndWait();
    dns_util_.Cancel();
}

void ShortLink::SendRequest(AutoBuffer& _buf_req, AutoBuffer& _buffer_extend) {
    xverbose_function();
    xdebug2(XTHIS)(TSF"bufReq.size:%_", _buf_req.Length());
    send_body_.Attach(_buf_req);
    send_extend_.Attach(_buffer_extend);
    thread_.start();
}

void ShortLink::__Run() {
    xmessage2_define(message, TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);
    xinfo_function(TSF"%_, net:%_", message.String(), getNetInfo());

    ConnectProfile conn_profile;
    int type = getCurrNetLabel(conn_profile.net_type);
    if (type == kMobile){
        conn_profile.ispcode = strtoll(conn_profile.net_type.c_str(), nullptr, 10);
    }
    conn_profile.nettype_for_report = mars::comm::getNetTypeForStatistics();
    conn_profile.start_time = ::gettickcount();
    conn_profile.tid = xlogger_tid();
    conn_profile.link_type = Task::kChannelShort;
    conn_profile.task_id = task_.taskid;
    __UpdateProfile(conn_profile);

    SOCKET fd_socket = __RunConnect(conn_profile);

    if (INVALID_SOCKET == fd_socket) return;
    if (OnSend) {
        OnSend(this);
    } else {
        xwarn2(TSF"OnSend NULL.");
    }
    int errtype = 0;
    int errcode = 0;
    __RunReadWrite(fd_socket, errtype, errcode, conn_profile);

    conn_profile.disconn_signal = ::getSignal(::getNetInfo() == kWifi);
    __UpdateProfile(conn_profile);
    
    if(!is_keep_alive_) {
        xinfo2(TSF"taskid:%_ sock %_ closed.", task_.taskid, fd_socket);
        socketOperator_->Close(fd_socket);
    } else {
        xinfo2(TSF"keep alive, do not close socket:%_", fd_socket);
    }
}

SOCKET ShortLink::__RunConnect(ConnectProfile& _conn_profile) {
    xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);

    _conn_profile.dns_time = ::gettickcount();
    __UpdateProfile(_conn_profile);

    if (!task_.shortlink_host_list.empty()){
        _conn_profile.host = task_.shortlink_host_list.front();
    }
    
    if (use_proxy_) {
        _conn_profile.proxy_info = context_->GetManager<AppManager>()->GetProxyInfo(_conn_profile.host);
    }
    
    bool use_proxy = _conn_profile.proxy_info.IsAddressValid();
    TLocalIPStack local_stack = local_ipstack_detect();
    bool isnat64 = local_stack == ELocalIPStack_IPv6;
    _conn_profile.local_net_stack = local_stack;
    
    //
    if (outter_vec_addr_.empty()){
        net_source_->GetShortLinkItems(task_.shortlink_host_list, _conn_profile.ip_items, dns_util_, _conn_profile.cgi);
    }else{
        //.如果有外部ip则直接使用，比如newdns.
        _conn_profile.ip_items = outter_vec_addr_;
    }
    
    if (_conn_profile.ip_items.empty()){
        xerror2(TSF"task socket connect fail %_ ipitems empty", message.String());
        __RunResponseError(kEctDns, kEctDnsMakeSocketPrepared, _conn_profile, false);
        return INVALID_SOCKET;
    }
    
    //.如果有debugip则不走代理逻辑.
    if (use_proxy && _conn_profile.ip_items.front().source_type == kIPSourceDebug){
        xwarn2(TSF"forbid proxy when debugip present.");
        use_proxy = false;
    }
    
    if (use_proxy && mars::comm::kProxyHttp == _conn_profile.proxy_info.type && net_source_->GetShortLinkDebugIP().empty()) {
        _conn_profile.ip = _conn_profile.proxy_info.ip;
        _conn_profile.port = _conn_profile.proxy_info.port;
    	_conn_profile.ip_type = kIPSourceProxy;
        IPPortItem item = {_conn_profile.ip, net_source_->GetShortLinkPort(), _conn_profile.ip_type, _conn_profile.host};
        //.如果是http代理，则把代理地址插到最前面.
        _conn_profile.ip_items.insert(_conn_profile.ip_items.begin(), item);
        __UpdateProfile(_conn_profile);
    } else {
        if (socketOperator_->Protocol() == Task::kTransportProtocolQUIC){
            xassert2(!use_proxy);
            for (auto& ip : _conn_profile.ip_items){
                ip.transport_protocol = Task::kTransportProtocolQUIC;
            }
        }
        
        _conn_profile.host      = _conn_profile.ip_items.front().str_host;
        _conn_profile.ip_type   = _conn_profile.ip_items.front().source_type;
        _conn_profile.ip        = _conn_profile.ip_items.front().str_ip;
        _conn_profile.port      = _conn_profile.ip_items.front().port;
        __UpdateProfile(_conn_profile);
    }
    
	std::string proxy_ip;
    if (use_proxy && mars::comm::kProxyNone != _conn_profile.proxy_info.type) {
		std::vector<std::string> proxy_ips;
        if (_conn_profile.proxy_info.ip.empty() && !_conn_profile.proxy_info.host.empty()) {
            if (!dns_util_.GetDNS().GetHostByName(_conn_profile.proxy_info.host, proxy_ips) || proxy_ips.empty()) {
                xwarn2(TSF"dns %_ error", _conn_profile.proxy_info.host);
                return INVALID_SOCKET;
            }
			proxy_ip = proxy_ips.front();
        } else {
			proxy_ip = _conn_profile.proxy_info.ip;
        }
    }
    
    std::vector<socket_address> vecaddr;
    if (use_proxy && mars::comm::kProxyHttp == _conn_profile.proxy_info.type) {
        vecaddr.push_back(socket_address(proxy_ip.c_str(), _conn_profile.proxy_info.port, _conn_profile.is_bind_cellular_network).v4tov6_address(local_stack));
    } else {
        for (size_t i = 0; i < _conn_profile.ip_items.size(); ++i) {
            if (!use_proxy || mars::comm::kProxyNone == _conn_profile.proxy_info.type) {
                vecaddr.push_back(socket_address(_conn_profile.ip_items[i].str_ip.c_str(), _conn_profile.ip_items[i].port, _conn_profile.is_bind_cellular_network).v4tov6_address(local_stack));
            } else {
                vecaddr.push_back(socket_address(_conn_profile.ip_items[i].str_ip.c_str(), _conn_profile.ip_items[i].port, _conn_profile.is_bind_cellular_network));
            }
        }
    }
    
	socket_address* proxy_addr = NULL;
    if (use_proxy && (mars::comm::kProxyHttpTunel == _conn_profile.proxy_info.type || mars::comm::kProxySocks5 == _conn_profile.proxy_info.type)) {
		proxy_addr = &((new socket_address(proxy_ip.c_str(), _conn_profile.proxy_info.port))->v4tov6_address(local_stack));
        _conn_profile.ip_type = kIPSourceProxy;
    }

    xinfo2_if(task_.priority >= 0, TSF"task socket dns sock %_ proxy:%_, host:%_, ip list:%_, is_keep_alive:%_", message.String(), kIPSourceProxy == _conn_profile.ip_type, _conn_profile.host, net_source_->DumpTable(_conn_profile.ip_items), is_keep_alive_);

    if (vecaddr.empty()) {
        xerror2(TSF"task socket connect fail %_ vecaddr empty", message.String());
        __RunResponseError(kEctDns, kEctDnsMakeSocketPrepared, _conn_profile, false);
        delete proxy_addr;
        return INVALID_SOCKET;
    }
    
    //
    _conn_profile.host = _conn_profile.ip_items[0].str_host;
    _conn_profile.ip_type = _conn_profile.ip_items[0].source_type;
    _conn_profile.ip = _conn_profile.ip_items[0].str_ip;
    _conn_profile.port = _conn_profile.ip_items[0].port;
    _conn_profile.nat64 = isnat64;
    _conn_profile.dns_endtime = ::gettickcount();
    _conn_profile.transport_protocol = socketOperator_->Protocol();
    _conn_profile.closefunc = socketOperator_->GetCloseFunction();
    _conn_profile.createstream_func = socketOperator_->GetCreateStreamFunc();
    _conn_profile.issubstream_func = socketOperator_->GetIsSubStreamFunc();
    static_assert(!std::is_member_function_pointer<decltype(_conn_profile.closefunc)>::value, "must static or global function.");
    static_assert(!std::is_member_function_pointer<decltype(_conn_profile.createstream_func)>::value, "must static or global function.");
    static_assert(!std::is_member_function_pointer<decltype(_conn_profile.issubstream_func)>::value, "must static or global function.");
    int type = getCurrNetLabel(_conn_profile.net_type);
    if (type == kMobile){
        _conn_profile.ispcode = strtoll(_conn_profile.net_type.c_str(), nullptr, 10);
    }
    _conn_profile.nettype_for_report = mars::comm::getNetTypeForStatistics();
    __UpdateProfile(_conn_profile);

    if(_conn_profile.ip_type != kIPSourceProxy && is_keep_alive_) {
        for (size_t i = 0; i < _conn_profile.ip_items.size(); ++i) {
            int fd = GetCacheSocket(_conn_profile.ip_items[i]);
            if(fd != INVALID_SOCKET) {
                _conn_profile.conn_rtt = 0;
                _conn_profile.ip_index = i;
                _conn_profile.conn_cost = 0;
                _conn_profile.host = _conn_profile.ip_items[i].str_host;
                _conn_profile.ip_type = _conn_profile.ip_items[i].source_type;
                _conn_profile.ip = _conn_profile.ip_items[i].str_ip;
                _conn_profile.port = _conn_profile.ip_items[i].port;
                _conn_profile.transport_protocol = _conn_profile.ip_items[i].transport_protocol;
                _conn_profile.conn_time = gettickcount();
                _conn_profile.start_connect_time = _conn_profile.conn_time;
                _conn_profile.connect_successful_time = _conn_profile.conn_time;
                _conn_profile.local_ip = socket_address::getsockname(fd).ip();
                _conn_profile.local_port = socket_address::getsockname(fd).port();
                _conn_profile.socket_fd = fd;
                _conn_profile.is_reused_fd = true;
                _conn_profile.connection_identify = socketOperator_->Identify(fd) + "@REUSE";

                //[dual-channel]
                _conn_profile.is_bind_cellular_network = _conn_profile.ip_items[i].is_bind_cellular_network;

                __UpdateProfile(_conn_profile);
                xinfo2(TSF"reused socket:%_", fd);
                return fd;
            }
        }
    }

    // set the first ip info to the profiler, after connect, the ip info will be overwrriten by the real one

    ShortLinkConnectObserver connect_observer(*this);

    _conn_profile.start_connect_time = ::gettickcount();
    
    if (outter_vec_addr_.empty()){
        auto ip_timeout = net_source_->GetIpConnectTimeout();
        socketOperator_->SetIpConnectionTimeout(std::get<0>(ip_timeout), std::get<1>(ip_timeout));
        xdebug2(TSF"ip connect time: %_, %_", std::get<0>(ip_timeout), std::get<1>(ip_timeout));
    }else{
        socketOperator_->SetIpConnectionTimeout(v4connect_timeout_ms_, v6connect_timeout_ms_);
    }
    SOCKET sock = socketOperator_->Connect(vecaddr, _conn_profile.proxy_info.type, proxy_addr, _conn_profile.proxy_info.username, _conn_profile.proxy_info.password);
	_conn_profile.connect_successful_time = ::gettickcount();
    delete proxy_addr;
    bool contain_v6 = __ContainIPv6(vecaddr);

    const SocketProfile& profile = socketOperator_->Profile();
    _conn_profile.conn_rtt = profile.rtt;
    _conn_profile.ip_index = profile.index;
    _conn_profile.conn_cost = profile.totalCost;
    _conn_profile.is0rtt = profile.is0rtt;

    __UpdateProfile(_conn_profile);
    
    //WeakNetworkLogic::Singleton::Instance()->OnConnectEvent(sock!=INVALID_SOCKET, profile.rtt, profile.index);
    net_source_->GetWeakNetworkLogic()->OnConnectEvent(sock!=INVALID_SOCKET, profile.rtt, profile.index);

    if (INVALID_SOCKET == sock) {
        xwarn2(TSF"task socket connect fail sock %_, net:%_", message.String(), getNetInfo());
        _conn_profile.conn_errcode = profile.errorCode;

        if (!socketOperator_->Breaker().IsBreak()) {
            __RunResponseError(kEctSocket, kEctSocketMakeSocketPrepared, _conn_profile, false);
        }
        else {
        	_conn_profile.disconn_errtype = kEctCanceld;
        	__UpdateProfile(_conn_profile);
        }

        return INVALID_SOCKET;
    }

    xassert2(0 <= profile.index && (unsigned int)profile.index < _conn_profile.ip_items.size());

    for (int i = 0; i < profile.index; ++i) {
        if (1 == connect_observer.ConnectingIndex[i] && func_network_report)
            func_network_report(__LINE__, kEctSocket, SOCKET_ERRNO(ETIMEDOUT), _conn_profile.ip_items[i].str_ip, _conn_profile.ip_items[i].str_host, _conn_profile.ip_items[i].port);
    }

    _conn_profile.host = _conn_profile.ip_items[profile.index].str_host;
    _conn_profile.ip_type = _conn_profile.ip_items[profile.index].source_type;
    _conn_profile.ip = _conn_profile.ip_items[profile.index].str_ip;
    _conn_profile.conn_time = gettickcount();
    _conn_profile.local_ip = socket_address::getsockname(sock).ip();
    _conn_profile.local_port = socket_address::getsockname(sock).port();
    _conn_profile.connection_identify = socketOperator_->Identify(sock);

    if (contain_v6 && profile.index > 0) {
        _conn_profile.ipv6_connect_failed = true;
    }

    __UpdateProfile(_conn_profile);

    xinfo2_if(task_.priority>=0, TSF"task socket connect success sock:%_, %_ host:%_, ip:%_, port:%_, local_ip:%_, local_port:%_, iptype:%_, net:%_", sock, message.String(), _conn_profile.host, _conn_profile.ip, _conn_profile.port, _conn_profile.local_ip, _conn_profile.local_port, IPSourceTypeString[_conn_profile.ip_type], _conn_profile.net_type);


//    struct linger so_linger;
//    so_linger.l_onoff = 1;
//    so_linger.l_linger = 0;

//    xerror2_if(0 != setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)), TSF"SO_LINGER %_(%_)", socket_errno, socket_strerror(socket_errno));
    return sock;
}


bool ShortLink::__ContainIPv6(const std::vector<socket_address>& _vecaddr) {
    if (!_vecaddr.empty()) {
        in6_addr addr6 = IN6ADDR_ANY_INIT;
        if (socket_inet_pton(AF_INET6, _vecaddr[0].ip(), &addr6)) { //first ip is ipv6
            xinfo2_if(!task_.long_polling && task_.priority >= 0, TSF"ip %_ is v6", _vecaddr[0].ip());
            return true;
        }
    }
    return false;
}

void ShortLink::__RunReadWrite(SOCKET _socket, int& _err_type, int& _err_code, ConnectProfile& _conn_profile) {
	xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);

	std::string url;
	std::map<std::string, std::string> headers;
#ifdef WIN32
	std::string replace_host = _conn_profile.host;
	if (kIPSourceProxy == _conn_profile.ip_type) {
		url += "http://";
		url += _conn_profile.host;
	} else {
		replace_host = _conn_profile.ip.empty() ? _conn_profile.host : _conn_profile.ip;
	}
	url += task_.cgi;

	headers[http::HeaderFields::KStringHost] = replace_host;
	headers["X-Online-Host"] = replace_host;
#else
	if (kIPSourceProxy == _conn_profile.ip_type) {
		url += "http://";
		url += _conn_profile.host;
	}
	url += task_.cgi;

	headers[http::HeaderFields::KStringHost] = _conn_profile.host;
#endif // WIN32

	if (_conn_profile.proxy_info.IsValid() && mars::comm::kProxyHttp == _conn_profile.proxy_info.type
		&& !_conn_profile.proxy_info.username.empty() && !_conn_profile.proxy_info.password.empty()) {
		std::string account_info = _conn_profile.proxy_info.username + ":" + _conn_profile.proxy_info.password;
		size_t dstlen = modp_b64_encode_len(account_info.length());

		char* dstbuf = (char*)malloc(dstlen);
		memset(dstbuf, 0, dstlen);

		int retsize = Comm::EncodeBase64((unsigned char*)account_info.c_str(), (unsigned char*)dstbuf, (int)account_info.length());
		dstbuf[retsize] = '\0';

		char auth_info[1024] = { 0 };
		snprintf(auth_info, sizeof(auth_info), "Basic %s", dstbuf);
		headers[http::HeaderFields::kStringProxyAuthorization] = auth_info;
        free(dstbuf);
	}

    //add user headers
    if(task_.headers.size() > 0) {
        auto iter = task_.headers.begin();
        while(iter != task_.headers.end()) {
            headers[iter->first] = iter->second;
            iter++;
        }
    }

	AutoBuffer out_buff;

    shortlink_pack(url, headers, send_body_, send_extend_, out_buff, tracker_.get());

	// send request
	xgroup2_define(group_send);
	xinfo2(TSF"task socket send sock:%_, %_ http len:%_, ", _socket, message.String(), out_buff.Length()) >> group_send;

    _conn_profile.start_send_packet_time = ::gettickcount();
	int send_ret = socketOperator_->Send(_socket, (const unsigned char*)out_buff.Ptr(), (unsigned int)out_buff.Length(), _err_code);
    _conn_profile.send_request_cost = ::gettickcount() - _conn_profile.start_send_packet_time;
    xinfo2(TSF"sent %_", send_ret) >> group_send;
    
	if (send_ret < 0) {
		xerror2(TSF"Send Request Error, ret:%0, errno:%1, nread:%_, nwrite:%_", send_ret, socketOperator_->ErrorDesc(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_send;
        is_keep_alive_ = false;
		__RunResponseError(kEctSocket, (_err_code == 0) ? kEctSocketWritenWithNonBlock : _err_code, _conn_profile, true);
		return;
	}
    
    GetSignalOnNetworkDataChange()(XLOGGER_TAG, send_ret, 0);

    if (socketOperator_->Breaker().IsBreak()) {
        xwarn2(TSF"Send Request break, sent:%_ nread:%_, nwrite:%_", send_ret, socket_nread(_socket), socket_nwrite(_socket)) >> group_send;
        return;
    }

    task_.priority >= 0 ? (xgroup2() << group_send) : (group_send.Clear());

	xgroup2_define(group_close);
	xgroup2_define(group_recv);
    
    xinfo2(TSF"task socket close sock:%_, %_, ", _socket, message.String()) >> group_close;
	xinfo2(TSF"task socket recv sock:%_,  %_, ", _socket, message.String()) >> group_recv;

	//recv response
    AutoBuffer body;
	AutoBuffer recv_buf;
	AutoBuffer extension;
    int        status_code = -1;
	off_t recv_pos = 0;
	MemoryBodyReceiver* receiver = new MemoryBodyReceiver(body);
	http::Parser parser(receiver, true);
    
    int timeout = 5000;
    if (socketOperator_->Protocol() == Task::kTransportProtocolQUIC){
        timeout = net_source_->GetQUICRWTimeoutMs(task_.cgi, &_conn_profile.quic_rw_timeout_source);
        _conn_profile.quic_rw_timeout_ms = timeout;
    }
    xinfo2(TSF"rwtimeout %_, timeout.source %_, ", timeout, _conn_profile.quic_rw_timeout_source) >> group_close;

    _conn_profile.start_read_packet_time = ::gettickcount();
	while (true) {
		int recv_ret = socketOperator_->Recv(_socket, recv_buf, KBufferSize, _err_code, timeout);
        xinfo2(TSF"socketOperator_ Recv %_/%_", recv_ret, _err_code);
       
        _conn_profile.rw_errcode = _err_code;
        _conn_profile.read_packet_finished_time = ::gettickcount();
        _conn_profile.recv_reponse_cost = _conn_profile.read_packet_finished_time - _conn_profile.start_read_packet_time;
        if (recv_ret == -2 && _err_code == SOCKET_ERRNO(ENOTCONN) && socketOperator_->Protocol() == Task::kTransportProtocolQUIC){
            _conn_profile.is_fast_fallback_tcp = 1;
            is_keep_alive_ = false;
        }
        __UpdateProfile(_conn_profile);
    
		if (recv_ret < 0) {
			xerror2(TSF"read block socket return false, error:%0, nread:%_, nwrite:%_", socketOperator_->ErrorDesc(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
            is_keep_alive_ = false;
			__RunResponseError(kEctSocket, (_err_code == 0) ? kEctSocketReadOnce : _err_code, _conn_profile, true);
			break;
		}

		if (socketOperator_->Breaker().IsBreak()) {
			xinfo2(TSF"user cancel, nread:%_, nwrite:%_", socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
        	_conn_profile.disconn_errtype = kEctCanceld;
			break;
		}

		if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == _err_code) {
			xerror2(TSF"read timeout error:(%_,%_), nread:%_, nwrite:%_ ", _err_code, socketOperator_->ErrorDesc(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
            
            if (socketOperator_->Protocol() == Task::kTransportProtocolQUIC){
                is_keep_alive_ = false;
                __RunResponseError(kEctSocket, kEctSocketRecvErr, _conn_profile, /*report=*/true);
                break;
            }
            
            continue;
		}
		if (recv_ret == 0 && socketOperator_->Protocol() != Task::kTransportProtocolQUIC) {
			xerror2(TSF"remote disconnect error:(%_,%_), nread:%_, nwrite:%_", _err_code, socketOperator_->ErrorDesc(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
            bool report_fail = true;
            if (_conn_profile.is_reused_fd) report_fail = false;
			__RunResponseError(kEctSocket, kEctSocketShutdown, _conn_profile, /*report=*/report_fail);
			break;
		}

		if (recv_ret > 0) {
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, 0, recv_ret);
            
			xinfo2(TSF"recv len:%_ ", recv_ret) >> group_recv;
            if (OnRecv)
                OnRecv(this, (unsigned int)(recv_buf.Length() - recv_pos), (unsigned int)recv_buf.Length());
            else
                xwarn2(TSF"OnRecv NULL.");
			recv_pos = recv_buf.Pos();
		}

		Parser::TRecvStatus parse_status = parser.Recv(recv_buf.Ptr(recv_buf.Length() - recv_ret), recv_ret);
        if (parser.FirstLineReady()) {
            status_code = parser.Status().StatusCode();
        }

		if (parse_status == http::Parser::kFirstLineError) {
			xerror2(TSF"http head not receive yet,but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__RunResponseError(kEctHttp, kEctHttpParseStatusLine, _conn_profile, true);
			break;
		}
		else if (parse_status == http::Parser::kHeaderFieldsError) {
			xerror2(TSF"parse http head failed, but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__RunResponseError(kEctHttp, kEctHttpSplitHttpHeadAndBody, _conn_profile, true);
			break;
		}
		else if (parse_status == http::Parser::kBodyError) {
			xerror2(TSF"content_length_ != body.Lenght(), Head:%0, http dump:%1 \n headers size:%2" , parser.Fields().ContentLength(), xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
			__RunResponseError(kEctHttp, kEctHttpSplitHttpHeadAndBody, _conn_profile, true);
			break;
		}
		else if (parse_status == http::Parser::kEnd) {
            if(is_keep_alive_) {    //parse server keep-alive config
                bool isKeepAlive = parser.Fields().IsConnectionKeepAlive();
                xwarn2_if(!isKeepAlive, "request keep-alive, but server return close");
                if(isKeepAlive) {
                    uint32_t timeout = parser.Fields().KeepAliveTimeout();
                    _conn_profile.keepalive_timeout = _conn_profile.transport_protocol == Task::kTransportProtocolQUIC ? 30 : timeout;
                    _conn_profile.socket_fd = _socket;
                    xinfo2(TSF"svr keepalive %_ seconds for url %_:%_", timeout, _conn_profile.ip, _conn_profile.port);
                } else {
                    is_keep_alive_ = false;
                }
            }

			if (status_code != 200) {
				xerror2(TSF"@%0, status_code != 200, code:%1, http dump:%2 \n headers size:%3", this, status_code, xlogger_memory_dump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
				__RunResponseError(kEctHttp, status_code, _conn_profile, true);
			}
			else {
				xinfo2(TSF"@%0, headers size:%_, ", this, parser.Fields().GetHeaders().size()) >> group_recv;
				__OnResponse(kEctOK, status_code, body, extension, _conn_profile, true);
			}
			break;
		}
		else {
			xdebug2(TSF"http parser status:%_ ", parse_status);
		}
	}

	xdebug2(TSF"read with nonblock socket http response, length:%_, ", recv_buf.Length()) >> group_recv;

    task_.priority >= 0 ? (xgroup2() << group_recv) : (group_recv.Clear());
#if defined(__ANDROID__) || defined(__APPLE__)
	struct tcp_info _info;
	if (getsocktcpinfo(_socket, &_info) == 0) {
		char tcp_info_str[1024] = {0};
		xinfo2(TSF"task socket close getsocktcpinfo:%_", tcpinfo2str(&_info, tcp_info_str, sizeof(tcp_info_str))) >> group_close;
	}
#endif
	xgroup2() << group_close;
}

void ShortLink::__UpdateProfile(const ConnectProfile _conn_profile) {
	STATIC_RETURN_SYNC2ASYNC_FUNC(boost::bind(&ShortLink::__UpdateProfile, this, _conn_profile));
    ConnectProfile profile = conn_profile_;
	conn_profile_ = _conn_profile;
	conn_profile_.tls_handshake_successful_time = profile.tls_handshake_successful_time;
    conn_profile_.tls_handshake_mismatch = profile.tls_handshake_mismatch;
    conn_profile_.tls_handshake_success = profile.tls_handshake_success;
}

void ShortLink::__RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _conn_profile, bool _report) {
    AutoBuffer buf;
    AutoBuffer extension;
    __OnResponse(_type, _errcode, buf, extension, _conn_profile, _report);
}

void ShortLink::__OnResponse(ErrCmdType _errType, int _status, AutoBuffer& _body, AutoBuffer& _extension, ConnectProfile& _conn_profile, bool _report) {
	_conn_profile.disconn_errtype = _errType;
	_conn_profile.disconn_errcode = _status;
    _conn_profile.channel_type = mars::stn::Task::kChannelShort;
	__UpdateProfile(_conn_profile);

 //   xassert2(!breaker_.IsBreak());

    if (kEctOK != _errType) {
//        xassert2(func_network_report);

        if (_report && func_network_report) func_network_report(__LINE__, _errType, _status, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
    }

    if (OnResponse) {
        move_wrapper<AutoBuffer> body(_body);
        move_wrapper<AutoBuffer> extension(_extension);
        OnResponse(this, _errType, _status, body, extension, false, _conn_profile);
    }
    else
        xwarn2(TSF"OnResponse NULL.");
}

void ShortLink::SetConnectParams(const std::vector<IPPortItem>& _out_addr, uint32_t v4timeout_ms, uint32_t v6timeout_ms) {
    outter_vec_addr_ = _out_addr;
    v4connect_timeout_ms_ = v4timeout_ms;
    v6connect_timeout_ms_ = v6timeout_ms;
}

void ShortLink::__CancelAndWaitWorkerThread() {
    xdebug_function();

    if (!thread_.isruning()) return;

    if (!socketOperator_->Breaker().Break()) {
        xassert2(false, "breaker fail");
    }
    thread_.join();
}
