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
 * httpquery.cc
 *
 *  Created on: 2014年6月27日
 *      Author: wutianqiang
 */

#include "httpquery.h"

#include <cstddef>

#include "mars/sdt/sdt_logic.h"

#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/http.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/sdt/constants.h"

#include "dnsquery.h"
#include "http_url_parser.h"
#include "tcpquery.h"

using namespace mars::sdt;

static char* string_strnstr(const char* _src, const char* _sfind, int _pos) {
    if (_src == NULL || _sfind == NULL) return NULL;

    int l1, l2;
    l2 = (int)strlen(_sfind);

    if (!l2) return (char*)_src;

    l1 = (int)strlen(_src);
    _pos = (_pos > l1) ? l1 : _pos;

    while (_pos >= l2) {
        _pos--;

        if (!memcmp(_src, _sfind, l2))
            return (char*)_src;

        _src++;
    }

    return NULL;
}

static int SplitHttpHeadAndBody(const AutoBuffer& buf, std::string& strHead) {
    char* pBuf = (char*)buf.Ptr();
    char* pos = string_strnstr(pBuf, "\r\n\r\n", (int)buf.Length());

    if (pos == NULL)
        return -1;

    //  '2' retain for '\r\n'.
    ptrdiff_t headLength = (pos - pBuf) + 2;

    strHead = std::string(pBuf, headLength);
    return (int)headLength + 2;
}


int SendHttpQuery(const std::string& _url, int& _status_code, std::string& _errmsg, int _timeout) {
    xinfo2(TSF"httpQuery:_url=%_", _url);
    if (!strutil::StartsWith(_url, "http://")) {
    	xerror2(TSF"url is not start with http://");
        _errmsg.append("url is not start with http://");
        return -2;
    }
    unsigned long long timebegin = gettickcount();

    if (_timeout <= 0) {
        _timeout = HTTP_DEFAULT_TIMEOUT;
    }
    HttpUrlParser http_url_parser(_url);
    std::string host = http_url_parser.Host();
    xdebug2(TSF"host=%0", host);

    std::string str_req("");
    http::RequestLine reqLine(http::RequestLine::kGet, http_url_parser.Path(), http::kVersion_1_1);
    str_req.append(reqLine.ToString());

    http::HeaderFields header;
    header.HeaderFiled("Accept", "text/html, application/xhtml+xml, */*");
    header.HeaderFiled("Accept-Language", "zh-CN");
    header.HeaderFiled("User-Agent", USER_AGENT);
    header.HeaderFiled("Accept-Encoding", "gzip, deflate");
    header.HeaderFiled("Proxy-Connection", "Keep-Alive");

    bool domain_is_ipaddr = socket_address(host.c_str(), 0).valid();  // 判断strHost是否是一个点分十进制IP

    header.HeaderFiled("Host", host.c_str());
    str_req.append(header.ToString());
    str_req.append("\r\n\r\n");  // important

    xdebug2(TSF"str_req=%_", str_req);

    unsigned int port = http_url_parser.Port();

    struct socket_ipinfo_t ipinfo;

    char ip[20] = {0};

    int ret = 0;

    do {
        if (!domain_is_ipaddr) {
            unsigned long long timespan1 = gettickspan(timebegin);

            if ((unsigned int)_timeout <= timespan1) {
                xwarn2(TSF"check http timeout.");
                _errmsg.append("check http timeout.");
                ret = -1;
                break;
            }

            if (0 == socket_gethostbyname(host.c_str(), &ipinfo, (int)(_timeout - timespan1), NULL)) {
                strncpy(ip, socket_address(ipinfo.ip[0]).ip(), sizeof(ip));
            } else {
                xerror2(TSF"check http get DNS error.");
                _errmsg.append("check http get DNS error.");
                ret = -1;
                break;
            }
        } else {
            strcpy(ip,  host.c_str());
        }

        unsigned long long timespan2 = gettickspan(timebegin);

        if ((unsigned int)_timeout <= timespan2) {
            xwarn2(TSF"check http timeout.");
            _errmsg.append("check http timeout.");
            ret = -1;
            break;
        }

        TcpQuery tcpquery(ip, port, (int)(_timeout - timespan2));
        unsigned long long timeSpan3 = gettickspan(timebegin);

        if ((unsigned int)_timeout <= timeSpan3) {
            xwarn2(TSF"check http timeout.");
            _errmsg.append("check http timeout.");
            ret = -1;
            break;
        }

        if ((ret = tcpquery.tcp_send((unsigned char const*)str_req.c_str(), (unsigned int)str_req.length(), (int)(_timeout - timeSpan3))) < 0) {
            xerror2(TSF"tcp send data error, ret: %0", ret);
            _errmsg.append("send http data error.");
            break;
        }

        AutoBuffer recv_autobuf;
        unsigned long long timeSpan4 = gettickspan(timebegin);

        if ((unsigned int)_timeout <= timeSpan4) {
            xwarn2(TSF"check http timeout.");
            _errmsg.append("check http timeout.");
            ret = -1;
            break;
        }

        if ((ret = tcpquery.tcp_receive(recv_autobuf, 1024/*HTTP_DUMMY_RECV_DATA_SIZE*/, (int)(_timeout - timeSpan4))) < 0) {
            xerror2(TSF"tcp receive data error, ret: %0", ret);
            _errmsg.append("receive http data error.");
            break;
        }

        xdebug2(TSF"recvAutoBuf=%0", (char*)recv_autobuf.Ptr());
        std::string str_statusline;
        SplitHttpHeadAndBody(recv_autobuf, str_statusline);
        http::StatusLine statusLine;
        statusLine.FromString(str_statusline);
        _status_code = statusLine.StatusCode();
    } while (false);

    xdebug2(TSF"ret=%0", ret);
    return ret;
}

