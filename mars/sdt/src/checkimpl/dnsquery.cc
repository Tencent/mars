// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


#include "dnsquery.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <algorithm>
#if defined __APPLE__
#include <fstream>
#endif

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/socket/socket_address.h"

#include "netchecker_trafficmonitor.h"

#define TRAFFIC_LIMIT_RET_CODE (INT_MIN)
#define DNS_PORT (53)
#define DEFAULT_TIMEOUT (3000)
#define NAME_SVR ("nameserver")
#define NAME_SVR_LEN (10)

// Type field of Query and Answer
#define T_A         1       /* host address */
#define T_NS        2       /* authoritative server */
#define T_CNAME     5       /* canonical name */
#define T_SOA       6       /* start of authority zone */
#define T_PTR       12      /* domain name pointer */
#define T_MX        15      /* mail routing information */

namespace {
// DNS header structure
#pragma pack(push, 1)
struct DNS_HEADER {
    unsigned    short id;           // identification number

    unsigned    char rd     : 1;    // recursion desired
    unsigned    char tc     : 1;    // truncated message
    unsigned    char aa     : 1;    // authoritive answer
    unsigned    char opcode : 4;    // purpose of message
    unsigned    char qr     : 1;    // query/response flag

    unsigned    char rcode  : 4;    // response code
    unsigned    char cd     : 1;    // checking disabled
    unsigned    char ad     : 1;    // authenticated data
    unsigned    char z      : 1;    // its z! reserved
    unsigned    char ra     : 1;    // recursion available

    unsigned    short q_count;      // number of question entries
    unsigned    short ans_count;    // number of answer entries
    unsigned    short auth_count;   // number of authority entries
    unsigned    short add_count;    // number of resource entries
};



// Constant sized fields of query structure
struct QUESTION {
    unsigned short qtype;
    unsigned short qclass;
};


// Constant sized fields of the resource record structure
struct  R_DATA {
    unsigned short type;
    unsigned short _class;
    unsigned int   ttl;
    unsigned short data_len;
};
#pragma pack(pop)

// Pointers to resource record contents
struct RES_RECORD {
    unsigned char*  name;
    struct R_DATA*  resource;
    unsigned char*  rdata;
};

// Structure of a Query
typedef struct {
    unsigned char*       name;
    struct QUESTION*     ques;
} QUERY;

}

//函数原型声明
static void           ChangetoDnsNameFormat(unsigned char*, std::string);
static unsigned char* ReadName(unsigned char*, unsigned char*, int*);
static void           GetHostDnsServerIP(std::vector<std::string>& _dns_servers);
static void           PrepareDnsQueryPacket(unsigned char* _buf, struct DNS_HEADER* _dns, unsigned char* _qname, const std::string& _host);
static void           ReadRecvAnswer(unsigned char* _buf, struct DNS_HEADER* _dns, unsigned char* _reader, struct RES_RECORD* _answers);
static int            RecvWithinTime(int _fd, char* _buf, size_t _buf_n, struct sockaddr* _addr, socklen_t* _len, unsigned int _sec, unsigned _usec);
static void           FreeAll(struct RES_RECORD* _answers);
static bool           isValidIpAddress(const char* _ipaddress);

/**
 *函数名:    socket_gethostbyname
 *功能: 输入域名，可得到该域名下所对应的IP地址列表
 *输入:       _host：输入的要查询的主机域名
 *输入:       _timeout：设置查询超时时间，单位为毫秒
 *输入:       _dnsserver 指定的dns服务器的IP
 *输出:        _ipinfo为要输出的ip信息结构体
 *返回值:          当返回-1表示查询失败，当返回0则表示查询成功
 *
 */
