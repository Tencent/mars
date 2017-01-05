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

#include <sstream>

#include "boost/bind.hpp"

#include "mars/app/app.h"
#include "mars/comm/socket/complexconnect.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/socket/block_socket.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/strutil.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/platform_comm.h"
#include "mars/baseevent/baseprjevent.h"

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif
#include "mars/stn/proto/shortlink_packer.h"



#define AYNC_HANDLER asyncreg_.Get()
#define STATIC_RETURN_SYNC2ASYNC_FUNC(func) RETURN_SYNC2ASYNC_FUNC(func, )

using namespace mars::stn;
using namespace mars::app;

static unsigned int KBufferSize = 8 * 1024;

namespace mars{
namespace stn{

class ShortLinkConnectObserver : public MComplexConnect {
  public:
    ShortLinkConnectObserver(ShortLink& _shortlink): shortlink_(_shortlink), index_(-1), rtt_(0), last_err_(-1) {
        memset(ConnectingIndex, 0, sizeof(ConnectingIndex));
    };

    virtual void OnCreated(unsigned int _index, const socket_address& _addr, SOCKET _socket) {}
    virtual void OnConnect(unsigned int _index, const socket_address& _addr, SOCKET _socket) {
        ConnectingIndex[_index] = 1;
    }
    virtual void OnConnected(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _error, int _rtt) {
        ConnectingIndex[_index] = 0;

        if (0 == _error) {
            xassert2(-1 == index_, "index_:%d", index_);
            index_ = _index;
        } else {
            xassert2(shortlink_.func_network_report);

            if (_index < shortlink_.Profile().ip_items.size())
                shortlink_.func_network_report(__LINE__, kEctSocket, _error, _addr.ip(), shortlink_.Profile().ip_items[_index].str_host, _addr.port());
        }

        if (last_err_ != 0) {
            last_err_ = _error;
            rtt_ = _rtt;
        }
    }

    int Index() const {return index_;}
    int LastErrorCode() const {return last_err_;}
    int Rtt() const {return rtt_;}

    char ConnectingIndex[32];

  private:
    ShortLinkConnectObserver(const ShortLinkConnectObserver&);
    ShortLinkConnectObserver& operator=(const ShortLinkConnectObserver&);

  private:
    ShortLink& shortlink_;
    int index_;
    int rtt_;
    int last_err_;
};

}}
///////////////////////////////////////////////////////////////////////////////////////

ShortLink::ShortLink(MessageQueue::MessageQueue_t _messagequeueid, NetSource& _netsource, const std::vector<std::string>& _host_list, const std::string& _url, const int _taskid, bool _use_proxy)
    : asyncreg_(MessageQueue::InstallAsyncHandler(_messagequeueid))
	, net_source_(_netsource)
	, thread_(boost::bind(&ShortLink::__Run, this), XLOGGER_TAG "::shortlink")
	, taskid_(_taskid)
    , url_(_url), use_proxy_(_use_proxy)
    , status_code_(-1)
    {
    xdebug2(XTHIS);
    xassert2(breaker_.IsCreateSuc(), "Create Breaker Fail!!!");
    shortlink_hosts_ = _host_list;
    if (shortlink_hosts_.empty())  shortlink_hosts_.push_back("");
    conn_profile_.host = shortlink_hosts_.front();
}

ShortLink::~ShortLink() {
    xinfo_function(TSF"taskid:%_, cgi:%_, @%_", taskid_, url_, this);
    __CancelAndWaitWorkerThread();
    asyncreg_.CancelAndWait();
}

void ShortLink::SendRequest(AutoBuffer& _buf_req) {
    xverbose_function();
    xdebug2(XTHIS)(TSF"bufReq.size:%_", _buf_req.Length());
    send_body_.Attach(_buf_req);

    thread_.start();
}

