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

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif
#include "mars/stn/proto/shortlink_packer.h"



#define AYNC_HANDLER asyncreg_.Get()
#define STATIC_RETURN_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

using namespace mars::stn;
using namespace mars::app;
using namespace http;

static unsigned int KBufferSize = 8 * 1024;

namespace mars{
namespace stn{

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
            xassert2(shortlink_.func_network_report);

            if (_index < shortlink_.Profile().ip_items.size())
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
///////////////////////////////////////////////////////////////////////////////////////

ShortLink::ShortLink(MessageQueue::MessageQueue_t _messagequeueid, NetSource& _netsource, const Task& _task, bool _use_proxy)
    : asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
	, net_source_(_netsource)
	, task_(_task)
	, thread_(boost::bind(&ShortLink::__Run, this), XLOGGER_TAG "::shortlink")
    , use_proxy_(_use_proxy)
    , tracker_(shortlink_tracker::Create())
    {
    xinfo2(TSF"%_, handler:(%_,%_)",XTHIS, asyncreg_.Get().queue, asyncreg_.Get().seq);
    xassert2(breaker_.IsCreateSuc(), "Create Breaker Fail!!!");
}

ShortLink::~ShortLink() {
    xinfo_function(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);
    __CancelAndWaitWorkerThread();
    asyncreg_.CancelAndWait();
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
	getCurrNetLabel(conn_profile.net_type);
	conn_profile.start_time = ::gettickcount();
	conn_profile.tid = xlogger_tid();
	__UpdateProfile(conn_profile);

    SOCKET fd_socket = __RunConnect(conn_profile);

    if (INVALID_SOCKET == fd_socket) return;
    OnSend(this);

    int errtype = 0;
    int errcode = 0;
    __RunReadWrite(fd_socket, errtype, errcode, conn_profile);

    conn_profile.disconn_signal = ::getSignal(::getNetInfo() == kWifi);
    __UpdateProfile(conn_profile);

    socket_close(fd_socket);
}


SOCKET ShortLink::__RunConnect(ConnectProfile& _conn_profile) {
    xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);

    std::vector<socket_address> vecaddr;

    _conn_profile.dns_time = ::gettickcount();
    __UpdateProfile(_conn_profile);

    if (!task_.shortlink_host_list.empty()) _conn_profile.host = task_.shortlink_host_list.front();
    
    if (use_proxy_) {
        _conn_profile.proxy_info = mars::app::GetProxyInfo(_conn_profile.host);
    }
    
    bool use_proxy = use_proxy_ && _conn_profile.proxy_info.IsValid();
    bool isnat64 = ELocalIPStack_IPv6 == local_ipstack_detect();

    if (use_proxy && mars::comm::kProxyHttp == _conn_profile.proxy_info.type && net_source_.GetShortLinkDebugIP().empty()) {
        _conn_profile.ip = _conn_profile.proxy_info.ip;
        _conn_profile.port = _conn_profile.proxy_info.port;
    	_conn_profile.ip_type = kIPSourceProxy;
        IPPortItem item = {_conn_profile.ip, net_source_.GetShortLinkPort(), _conn_profile.ip_type, _conn_profile.host};
        _conn_profile.ip_items.push_back(item);
        __UpdateProfile(_conn_profile);
    } else {
        if (net_source_.GetShortLinkItems(task_.shortlink_host_list, _conn_profile.ip_items, dns_util_)) {
        	_conn_profile.host = _conn_profile.ip_items[0].str_host;
        	_conn_profile.ip_type = _conn_profile.ip_items[0].source_type;
        	_conn_profile.ip = _conn_profile.ip_items[0].str_ip;
        	_conn_profile.port = _conn_profile.ip_items[0].port;
        	__UpdateProfile(_conn_profile);
        }
    }
    
	std::string proxy_ip;
    if (use_proxy && mars::comm::kProxyNone != _conn_profile.proxy_info.type) {
		std::vector<std::string> proxy_ips;
        if (_conn_profile.proxy_info.ip.empty() && !_conn_profile.proxy_info.host.empty()) {
            if (!dns_util_.GetDNS().GetHostByName(_conn_profile.proxy_info.host, proxy_ips) || proxy_ips.empty()) {
                xwarn2(TSF"dns %_ error", _conn_profile.proxy_info.host);
                return false;
            }
			proxy_ip = proxy_ips.front();
        } else {
			proxy_ip = _conn_profile.proxy_info.ip;
        }
    }
    
    if (use_proxy && mars::comm::kProxyHttp == _conn_profile.proxy_info.type) {
        vecaddr.push_back(socket_address(proxy_ip.c_str(), _conn_profile.proxy_info.port).v4tov6_address(isnat64));
    } else {
        for (size_t i = 0; i < _conn_profile.ip_items.size(); ++i) {
            if (!use_proxy || mars::comm::kProxyNone == _conn_profile.proxy_info.type) {
                vecaddr.push_back(socket_address(_conn_profile.ip_items[i].str_ip.c_str(), _conn_profile.port).v4tov6_address(isnat64));
            } else {
                vecaddr.push_back(socket_address(_conn_profile.ip_items[i].str_ip.c_str(), _conn_profile.port));
            }
        }
    }
    
