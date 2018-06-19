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
 * proxy_test.cc
 *
 *  Created on: 2017-3-29
 *      Author: yanguoyue
 */

#include "proxy_test.h"

#include "mars/comm/comm_data.h"
#include "mars/comm/socket/complexconnect.h"

#include "mars/comm/socket/socket_address.h"
#include "mars/comm/socket/local_ipstack.h"
#include "mars/comm/socket/block_socket.h"
#include "mars/comm/xlogger/xlogger.h"

#include "mars/comm/platform_comm.h"
#include "mars/comm/http.h"
#include "mars/comm/crypt/ibase64.h"

#include "mars/stn/config.h"

#if defined(__ANDROID__) || defined(__APPLE__)
#include "mars/comm/socket/getsocktcpinfo.h"
#endif

#include "mars/stn/proto/shortlink_packer.h"
#include "net_source.h"

static unsigned int BUFFER_SIZE = 8 * 1024;
static const uint16_t TEST_PORT = 80;
using namespace mars::stn;

ProxyTest::~ProxyTest() {

}

bool ProxyTest::ProxyIsAvailable(const mars::comm::ProxyInfo _proxy_info, const std::string& _host, const std::vector<std::string>& _hardcode_ips) {
    xinfo_function(TSF"type:%_ host:%_ ip:%_:%_ username:%_ test_host:%_ hardcode_ip:%_", _proxy_info.type, _proxy_info.host, _proxy_info.ip, _proxy_info.port, _proxy_info.username, _host, _hardcode_ips.empty()?"": _hardcode_ips.front());
    
    if (!_proxy_info.IsValid() ||( _host.empty() && _hardcode_ips.empty())) {
        xerror2(TSF"parameter is invalid");
        return false;
    }
    
    SOCKET sock = __Connect(_proxy_info, _host, _hardcode_ips);
    if (INVALID_SOCKET == sock) {
        return false;
    }

    int status_code = __ReadWrite(sock, _proxy_info, _host);

    socket_close(sock);
    
    xinfo2(TSF"test proxy status code:%_", status_code);
    
    return status_code == 200;

}

SOCKET ProxyTest::__Connect(const mars::comm::ProxyInfo& _proxy_info, const std::string& _host, const std::vector<std::string>& _hardcode_ips) {
    NetSource::DnsUtil dns_util_;
    
	std::string proxy_ip;
    if (mars::comm::kProxyNone != _proxy_info.type) {
        if (!_proxy_info.ip.empty()) {
            proxy_ip = _proxy_info.ip;
        } else {
			std::vector<std::string> proxy_ips;
            if (!dns_util_.GetDNS().GetHostByName(_proxy_info.host, proxy_ips) || proxy_ips.empty()) {
                xwarn2(TSF"dns proxy host error, host:%_", _proxy_info.host);
                return INVALID_SOCKET;
            }
			proxy_ip = proxy_ips.front();
        }
    }
    
    bool isnat64 = ELocalIPStack_IPv6 == local_ipstack_detect();
    std::vector<socket_address> vecaddr;
    
    if (mars::comm::kProxyHttp == _proxy_info.type) {
		vecaddr.push_back(socket_address(proxy_ip.c_str(), _proxy_info.port).v4tov6_address(isnat64));
    } else {
		std::vector<std::string> test_ips;
		if (!dns_util_.GetDNS().GetHostByName(_host, test_ips) || test_ips.empty()) {
			xwarn2(TSF"dns test_host error, host:%_", _host);
			if (_hardcode_ips.empty()) {
				return INVALID_SOCKET;
			}
			test_ips = _hardcode_ips;
		}

        for (size_t i = 0; i < test_ips.size(); ++i) {
            if (mars::comm::kProxyNone == _proxy_info.type) {
                vecaddr.push_back(socket_address(test_ips[i].c_str(), TEST_PORT).v4tov6_address(isnat64));
            } else {
                vecaddr.push_back(socket_address(test_ips[i].c_str(), TEST_PORT));
            }
        }
    }
    
    
    if (vecaddr.empty()) {
        xerror2("test proxy socket close sock:-1 vecaddr empty");
        return INVALID_SOCKET;
    }
    
    socket_address* proxy_addr = NULL;
    
    if (mars::comm::kProxyNone != _proxy_info.type && mars::comm::kProxyHttp != _proxy_info.type) {
		proxy_addr = &((new socket_address(proxy_ip.c_str(), _proxy_info.port))->v4tov6_address(isnat64));
    }
    
    ComplexConnect com_connect(kLonglinkConnTimeout, kLonglinkConnInteral, kLonglinkConnInteral, kLonglinkConnMax);
    
    SOCKET sock = com_connect.ConnectImpatient(vecaddr, testproxybreak_, NULL, _proxy_info.type, proxy_addr, _proxy_info.username, _proxy_info.password);
    delete proxy_addr;
    
    if (INVALID_SOCKET == sock) {
        xwarn2(TSF"test proxy connect fail sock:-1, costtime:%0", com_connect.TotalCost());
    }
    
    xinfo2(TSF"test proxy connect suc sock:%_, net:%_", sock, ::getNetInfo());
    
    return sock;
    
}