int socket_gethostbyname(const char* _host, socket_ipinfo_t* _ipinfo, int _timeout /*ms*/, const char* _dnsserver, NetCheckTrafficMonitor* _traffic_monitor) {
    xinfo2(TSF"in socket_gethostbyname,_host=%0", _host);

    if (NULL == _host) return -1;

    if (NULL == _ipinfo) return -1;

    if (_timeout <= 0) _timeout = DEFAULT_TIMEOUT;

    std::vector<std::string> dns_servers;

    if (_dnsserver && isValidIpAddress(_dnsserver)) {
        xinfo2(TSF"DNS server: %0", _dnsserver);
        dns_servers.push_back(_dnsserver);
    } else {
        xinfo2(TSF"use default DNS server.");
        GetHostDnsServerIP(dns_servers);
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // UDP packet for DNS queries

    if (sock < 0) {
        xerror2(TSF"in socket_gethostbyname get socket error");
        return -1;
    }

    struct sockaddr_in dest = {0};

    if (dns_servers.empty()) {
        xerror2(TSF"No dns servers error.");
        ::socket_close(sock);
        return -1;
    }

    std::vector<std::string>::iterator iter = dns_servers.begin();
    // 配置DNS服务器的IP和端口号
    dest = *(struct sockaddr_in*)(&socket_address((*iter).c_str(), DNS_PORT).address());

    struct RES_RECORD answers[SOCKET_MAX_IP_COUNT];  // the replies from the DNS server
    memset(answers, 0, sizeof(RES_RECORD)*SOCKET_MAX_IP_COUNT);

    int ret = -1;

    do {
        const unsigned int BUF_LEN = 65536;
        unsigned char send_buf[BUF_LEN] = {0};
        unsigned char recv_buf[BUF_LEN] = {0};
        struct DNS_HEADER* dns = (struct DNS_HEADER*)send_buf;
        unsigned char* qname = (unsigned char*)&send_buf[sizeof(struct DNS_HEADER)];
        PrepareDnsQueryPacket(send_buf, dns, qname, _host);
        unsigned long send_packlen = sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION);

        if (NULL != _traffic_monitor) {
            if (_traffic_monitor->sendLimitCheck(send_packlen)) {
                ret = TRAFFIC_LIMIT_RET_CODE;
                break;
            }
        }

        if (sendto(sock, (char*)send_buf, send_packlen, 0, (struct sockaddr*)&dest, sizeof(dest)) == -1) {
            xerror2(TSF"send dns query error.");
            break;
        }

        struct sockaddr_in recv_src = {0};

        socklen_t recv_src_len = sizeof(recv_src);

        int recvPacketLen = 0;

        if ((recvPacketLen = RecvWithinTime(sock, (char*)recv_buf, BUF_LEN, (struct sockaddr*)&recv_src, &recv_src_len, _timeout / 1000, (_timeout % 1000) * 1000)) == -1) {
            xerror2(TSF"receive dns query error.");
            break;
        }

        if (NULL != _traffic_monitor) {
            if (_traffic_monitor->recvLimitCheck(recvPacketLen)) {
                ret = TRAFFIC_LIMIT_RET_CODE;
                break;
            }
        }

        // move ahead of the dns header and the query field
        unsigned char* reader = &recv_buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION)];
        dns = (struct DNS_HEADER*)recv_buf;   // 指向recv_buf的header
        ReadRecvAnswer(recv_buf, dns, reader, answers);

        // 把查询到的IP放入返回参数_ipinfo结构体中
        int answer_count = std::min(SOCKET_MAX_IP_COUNT, (int)ntohs(dns->ans_count));
        _ipinfo->size = 0;

        for (int i = 0; i < answer_count; ++i) {
            if (1 == ntohs(answers[i].resource->type)) {  // IPv4 address
                in_addr_t* p = (in_addr_t*)answers[i].rdata;
                _ipinfo->ip[_ipinfo->size].s_addr = (*p);  // working without ntohl
                _ipinfo->size++;
            }
        }

        if (0 >= _ipinfo->size) {  // unkown host, dns->rcode == 3
            xerror2(TSF"unknown host.");
            break;
        }

        //      _ipinfo->dns = ;
        ret = 0;
    } while (false);

    FreeAll(answers);
    xinfo2(TSF"close fd in dnsquery,sock=%0", sock);
    ::socket_close(sock);
    return ret;  //* 查询DNS服务器超时
}

int socket_gethostbyname(const char* _host, socket_ipinfo_t* _ipinfo, int _timeout /*ms*/, const char* _dnsserver) {
    return socket_gethostbyname(_host, _ipinfo, _timeout, _dnsserver, NULL);
}
bool isValidIpAddress(const char* _ipaddress) {
    struct sockaddr_in sa;

    int result = socket_inet_pton(AF_INET, _ipaddress, (void*) & (sa.sin_addr));
    return result != 0;
}

void FreeAll(struct RES_RECORD* _answers) {
    int i;

    for (i = 0; i < SOCKET_MAX_IP_COUNT; i++) {
        if (_answers[i].name != NULL)
            free(_answers[i].name);

        if (_answers[i].rdata != NULL)
            free(_answers[i].rdata);
    }
}