	socket_address* proxy_addr = NULL;
    if (use_proxy && (mars::comm::kProxyHttpTunel == _conn_profile.proxy_info.type || mars::comm::kProxySocks5 == _conn_profile.proxy_info.type)) {
		proxy_addr = &((new socket_address(proxy_ip.c_str(), _conn_profile.proxy_info.port))->v4tov6_address(isnat64));
        _conn_profile.ip_type = kIPSourceProxy;
    }

    xinfo2(TSF"task socket dns sock %_ proxy:%_, host:%_, ip list:%_", message.String(), kIPSourceProxy == _conn_profile.ip_type, _conn_profile.host, NetSource::DumpTable(_conn_profile.ip_items));

    if (vecaddr.empty()) {
        xerror2(TSF"task socket connect fail %_ vecaddr empty", message.String());
        __RunResponseError(kEctDns, kEctDnsMakeSocketPrepared, _conn_profile, false);
        return INVALID_SOCKET;
    }

    _conn_profile.host = _conn_profile.ip_items[0].str_host;
    _conn_profile.ip_type = _conn_profile.ip_items[0].source_type;
    _conn_profile.ip = _conn_profile.ip_items[0].str_ip;
    _conn_profile.port = _conn_profile.ip_items[0].port;
    _conn_profile.nat64 = isnat64;
    _conn_profile.dns_endtime = ::gettickcount();
    getCurrNetLabel(_conn_profile.net_type);
    __UpdateProfile(_conn_profile);

    // set the first ip info to the profiler, after connect, the ip info will be overwrriten by the real one

    ShortLinkConnectObserver connect_observer(*this);
	ComplexConnect conn(kShortlinkConnTimeout, kShortlinkConnInterval);

    SOCKET sock = conn.ConnectImpatient(vecaddr, breaker_, &connect_observer, _conn_profile.proxy_info.type, proxy_addr, _conn_profile.proxy_info.username, _conn_profile.proxy_info.password);
    delete proxy_addr;

    _conn_profile.conn_rtt = conn.IndexRtt();
    _conn_profile.ip_index = conn.Index();
    _conn_profile.conn_cost = conn.TotalCost();

    __UpdateProfile(_conn_profile);

    if (INVALID_SOCKET == sock) {
        xwarn2(TSF"task socket connect fail sock %_, net:%_", message.String(), getNetInfo());
        _conn_profile.conn_errcode = conn.ErrorCode();

        if (!breaker_.IsBreak()) {
            __RunResponseError(kEctSocket, kEctSocketMakeSocketPrepared, _conn_profile, false);
        }
        else {
        	_conn_profile.disconn_errtype = kEctCanceld;
        	__UpdateProfile(_conn_profile);
        }

        return INVALID_SOCKET;
    }

    xassert2(0 <= conn.Index() && (unsigned int)conn.Index() < _conn_profile.ip_items.size());

    for (int i = 0; i < conn.Index(); ++i) {
        if (1 == connect_observer.ConnectingIndex[i])
            func_network_report(__LINE__, kEctSocket, SOCKET_ERRNO(ETIMEDOUT), _conn_profile.ip_items[i].str_ip, _conn_profile.ip_items[i].str_host, _conn_profile.ip_items[i].port);
    }

    _conn_profile.host = _conn_profile.ip_items[conn.Index()].str_host;
    _conn_profile.ip_type = _conn_profile.ip_items[conn.Index()].source_type;
    _conn_profile.ip = _conn_profile.ip_items[conn.Index()].str_ip;
    _conn_profile.conn_time = gettickcount();
    _conn_profile.local_ip = socket_address::getsockname(sock).ip();
    _conn_profile.local_port = socket_address::getsockname(sock).port();
    __UpdateProfile(_conn_profile);

    xinfo2(TSF"task socket connect success sock:%_, %_ host:%_, ip:%_, port:%_, local_ip:%_, local_port:%_, iptype:%_, net:%_", sock, message.String(), _conn_profile.host, _conn_profile.ip, _conn_profile.port, _conn_profile.local_ip, _conn_profile.local_port, IPSourceTypeString[_conn_profile.ip_type], _conn_profile.net_type);


//    struct linger so_linger;
//    so_linger.l_onoff = 1;
//    so_linger.l_linger = 0;

//    xerror2_if(0 != setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)), TSF"SO_LINGER %_(%_)", socket_errno, socket_strerror(socket_errno));
    return sock;
}

void ShortLink::__RunReadWrite(SOCKET _socket, int& _err_type, int& _err_code, ConnectProfile& _conn_profile) {
	xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", task_.taskid, task_.cgi, this);

    std::string url;
    if (kIPSourceProxy==_conn_profile.ip_type) {
        url +="http://";
        url += _conn_profile.host;
    }
	url += task_.cgi;


	std::map<std::string, std::string> headers;
	headers[http::HeaderFields::KStringHost] = _conn_profile.host;

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
	}