void ShortLink::__Run() {
    xmessage2_define(message, TSF"taskid:%_, cgi:%_, @%_", taskid_, url_, this);
    xinfo_function(TSF"%_, net:%_, body:%_", message.String(), getNetInfo(), buf_body_.Length());

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
    xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", taskid_, url_, this);

    std::vector<socket_address> vecaddr;

    _conn_profile.dns_time = ::gettickcount();
    __UpdateProfile(_conn_profile);

    _conn_profile.host = shortlink_hosts_.front();

    if (use_proxy_ && net_source_.GetShortLinkProxyInfo(_conn_profile.port, _conn_profile.ip, shortlink_hosts_)) {
    	_conn_profile.ip_type = kIPSourceProxy;
        IPPortItem item = {_conn_profile.ip, net_source_.GetShortLinkPort(), _conn_profile.ip_type, _conn_profile.host};
        _conn_profile.ip_items.push_back(item);
        __UpdateProfile(_conn_profile);
    } else {
        if (net_source_.GetShortLinkItems(shortlink_hosts_, _conn_profile.ip_items, dns_util_)) {
        	_conn_profile.host = _conn_profile.ip_items[0].str_host;
        	_conn_profile.ip_type = _conn_profile.ip_items[0].source_type;
        	_conn_profile.ip = _conn_profile.ip_items[0].str_ip;
        	_conn_profile.port = _conn_profile.ip_items[0].port;
        	__UpdateProfile(_conn_profile);
        }
    }

    xinfo2(TSF"task socket dns sock %_ proxy:%_, host:%_, ip list:%_", message.String(), kIPSourceProxy == _conn_profile.ip_type, _conn_profile.host, NetSource::DumpTable(_conn_profile.ip_items));

    bool isnat64 = ELocalIPStack_IPv6 == local_ipstack_detect();
    for (unsigned int i = 0; i < _conn_profile.ip_items.size(); ++i) {
        vecaddr.push_back(socket_address(_conn_profile.ip_items[i].str_ip.c_str(), _conn_profile.port).v4tov6_address(isnat64));
    }

    if (vecaddr.empty()) {
        xerror2(TSF"task socket connect fail %_ vecaddr empty", message.String());
        __RunResponseError(kEctDns, kEctDnsMakeSocketPrepared, _conn_profile);
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


    uint64_t startconnecttime = ::gettickcount();
    ShortLinkConnectObserver connect_observer(*this);

    SOCKET sock = ComplexConnect(kShortlinkConnTimeout, kShortlinkConnInterval).ConnectImpatient(vecaddr, breaker_, &connect_observer);

    _conn_profile.conn_errcode = connect_observer.LastErrorCode();
    _conn_profile.conn_rtt = connect_observer.Rtt();
    _conn_profile.ip_index = connect_observer.Index();
    __UpdateProfile(_conn_profile);

    if (INVALID_SOCKET == sock) {
        xwarn2(TSF"task socket connect fail sock %_, net:%_", message.String(), getNetInfo());

        if (!breaker_.IsBreak()) {
            __RunResponseError(kEctSocket, kEctSocketMakeSocketPrepared, _conn_profile, false);
        }
        else {
        	_conn_profile.disconn_errtype = kEctCanceled;
        	__UpdateProfile(_conn_profile);
        }

        return INVALID_SOCKET;
    }

    xassert2(0 <= connect_observer.Index() && (unsigned int)connect_observer.Index() < _conn_profile.ip_items.size());

    for (int i = 0; i < connect_observer.Index(); ++i) {
        if (1 == connect_observer.ConnectingIndex[i])
            func_network_report(__LINE__, kEctSocket, SOCKET_ERRNO(ETIMEDOUT), _conn_profile.ip_items[i].str_ip, _conn_profile.ip_items[i].str_host, _conn_profile.ip_items[i].port);
    }

    _conn_profile.host = _conn_profile.ip_items[connect_observer.Index()].str_host;
    _conn_profile.ip_type = _conn_profile.ip_items[connect_observer.Index()].source_type;
    _conn_profile.ip = _conn_profile.ip_items[connect_observer.Index()].str_ip;
    _conn_profile.conn_cost = gettickspan(startconnecttime);
    _conn_profile.conn_time = gettickcount();
    _conn_profile.local_ip = socket_address::getsockname(sock).ip();
    __UpdateProfile(_conn_profile);


    xinfo2(TSF"task socket connect success sock:%_, %_ host:%_, ip:%_, port:%_, iptype:%_, net:%_", sock, message.String(), _conn_profile.host, _conn_profile.ip, _conn_profile.port, IPSourceTypeString[_conn_profile.ip_type], _conn_profile.net_type);


//    struct linger so_linger;
//    so_linger.l_onoff = 1;
//    so_linger.l_linger = 0;

//    xerror2_if(0 != setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger)), TSF"SO_LINGER %_(%_)", socket_errno, socket_strerror(socket_errno));
    return sock;
}

void ShortLink::__RunReadWrite(SOCKET _socket, int& _err_type, int& _err_code, ConnectProfile& _conn_profile) {
	xmessage2_define(message)(TSF"taskid:%_, cgi:%_, @%_", taskid_, url_, this);

    std::string url;
    if (kIPSourceProxy==_conn_profile.ip_type) {
        url +="http://";
        url += _conn_profile.host;
    }
	url += url_;


	std::map<std::string, std::string> headers;
	headers[http::HeaderFields::KStringHost] = _conn_profile.host;
	AutoBuffer out_buff;

	shortlink_pack(url, headers, send_body_,  out_buff);

	// send request
	xgroup2_define(group_send);
	xinfo2(TSF"task socket send sock:%_, %_ http len:%_, ", _socket, message.String(), out_buff.Length()) >> group_send;

	int send_ret = block_socket_send(_socket, (const unsigned char*)out_buff.Ptr(), (unsigned int)out_buff.Length(), breaker_, _err_code);

	if (send_ret < 0) {
		xerror2(TSF"Send Request Error, ret:%0, errno:%1, nread:%_, nwrite:%_", send_ret, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_send;
		__OnResponse(kEctSocket, (_err_code == 0) ? kEctSocketWritenWithNonBlock : _err_code, buf_body_, _conn_profile, false);
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
	AutoBuffer recv_buf;
	off_t recv_pos = 0;
    http::MemoryBodyReceiver* receiver = new http::MemoryBodyReceiver(buf_body_);
	http::Parser parser(receiver, true);

	while (true) {
		int recv_ret = block_socket_recv(_socket, recv_buf, KBufferSize, breaker_, _err_code, 5000);

		if (recv_ret < 0) {
			xerror2(TSF"read block socket return false, error:%0, nread:%_, nwrite:%_", strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__OnResponse(kEctSocket, (_err_code == 0) ? kEctSocketReadOnce : _err_code, buf_body_, _conn_profile, socket_nwrite(_socket) == 0);
			break;
		}

		if (breaker_.IsBreak()) {
			xinfo2(TSF"user cancel, nread:%_, nwrite:%_", socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
        	_conn_profile.disconn_errtype = kEctCanceled;
			break;
		}

		if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == _err_code) {
			xerror2(TSF"read timeout error:(%_,%_), nread:%_, nwrite:%_ ", _err_code, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
//			__OnResponse(kEctSocket, kEctSocketRecvErr, buf_body_, _conn_profile, socket_nwrite(_socket) == 0);
//			break;
            continue;
		}
		if (recv_ret == 0) {
			xerror2(TSF"remote disconnect, nread:%_, nwrite:%_", _err_code, strerror(_err_code), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__OnResponse(kEctSocket,  kEctSocketShutdown, buf_body_, _conn_profile, socket_nwrite(_socket) == 0);
			break;
		}

		if (recv_ret > 0) {
            GetSignalOnNetworkDataChange()(XLOGGER_TAG, 0, recv_ret);
            
			xinfo2(TSF"recv len:%_ ", recv_ret) >> group_recv;
			OnRecv(this, (unsigned int)(recv_buf.Length() - recv_pos), (unsigned int)recv_buf.Length());
			recv_pos = recv_buf.Pos();
		}

        http::Parser::TRecvStatus parse_status = parser.Recv(recv_buf.Ptr(recv_buf.Length() - recv_ret), recv_ret);
        if (parser.FirstLineReady()) {
            status_code_ = parser.Status().StatusCode();
        }

		if (parse_status == http::Parser::kFirstLineError) {
			xerror2(TSF"http head not receive yet,but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__OnResponse(kEctHttp, kEctHttpParseStatusLine, buf_body_, _conn_profile, socket_nwrite(_socket) == 0);
			break;
		}
		else if (parse_status == http::Parser::kHeaderFieldsError) {
			xerror2(TSF"parse http head failed, but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_socket), socket_nwrite(_socket)) >> group_close;
			__OnResponse(kEctHttp, kEctHttpSplitHttpHeadAndBody, buf_body_, _conn_profile);
			break;
		}
		else if (parse_status == http::Parser::kBodyError) {
			xerror2(TSF"content_length_ != buf_body_.Lenght(), Head:%0, http dump:%1 \n headers size:%2" , parser.Fields().ContentLength(), xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
			__OnResponse(kEctHttp, kEctHttpSplitHttpHeadAndBody, buf_body_, _conn_profile);
			break;
		}
		else if (parse_status == http::Parser::kEnd) {
			if (status_code_ != 200) {
				xerror2(TSF"@%0, status_code_ != 200, code:%1, http dump:%2 \n headers size:%3", this, status_code_, xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size()) >> group_close;
				__OnResponse(kEctHttp, status_code_, buf_body_, _conn_profile);
			}
			else {
				xinfo2(TSF"@%0, headers size:%_, ", this, parser.Fields().GetHeaders().size()) >> group_recv;
				__OnResponse(kEctOK, status_code_, buf_body_, _conn_profile);
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

void ShortLink::__RunResponseError(ErrCmdType _type, int _errcode, ConnectProfile& _conn_profile, bool _isreport) {
    AutoBuffer buf;
    __OnResponse(_type, _errcode, buf, _conn_profile, false, _isreport);
}

void ShortLink::__OnResponse(ErrCmdType _errType, int _status, AutoBuffer& _body, ConnectProfile& _conn_profile, bool _cancelRetry, bool _report) {
	_conn_profile.disconn_errtype = _errType;
	_conn_profile.disconn_errcode = _status;
	__UpdateProfile(_conn_profile);

    xassert2(!breaker_.IsBreak());

    if (kEctOK != _errType) {
        xassert2(func_network_report);

        if (_report) func_network_report(__LINE__, _errType, _status, _conn_profile.ip, _conn_profile.host, _conn_profile.port);
    }

    OnResponse(this, _errType, _status, _body, _cancelRetry, _conn_profile);
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