void ReadRecvAnswer(unsigned char* _buf, struct DNS_HEADER* _dns, unsigned char* _reader, struct RES_RECORD* _answers) {
    // reading answers
    int i, j, stop = 0;
    int answer_count = std::min(SOCKET_MAX_IP_COUNT, (int)ntohs(_dns->ans_count));

    for (i = 0; i < answer_count; i++) {
        _answers[i].name = ReadName(_reader, _buf, &stop);
        _reader = _reader + stop;

        _answers[i].resource = (struct R_DATA*)(_reader);
        _reader = _reader + sizeof(struct R_DATA);

        if (ntohs(_answers[i].resource->type) == 1) {  // if its an ipv4 address
            _answers[i].rdata = (unsigned char*)malloc(ntohs(_answers[i].resource->data_len));

            if (NULL == _answers[i].rdata) {
                xerror2(TSF"answer error.");
                return;
            }

            for (j = 0 ; j < ntohs(_answers[i].resource->data_len) ; j++)
                _answers[i].rdata[j] = _reader[j];

            _answers[i].rdata[ntohs(_answers[i].resource->data_len)] = '\0';
            _reader = _reader + ntohs(_answers[i].resource->data_len);
        } else {
            _answers[i].rdata = ReadName(_reader, _buf, &stop);
            _reader = _reader + stop;
        }
    }
}

unsigned char* ReadName(unsigned char* _reader, unsigned char* _buffer, int* _count) {
    unsigned char* name;
    unsigned int p = 0, jumped = 0, offset;
    const unsigned int INIT_SIZE = 256, INCREMENT = 64;
    int timesForRealloc = 0;
    int i , j;

    *_count = 1;
    name   = (unsigned char*)malloc(INIT_SIZE);

    if (NULL == name) {
        xerror2(TSF"malloc error.");
        return NULL;
    }

    name[0] = '\0';

    // read the names in 3www6google3com format
    while (*_reader != 0) {
        if (*_reader >= 192) {  // 192 = 11000000 ,如果该字节前两位bit为11，则表示使用的是地址偏移来表示name
            offset = (*_reader) * 256 + *(_reader + 1) - 49152;  // 49152 = 11000000 00000000  计算相对于报文起始地址的偏移字节数，即去除两位为11的bit，剩下的14位表示的值
            _reader = _buffer + offset - 1;
            jumped = 1;  // we have jumped to another location so counting wont go up!
        } else
            name[p++] = *_reader;

        _reader = _reader + 1;

        if (jumped == 0) *_count = *_count + 1;  // if we have not jumped to another location then we can count up

        if (*_count >= (int)(INIT_SIZE + INCREMENT * timesForRealloc)) {
            timesForRealloc++;

            unsigned char* more_name = NULL;
            more_name = (unsigned char*)realloc(name, (INIT_SIZE + INCREMENT * timesForRealloc));

            if (NULL == more_name) {
                xerror2(TSF"realloc error.");
                free(name);
                return NULL;
            }

            name = more_name;
        }
    }

    name[p] = '\0';  // string complete

    if (jumped == 1) *_count = *_count + 1;  // number of steps we actually moved forward in the packet

    // now convert 3www6google3com0 to www.google.com
    for (i = 0; i < (int)strlen((const char*)name); i++) {
        p = name[i];

        for (j = 0; j < (int)p; j++) {
            name[i] = name[i + 1];
            i = i + 1;
        }

        name[i] = '.';
    }

    name[i - 1] = '\0';  // remove the last dot
    return name;
}

// this will convert www.google.com to 3www6google3com
void ChangetoDnsNameFormat(unsigned char* _qname, std::string _hostname) {
    int lock = 0 , i;
    _hostname.append(".");
    const char* host = _hostname.c_str();

    for (i = 0; i < (int)strlen(host); i++) {
        if (host[i] == '.') {
            *_qname++ = i - lock;

            for (; lock < i; lock++) {
                *_qname++ = host[lock];
            }

            lock++;
        }
    }

    *_qname++ = '\0';
}

void PrepareDnsQueryPacket(unsigned char* _buf, struct DNS_HEADER* _dns, unsigned char* _qname, const std::string& _host) {
    struct QUESTION*  qinfo = NULL;
    // Set the DNS structure to standard queries
    _dns->id = getpid();
    _dns->qr = 0;      // This is a query
    _dns->opcode = 0;  // This is a standard query
    _dns->aa = 0;      // Not Authoritative
    _dns->tc = 0;      // This message is not truncated
    _dns->rd = 1;      // Recursion Desired
    _dns->ra = 0;      // Recursion not available!
    _dns->z  = 0;
    _dns->ad = 0;
    _dns->cd = 0;
    _dns->rcode = 0;
    _dns->q_count = htons(1);   // we have only 1 question
    _dns->ans_count  = 0;
    _dns->auth_count = 0;
    _dns->add_count  = 0;
    // point to the query portion
    _qname = (unsigned char*)&_buf[sizeof(struct DNS_HEADER)];
    ChangetoDnsNameFormat(_qname, _host);  // 将传入的域名host转换为标准的DNS报文可用的格式，存入qname中
    qinfo = (struct QUESTION*)&_buf[sizeof(struct DNS_HEADER) + (strlen((const char*)_qname) + 1)];  // fill it

    qinfo->qtype = htons(1);  //只查询 ipv4 address
    qinfo->qclass = htons(1);  // its internet
}