int ProxyTest::__ReadWrite(SOCKET _sock, const mars::comm::ProxyInfo& _proxy_info, const std::string& _host) {
    
    std::map<std::string, std::string> headers;
    headers[http::HeaderFields::KStringHost] = _host;
    
    if (_proxy_info.IsValid() && mars::comm::kProxyHttp == _proxy_info.type && !_proxy_info.username.empty() && !_proxy_info.password.empty()) {
        std::string account_info = _proxy_info.username + ":" + _proxy_info.password;
        size_t dstlen = modp_b64_encode_len(account_info.length());
        
        char* dstbuf = (char*)malloc(dstlen);
        memset(dstbuf, 0, dstlen);
        
        int retsize = Comm::EncodeBase64((unsigned char*)account_info.c_str(), (unsigned char*)dstbuf, (int)account_info.length());
        dstbuf[retsize] = '\0';
        
        char auth_info[1024] = {0};
        snprintf(auth_info, sizeof(auth_info), "Basic %s", dstbuf);
        headers[http::HeaderFields::kStringProxyAuthorization] = auth_info;
    }
    
    AutoBuffer body;
    AutoBuffer out_buff;
    
    using namespace http;
    Builder req_builder(kRequest);
    req_builder.Request().Method(RequestLine::kGet);
    req_builder.Request().Version(kVersion_1_1);
    
    req_builder.Fields().HeaderFiled(HeaderFields::MakeAcceptAll());
    req_builder.Fields().HeaderFiled(HeaderFields::KStringUserAgent, HeaderFields::KStringMicroMessenger);
    req_builder.Fields().HeaderFiled(HeaderFields::MakeCacheControlNoCache());
    req_builder.Fields().HeaderFiled(HeaderFields::MakeContentTypeOctetStream());
    req_builder.Fields().HeaderFiled(HeaderFields::MakeConnectionClose());
    req_builder.Fields().HeaderFiled(HeaderFields::KStringHost, _host.c_str());
    req_builder.Fields().HeaderFiled(HeaderFields::KStringContentLength, "0");
    
    
    if (mars::comm::kProxyHttp == _proxy_info.type && !_proxy_info.username.empty() && !_proxy_info.password.empty()) {
        std::string account_info = _proxy_info.username + ":" + _proxy_info.password;
        size_t dstlen = modp_b64_encode_len(account_info.length());
        
        char* dstbuf = (char*)malloc(dstlen);
        memset(dstbuf, 0, dstlen);
        
        int retsize = Comm::EncodeBase64((unsigned char*)account_info.c_str(), (unsigned char*)dstbuf, (int)account_info.length());
        dstbuf[retsize] = '\0';
        
        char auth_info[1024] = {0};
        snprintf(auth_info, sizeof(auth_info), "Basic %s", dstbuf);
        req_builder.Fields().HeaderFiled(HeaderFields::kStringProxyAuthorization, auth_info);

    }
    
    std::string url;
    if (mars::comm::kProxyHttp == _proxy_info.type) {
        url = std::string("http://") + _host;
    } else {
        url = "/";
    }

    req_builder.Request().Url(url);
    req_builder.HeaderToBuffer(out_buff);
    
    int err_code = 0;
    int send_ret = block_socket_send(_sock, (const unsigned char*)out_buff.Ptr(), (unsigned int)out_buff.Length(), testproxybreak_, err_code);
    
    if (send_ret < 0) {
        xerror2(TSF"test proxy Error, ret:%0, errno:%1, nread:%_, nwrite:%_", send_ret, strerror(err_code), socket_nread(_sock), socket_nwrite(_sock));
        return false;
    }
    
    if (testproxybreak_.IsBreak()) {
        xwarn2(TSF"test proxy break, sent:%_ nread:%_, nwrite:%_", send_ret, socket_nread(_sock), socket_nwrite(_sock));
        return false;
    }
    
    //recv response
    AutoBuffer recv_buf;
    AutoBuffer recv_body;
    http::MemoryBodyReceiver* receiver = new http::MemoryBodyReceiver(recv_body);
    http::Parser parser(receiver, true);
    
    int status_code = 0;
    
    while (true) {
        int recv_ret = block_socket_recv(_sock, recv_buf, BUFFER_SIZE, testproxybreak_, err_code, 5000);
        
        if (recv_ret < 0) {
            xerror2(TSF"read block socket return false, error:%0, nread:%_, nwrite:%_", strerror(err_code), socket_nread(_sock), socket_nwrite(_sock));
            break;
        }
        
        if (testproxybreak_.IsBreak()) {
            xinfo2(TSF"user cancel, nread:%_, nwrite:%_", socket_nread(_sock), socket_nwrite(_sock));
            break;
        }
        
        if (recv_ret == 0 && SOCKET_ERRNO(ETIMEDOUT) == err_code) {
            xerror2(TSF"read timeout error:(%_,%_), nread:%_, nwrite:%_ ", err_code, strerror(err_code), socket_nread(_sock), socket_nwrite(_sock));
            continue;
        }
        if (recv_ret == 0) {
            xerror2(TSF"remote disconnect, nread:%_, nwrite:%_", err_code, strerror(err_code), socket_nread(_sock), socket_nwrite(_sock));
            break;
        }
        
        http::Parser::TRecvStatus parse_status = parser.Recv(recv_buf.Ptr(recv_buf.Length() - recv_ret), recv_ret);
        if (parser.FirstLineReady()) {
            status_code = parser.Status().StatusCode();
        }
        
        if (parse_status == http::Parser::kFirstLineError) {
            xerror2(TSF"http head not receive yet,but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_sock), socket_nwrite(_sock));
            break;
        } else if (parse_status == http::Parser::kHeaderFieldsError) {
            xerror2(TSF"parse http head failed, but socket closed, length:%0, nread:%_, nwrite:%_ ", recv_buf.Length(), socket_nread(_sock), socket_nwrite(_sock));
            break;
        } else if (parse_status == http::Parser::kBodyError) {
            xerror2(TSF"content_length_ != buf_body_.Lenght(), Head:%0, http dump:%1 \n headers size:%2" , parser.Fields().ContentLength(), xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size());
            break;
        } else if (parse_status == http::Parser::kEnd) {
            if (status_code != 200) {
                xerror2(TSF"@%0, status_code_ != 200, code:%1, http dump:%2 \n headers size:%3", this, status_code, xdump(recv_buf.Ptr(), recv_buf.Length()), parser.Fields().GetHeaders().size());
            }
            break;
        } else {
            xdebug2(TSF"http parser status:%_ ", parse_status);
        }
    }
    
    return status_code;
}