	AutoBuffer out_buff;

	shortlink_pack(url, headers, send_body_, send_extend_, out_buff, tracker_.get());

	// send request
	xgroup2_define(group_send);
	xinfo2(TSF"task socket send sock:%_, %_ http len:%_, ", _socket, message.String(), out_buff.Length()) >> group_send;

	int send_ret = block_socket_send(_socket, (const unsigned char*)out_buff.Ptr(), (unsigned int)out_buff.Length(), breaker_, _err_code);

	if (send_ret < 0) {
		xerror2(TSF"Send Request Error, ret:%0, errno:%1, nread:%_, nwrite:%_", send_ret, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_send;
		__RunResponseError(kEctSocket, (_err_code == 0) ? kEctSocketWritenWithNonBlock : _err_code, _conn_profile, true);
		return;
	}
    
    GetSignalOnNetworkDataChange()(XLOGGER_TAG, send_ret, 0);

    if (breaker_.IsBreak()) {
        xwarn2(TSF"Send Request break, sent:%_ nread:%_, nwrite:%_", send_ret, socket_nread(_socket), socket_nwrite(_socket)) >> group_send;
        return;
    }

	xgroup2() << group_send;

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

	while (true) {
		int recv_ret = block_socket_recv(_socket, recv_buf, KBufferSize, breaker_, _err_code, 5000);

		if (recv_ret < 0) {
			xerror2(TSF"read block socket return false, error:%0, nread:%_, nwrite:%_", strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__RunResponseError(kEctSocket, (_err_code == 0) ? kEctSocketReadOnce : _err_code, _conn_profile, true);
			break;
		}

		if (breaker_.IsBreak()) {
			xinfo2(TSF"user cancel, nread:%_, nwrite:%_", socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
        	_conn_profile.disconn_errtype = kEctCanceld;
			break;
		}

		if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == _err_code) {
			xerror2(TSF"read timeout error:(%_,%_), nread:%_, nwrite:%_ ", _err_code, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
            continue;
		}
		if (recv_ret == 0) {
			xerror2(TSF"remote disconnect, nread:%_, nwrite:%_", _err_code, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__RunResponseError(kEctSocket, kEctSocketShutdown, _conn_profile, true);
			break;
		}

		if (recv_ret > 0) {
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, 0, recv_ret);
            
			xinfo2(TSF"recv len:%_ ", recv_ret) >> group_recv;
			OnRecv(this, (unsigned int)(recv_buf.Length() - recv_pos), (unsigned int)recv_buf.Length());
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
			xerror2(TSF"content_length_ != body.Lenght(), Head:%0, http dump:%1 \n headers size:%2" , parser.Fields().ContentLength(), xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
			__RunResponseError(kEctHttp, kEctHttpSplitHttpHeadAndBody, _conn_profile, true);
			break;
		}
		else if (parse_status == http::Parser::kEnd) {
			if (status_code != 200) {
				xerror2(TSF"@%0, status_code != 200, code:%1, http dump:%2 \n headers size:%3", this, status_code, xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
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

	xgroup2() << group_recv;
#if defined(__ANDROID__) || defined(__APPLE__)
	struct tcp_info _info;
	if (getsocktcpinfo(_socket, &_info) == 0) {
		char tcp_info_str[1024] = {0};
		xinfo2(TSF"task socket close getsocktcpinfo:%_", tcpinfo2str(&_info, tcp_info_str, sizeof(tcp_info_str))) >> group_close;
	}
#endif
	xgroup2() << group_close;
}

void ShortLink::__UpdateProfile(const ConnectProfile& _conn_profile) {
	STATIC_RETURN_SYNC2ASYNC_FUNC(boost::bind(&ShortLink::__UpdateProfile, this, _conn_profile));
	conn_profile_ = _conn_profile;
}

void ShortLink::__RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _conn_profile, bool _report) {
    AutoBuffer buf;
    AutoBuffer extension;
    __OnResponse(_type, _errcode, buf, extension, _conn_profile, _report);
}

void ShortLink::__OnResponse(ErrCmdType _errType, int _status, AutoBuffer& _body, AutoBuffer& _extension, ConnectProfile& _conn_profile, bool _report) {
	_conn_profile.disconn_errtype = _errType;
	_conn_profile.disconn_errcode = _status;
	__UpdateProfile(_conn_profile);

 //   xassert2(!breaker_.IsBreak());

    if (kEctOK != _errType) {
        xassert2(func_network_report);

        if (_report) func_network_report(__LINE__, _errType, _status, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
    }

    OnResponse(this, _errType, _status, _body, _extension, -1 != _conn_profile.ip_index, _conn_profile);
}

void ShortLink::__CancelAndWaitWorkerThread() {
    xdebug_function();

    if (!thread_.isruning()) return;

    xassert2(breaker_.IsCreateSuc());

    if (!breaker_.Break()) {
        xassert2(false, "breaker fail");
        breaker_.Close();
    }

    dns_util_.Cancel();
    thread_.join();
}