int RecvWithinTime(int _fd, char* _buf, size_t _buf_n, struct sockaddr* _addr, socklen_t* _len, unsigned int _sec, unsigned _usec) {
    struct timeval tv;
    fd_set readfds, exceptfds;
    int n = 0;

    FD_ZERO(&readfds);
    FD_SET(_fd, &readfds);
    FD_ZERO(&exceptfds);
    FD_SET(_fd, &exceptfds);

    tv.tv_sec = _sec;
    tv.tv_usec = _usec;

    int ret = -1;
label:
    ret = select(_fd + 1, &readfds, NULL, &exceptfds, &tv);

    if (-1 == ret) {
        if (EINTR == errno) {
            // select被信号中断 handler
            FD_ZERO(&readfds);
            FD_SET(_fd, &readfds);
            FD_ZERO(&exceptfds);
            FD_SET(_fd, &exceptfds);
            goto label;
        }
    }

    if (FD_ISSET(_fd, &exceptfds)) {
        // socket异常处理
        xerror2(TSF"socket exception.");
        return -1;
    }

    if (FD_ISSET(_fd, &readfds)) {
        if ((n = (int)recvfrom(_fd, _buf, _buf_n, 0, _addr, _len)) >= 0) {
            return n;
        }
    }

    return -1;  // 超时或者select失败
}

#ifdef ANDROID

#include <sys/system_properties.h>
void GetHostDnsServerIP(std::vector<std::string>& _dns_servers) {
    char buf1[PROP_VALUE_MAX];
    char buf2[PROP_VALUE_MAX];
    __system_property_get("net.dns1", buf1);
    __system_property_get("net.dns2", buf2);
    _dns_servers.push_back(std::string(buf1));  // 主DNS
    _dns_servers.push_back(std::string(buf2));  // 备DNS
    xinfo2(TSF"main dns: %0", std::string(buf1).c_str());
    xinfo2(TSF"sub dns: %0", std::string(buf2).c_str());
}

#elif defined __APPLE__ 
#include <TargetConditionals.h>
#include <resolv.h>
#define RESOLV_CONFIG_PATH ("/etc/resolv.conf")
#if TARGET_OS_IPHONE
void GetHostDnsServerIP(std::vector<std::string>& _dns_servers) {
	_dns_servers.clear();
    std::ifstream fin(RESOLV_CONFIG_PATH);

    const int LINE_LENGTH = 256;
    char str[LINE_LENGTH];

    if (fin.good()) {
        while (!fin.eof()) {
            if (fin.getline(str, LINE_LENGTH).good()) {
                std::string s(str);
                int num = (int)s.find(NAME_SVR, 0);

                if (num >= 0) {
                    s.erase(std::remove_if(s.begin(), s.end(), isspace), s.end());
                    s = s.erase(0, NAME_SVR_LEN);
                    _dns_servers.push_back(s);
                }
            } else {
                break;
            }
        }
    } else {
        //  /etc/resolv.conf 不存在
        struct __res_state stat = {0};
        res_ninit(&stat);

//        if (stat.nsaddr_list != 0) {
            struct sockaddr_in nsaddr;

            for (int i = 0; i < stat.nscount; i++) {
                nsaddr = stat.nsaddr_list[i];
                const char* nsIP = socket_address(nsaddr).ip();

                if (NULL != nsIP)
                	_dns_servers.push_back(std::string(nsIP));
            }
//        }

        res_ndestroy(&stat);
    }
}
#else
void GetHostDnsServerIP(std::vector<std::string>& _dns_servers)
{
    //EMPTY FOR MAC
}
#endif //endif TARGET_OS_IPHONE
#elif defined WP8
void GetHostDnsServerIP(std::vector<std::string>& _dns_servers) {
}
#elif defined _WIN32
#include <stdio.h>
#include <windows.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")

void GetHostDnsServerIP(std::vector<std::string>& _dns_servers) {
    FIXED_INFO fi;
    ULONG ulOutBufLen = sizeof(fi);

    if (::GetNetworkParams(&fi, &ulOutBufLen) != ERROR_SUCCESS) {
        xinfo2(TSF" GetNetworkParams() failed");
        return;
    }

    IP_ADDR_STRING* pIPAddr = fi.DnsServerList.Next;

    while (pIPAddr != NULL) {
    	_dns_servers.push_back(pIPAddr->IpAddress.String);
        pIPAddr = pIPAddr->Next;
    }

    return;
}
#else

void GetHostDnsServerIP(std::vector<std::string>& _dns_servers) {
}

#endif

