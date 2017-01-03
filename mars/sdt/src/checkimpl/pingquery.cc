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
 * pingquery.cc
 *
 *  Created on: 2014年6月18日
 *      Author: wutianqiang
 */

#include "pingquery.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/network/getgateway.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/socket/socket_address.h"
#include "mars/sdt/constants.h"

#include "netchecker_trafficmonitor.h"

using namespace mars::sdt;

#define TRAFFIC_LIMIT_RET_CODE (INT_MIN)

static void clearPingStatus(struct PingStatus& _ping_status) {
    _ping_status.res.clear();
    _ping_status.loss_rate = 0.0;
    _ping_status.maxrtt = 0.0;
    _ping_status.minrtt = 0.0;
    _ping_status.avgrtt = 0.0;
    memset(_ping_status.ip, 0, 16);
}
#ifdef ANDROID

#define MAXLINE (512) /* max text line length */

void str_split(char _spliter, std::string _pingresult, std::vector<std::string>& _vec_pingres) {
    int find_begpos = 0;
    int findpos = 0;

    while ((unsigned int)findpos < _pingresult.length()) {
    	findpos = _pingresult.find_first_of(_spliter, find_begpos);
        _vec_pingres.push_back(std::string(_pingresult, find_begpos, findpos - find_begpos));
        find_begpos = findpos + 1;
    }
}

int PingQuery::RunPingQuery(int _querycount, int interval/*S*/, int timeout/*S*/, const char* dest, unsigned int packetSize) {  // use popen
    xinfo2(TSF"in runpingquery");
    xassert2(_querycount >= 0, "ping count should be more than 0");
    xassert2(interval >= 0, "interval should be more than 0");
    xassert2(timeout >= 0, "timeout should be more than 0");

    if (_querycount == 0)
    	_querycount = DEFAULT_PING_COUNT;

    if (interval == 0)
        interval = DEFAULT_PING_INTERVAL;

    if (timeout == 0)
        timeout = DEFAULT_PING_TIMEOUT;

    if (NULL == dest || 0 == strlen(dest)) {
        struct  in_addr _addr;
        int ret = getdefaultgateway(&_addr);

        if (-1 == ret) {
            xerror2(TSF"get default gateway error.");
            return -1;
        }

        dest = socket_address(_addr).ip();

        if (NULL == dest || 0 == strlen(dest)) {
            xerror2(TSF"ping dest host is NULL.");
            return -1;
        }

        xinfo2(TSF"get default gateway: %0", dest);
    }

    char line[MAXLINE] = {0};
    pingresult_.clear();

    if (NULL != traffic_monitor_) {
        int sendLen = (packetSize > 0 ? packetSize : 56) * _querycount;

        if (traffic_monitor_->sendLimitCheck(sendLen)) {  // 56 is default packet size
            xwarn2(TSF"limitCheck!!!sendLen=%0", sendLen);
            return TRAFFIC_LIMIT_RET_CODE;
        }
    }

    char cmd[256] = {0};

    if (strlen(dest) > 200) {
        xerror2(TSF"domain name is too long.");
        return -1;
    }

    int index = snprintf(cmd, 256, "ping -c %d -i %d -w %d", _querycount, interval, timeout);

    if (index < 0 || index >= 256) {
        xerror2(TSF"sprintf return error.index=%_", index);
        return -1;
    }

    int tempLen = 0;

    if (packetSize > 0) {
        tempLen = snprintf((char*)&cmd[index], 256 - index, " -s %u  %s", packetSize, dest);
    } else {
        tempLen = snprintf((char*)&cmd[index], 256 - index, " %s", dest);
    }

    if (tempLen < 0 || tempLen >= 256 - index) {
        xerror2(TSF"sprintf return error.tempLen=%_, index=%_", tempLen, index);
        return -1;
    }

    xinfo2(TSF"popen cmd=%0", cmd);
    FILE* pp = popen(cmd, "r");

    if (!pp) {
        xerror2(TSF"popen error:%0", strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), pp) != NULL) {
        pingresult_.append(line, strlen(line));
    }

    pclose(pp);

    if (pingresult_.empty()) {
        xerror2(TSF"m_strPingResult is empty");
        return -1;
    }

    struct PingStatus pingStatusTemp;  //= {0};notice: cannot initial with = {0},crash

    clearPingStatus(pingStatusTemp);

    GetPingStatus(pingStatusTemp);

    if (0 == pingStatusTemp.avgrtt && 0 == pingStatusTemp.maxrtt) {
        xinfo2(TSF"remote host is not available");
        return -1;
    }

    xinfo2(TSF"m_strPingResult = %0", pingresult_);
    return 0;
}

int PingQuery::GetPingStatus(struct PingStatus& _ping_status) {
    xinfo_function();
    clearPingStatus(_ping_status);

    if (pingresult_.empty())  return -1;

    _ping_status.res = pingresult_;  //
    std::vector<std::string> vecPingRes;
    str_split('\n', pingresult_, vecPingRes);

    std::vector<std::string>::iterator iter = vecPingRes.begin();

    for (; iter != vecPingRes.end(); ++iter) {
        if (vecPingRes.begin() == iter) {  // extract ip from the result string and assign to _ping_status.ip
            int index1 = iter->find_first_of("(", 0);

            if (index1 > 0) {
                int index2 = iter->find_first_of(")", 0);

                if (index2 > index1) {
                    int size = index2 - index1 - 1;
                    std::string ipTemp(iter->substr(index1 + 1, size));
                    xinfo2(TSF"ipTemp=%_, size = %_", ipTemp.c_str(), size);
                    xassert2(size <= 16 && size > 0);
                    strncpy(_ping_status.ip, ipTemp.c_str(), (size < 16 ? size : 15));
                    xdebug2(TSF"_ping_status.ip=%_", _ping_status.ip);
                }
            }
        }  // end if(vecPingRes.begin()==iter)

        int num = iter->find("packet loss", 0);

        if (num >= 0) {
            int loss_rate = 0;
            int i = 3;

            while (iter->at(num - i) != ' ') {
                loss_rate += ((iter->at(num - i) - '0') * (int)pow(10.0, (double)(i - 3)));
                i++;
            }

            _ping_status.loss_rate  = (double)loss_rate / 100;
        }

        int num2 = iter->find("rtt min/avg/max", 0);

        if (num2 >= 0) {
            int find_begpos = 23;
            int findpos = iter->find_first_of('/', find_begpos);
            std::string sminRTT(*iter, find_begpos, findpos - find_begpos);
            find_begpos = findpos + 1;
            findpos = iter->find_first_of('/', find_begpos);
            std::string savgRTT(*iter, find_begpos, findpos - find_begpos);
            find_begpos = findpos + 1;
            findpos = iter->find_first_of('/', find_begpos);
            std::string smaxRTT(*iter, find_begpos, findpos - find_begpos);

            _ping_status.minrtt = atof(sminRTT.c_str());
            _ping_status.avgrtt = atof(savgRTT.c_str());
            _ping_status.maxrtt = atof(smaxRTT.c_str());
        }
    }

    return 0;
}


#elif defined __APPLE__

// APPLE
#include    <netinet/ip.h>
#include    <sys/time.h>
#include    <sys/un.h>
#include    <arpa/inet.h>
#include    <signal.h>
#include    <netinet/in_systm.h>
#include    <netinet/ip.h>
#include    <sys/types.h>
#include    <time.h>
#include    <sys/socket.h>
#include    <netdb.h>

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include    "mars/comm/objc/ip_icmp.h"
#else
#include    <netinet/ip_icmp.h>
#endif

#include "mars/comm/time_utils.h"  // comm/utils.h
#define MAXBUFSIZE      4096

static int DATALEN = 56;        /* data that goes with ICMP echo request */
static const int IP_HEADER_LEN = 20;
static const int ICMP_HEADER_LEN = 8;

static char* sock_ntop_host(const struct sockaddr* sa, socklen_t salen) {
    static char str[128];   /* Unix domain is largest */

    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in* sin = (struct sockaddr_in*)sa;

        if (socket_inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))
                == NULL)
            return (NULL);

        return (str);
    }

#ifdef  IPV6

    case AF_INET6: {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)sa;

        if (inet_ntop
                (AF_INET6, &sin6->sin6_addr, str,
                 sizeof(str)) == NULL)
            return (NULL);

        return (str);
    }

#endif



#ifdef  AF_UNIX

    case AF_UNIX: {
        struct sockaddr_un* unp = (struct sockaddr_un*)sa;

        /* OK to have no pathname bound to the socket: happens on
           every connect() unless client calls bind() first. */
        if (unp->sun_path[0] == 0)
            strcpy(str, "(no pathname bound)");
        else
            snprintf(str, sizeof(str), "%s", unp->sun_path);

        return (str);
    }

#endif



#ifdef  HAVE_SOCKADDR_DL_STRUCT

    case AF_LINK: {
        struct sockaddr_dl* sdl = (struct sockaddr_dl*)sa;

        if (sdl->sdl_nlen > 0)
            snprintf(str, sizeof(str), "%*s",
                     sdl->sdl_nlen, &sdl->sdl_data[0]);
        else
            snprintf(str, sizeof(str), "AF_LINK, index=%d",
                     sdl->sdl_index);

        return (str);
    }

#endif

    default:
        snprintf(str, sizeof(str),
                 "sock_ntop_host: unknown AF_xxx: %d, len %d",
                 sa->sa_family, salen);
        return (str);
    }

    return (NULL);
}

static char* Sock_ntop_host(const struct sockaddr* sa, socklen_t salen) {
    char* ptr;

    if ((ptr = sock_ntop_host(sa, salen)) == NULL) {
        xerror2(TSF"sock_ntop_host error,errno=%0", errno); /* inet_ntop() sets errno */
    }

    return (ptr);
}
static void Gettimeofday(struct timeval* tv, void* foo) {
    if (gettimeofday(tv, (struct timezone*)foo) == -1) {
        xerror2(TSF"gettimeofday error");
    }

    return;
}
static int Sendto(int fd, const void* ptr, size_t nbytes, int flags, const struct sockaddr* sa, socklen_t salen) {
    xdebug_function();
    int len = 0;

    if ((len = (int)sendto(fd, ptr, nbytes, flags, sa, salen)) != (ssize_t) nbytes) {
        xerror2(TSF"sendto: uncomplete packet");
    }

    return len;
}


static struct addrinfo* Host_serv(const char* host, const char* serv, int family, int socktype) {
    int n;
    struct addrinfo hints, *res;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;  /* always return canonical name */
    hints.ai_family = family;   /* 0, AF_INET, AF_INET6, etc. */
    hints.ai_socktype = socktype;   /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
        xerror2(TSF"host_serv error for %0, %1: %2",
                ((host == NULL) ? "(no hostname)" : host),
                ((serv == NULL) ? "(no service name)" : serv),
                gai_strerror(n));
        return NULL;
    }

    return (res);       /* return pointer to first on linked list */
}


static int Socket(int family, int type, int protocol) {
    int n;

    if ((n = socket(family, type, protocol)) < 0) {
        xerror2(TSF"socket error");
    }

    return (n);
}


static uint16_t in_cksum(uint16_t* _addr, int _len) {
    int             nleft = _len;
    uint32_t        sum = 0;
    uint16_t*        w = _addr;
    uint16_t        answer = 0;

    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* 4mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }

    /* 4add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}

static void tv_sub(struct timeval* _out, struct timeval* _in) {
    if ((_out->tv_usec -= _in->tv_usec) < 0) {     /* out -= in */
        --_out->tv_sec;
        _out->tv_usec += 1000000;
    }

    _out->tv_sec -= _in->tv_sec;
}

void PingQuery::proc_v4(char* _ptr, ssize_t _len, struct msghdr* _msg, struct timeval* _tvrecv) {
    int     icmplen;
    double      rtt;
    struct icmp* icmp;
    struct timeval*  tvsend;
    icmp = (struct icmp*) _ptr;

    if ((icmplen = (int)_len - IP_HEADER_LEN) < ICMP_HEADER_LEN) {
        xerror2(TSF"receive malformed icmp packet");
        return;             /* malformed packet */
    }

    // if (icmp->icmp_type == ICMP_ECHOREPLY)
    //  {
    xdebug2(TSF"icmp->icmp_type=%0,is equal with ICMP_ECHOREPLY:%1", icmp->icmp_type, icmp->icmp_type == ICMP_ECHOREPLY);

    if (icmplen < ICMP_HEADER_LEN + sizeof(struct timeval)) {
        xerror2(TSF"not enough data to compute RTT");
        return;         /* not enough data to use */
    }

    tvsend = (struct timeval*)(&_ptr[ICMP_MINLEN]);
    xdebug2(TSF"before ntohl tvsend sec=%_, nsec=%_; tvrecv sec=%_, usec=%_", tvsend->tv_sec
            , tvsend->tv_usec, _tvrecv->tv_sec, _tvrecv->tv_usec);

    tvsend->tv_sec = ntohl(tvsend->tv_sec);
    tvsend->tv_usec = ntohl(tvsend->tv_usec);

    xdebug2(TSF"tvsend sec=%_, nsec=%_; tvrecv sec=%_, usec=%_", tvsend->tv_sec
            , tvsend->tv_usec, _tvrecv->tv_sec, _tvrecv->tv_usec);

    tv_sub(_tvrecv, tvsend);
    rtt = _tvrecv->tv_sec * 1000.0 + _tvrecv->tv_usec / 1000.0;

    if (rtt < 10000.0 && rtt > 0.0) {
        vecrtts_.push_back(rtt);
    } else {
        xerror2(TSF"rtt = %0 is illegal.receive %1 bytes from %2", rtt, icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)));
    }

    char tempbuff[1024] = {0};
    snprintf(tempbuff, 1024, "%d bytes from %s: seq=%d,  rtt=%f ms\n",
             icmplen, Sock_ntop_host(&recvaddr_, sizeof(recvaddr_)),
             ntohs(icmp->icmp_seq), rtt);
    xinfo2(TSF"%_", (char*)tempbuff);
    pingresult_.append(tempbuff);
    //   }
}


int PingQuery::__prepareSendAddr(const char* _dest) {
    struct addrinfo* ai;
    char* h;
    const char* host = _dest;

    ai = Host_serv(host, NULL, 0, 0);

    if (NULL == ai) return -1;

    h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
    xinfo2(TSF"PING %0 (%1): %2 data bytes\n", (ai->ai_canonname ? ai->ai_canonname : h), h, DATALEN);

    if (ai->ai_family != AF_INET) {
        xinfo2(TSF"unknown address family %0\n", ai->ai_family);
        return -1;
    }

    memcpy(&sendaddr_, ai->ai_addr, sizeof(struct sockaddr));
    xdebug2(TSF"m_sendAddr=%0", socket_address(&sendaddr_).ip());
    freeaddrinfo(ai);  // 閲婃斁addrinfo鍐呴儴瀛楁malloc鐨勫唴瀛橈紙鐢眊etaddrinfo鍑芥暟鍐呴儴浜х敓锛�
    return 0;
}

int PingQuery::__initialize(const char* _dest) {
    if (-1 == __prepareSendAddr(_dest)) return -1;;

    sockfd_ = Socket(sendaddr_.sa_family, SOCK_DGRAM/*SOCK_RAW*/, IPPROTO_ICMP);

    if (sockfd_ < 0) return -1;

    int size = 60 * 1024;       /* OK if setsockopt fails */
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    // make nonblock socket
    if (0 != socket_ipv6only(sockfd_, 0)){
        xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno));
    }
    int ret = ::socket_set_nobio(sockfd_);

    if (ret != 0) {
        xerror2(TSF"__initialize():set nonblock socket error:%0", socket_strerror(socket_errno));
        return -1;
    }

    return 0;
}
void PingQuery::__deinitialize() {
    if (sockfd_ >= 0) {
        ::socket_close(sockfd_);
    }
}
int PingQuery::__recv() {
    char            recvbuf[MAXBUFSIZE];
    char            controlbuf[MAXBUFSIZE];
    memset(recvbuf, 0, MAXBUFSIZE);
    memset(controlbuf, 0, MAXBUFSIZE);

    struct msghdr   msg = {0};
    struct iovec    iov = {0};


    iov.iov_base = recvbuf;
    iov.iov_len = sizeof(recvbuf);
    msg.msg_name = &recvaddr_;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = controlbuf;

    msg.msg_namelen = sizeof(recvaddr_);
    msg.msg_controllen = sizeof(controlbuf);

    int n = (int)recvmsg(sockfd_, &msg, 0);

    if (NULL != traffic_monitor_) {
        if (traffic_monitor_->recvLimitCheck(n)) {
            xwarn2(TSF"limitCheck,recv Size=%0", n);
            return TRAFFIC_LIMIT_RET_CODE;
        }
    }

    xinfo2(TSF"after recvmsg() n =%0\n", (int)n);

    if (n < 0) {
        return -1;
    }

    struct timeval  tval;

    Gettimeofday(&tval, NULL);

    xdebug2(TSF"gettimeofday sec=%0,usec=%1", tval.tv_sec, tval.tv_usec);

    proc_v4(recvbuf + IP_HEADER_LEN, n, &msg, &tval);  // 杩欎釜闀垮害n锛屽寘鍚�20涓瓧鑺傜殑ip澶�

    return n;
}

int PingQuery::__send() {
    char sendbuffer[MAXBUFSIZE];
    memset(sendbuffer, 0, MAXBUFSIZE);
    int len = 0;
    __preparePacket(sendbuffer, len);

    if (NULL != traffic_monitor_) {
        if (traffic_monitor_->sendLimitCheck(len)) {
            xwarn2(TSF"limitCheck!!len=%0", len);
            return TRAFFIC_LIMIT_RET_CODE;
        }
    }

    int sendLen = Sendto(sockfd_, sendbuffer, len, 0, &sendaddr_, sizeof(sendaddr_));
    sendtimes_++;

    return sendLen;
}

void PingQuery::__preparePacket(char* _sendbuffer, int& _len) {
    char    sendbuf[MAXBUFSIZE];
    memset(sendbuf, 0, MAXBUFSIZE);
    struct icmp* icmp;
    icmp = (struct icmp*) sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = getpid() & 0xffff;/* ICMP ID field is 16 bits */
    icmp->icmp_seq = htons(nsent_++);
    memset(&sendbuf[ICMP_MINLEN], 0xa5, DATALEN);   /* fill with pattern */

    struct timeval now;
    (void)gettimeofday(&now, NULL);
    xdebug2(TSF"gettimeofday now sec=%0, nsec=%1", now.tv_sec, now.tv_usec);
    now.tv_usec = htonl(now.tv_usec);
    now.tv_sec = htonl(now.tv_sec);
    bcopy((void*)&now, (void*)&sendbuf[ICMP_MINLEN], sizeof(now));
    _len = ICMP_MINLEN + DATALEN;        /* checksum ICMP header and data */
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((u_short*) icmp, _len);
    memcpy(_sendbuffer, sendbuf, _len);
}

void PingQuery::__onAlarm() {
    readwrite_breaker_.Break();
}

// return :-1 no send all packets
//         0 send all packets
int PingQuery::__runReadWrite(int& _errcode) {
    unsigned long timeout_point = timeout_ * 1000 + gettickcount();
    unsigned long send_next = 0;

    while (readcount_ > 0) {
        bool    should_send = false;

        if (send_next <= gettickcount() && sendcount_ > 0) {
            send_next = gettickcount() + interval_ * 1000;
            alarm_.Cancel();
            alarm_.Start(interval_ * 1000);  // m_interval*1000 convert m_interval from s to ms
            should_send = true;
        }

        SocketSelect sel(readwrite_breaker_, true);
        sel.PreSelect();
        sel.Read_FD_SET(sockfd_);
        sel.Exception_FD_SET(sockfd_);

        if (sendcount_ > 0) sel.Write_FD_SET(sockfd_);

        long timeoutMs = timeout_point - gettickcount();

        if (timeoutMs < 0) {
            return -1;  //设置的超时时间内没有收完所有的包
        }

        int retsel = sel.Select((int)timeoutMs);

        if (retsel < 0) {
            xerror2(TSF"retSel<0");
            _errcode = sel.Errno();
            return -1;
        }

        if (sel.IsException()) {
            xerror2(TSF"socketselect exception");
            _errcode = socket_error(sockfd_);
            return -1;
        }

        if (sel.Exception_FD_ISSET(sockfd_)) {
            _errcode = socket_error(sockfd_);

            return -1;
        }

        if (sel.Write_FD_ISSET(sockfd_) && should_send) {
            int sendLen = __send();

            if (TRAFFIC_LIMIT_RET_CODE == sendLen) {
                return TRAFFIC_LIMIT_RET_CODE;
            }

            if (sendLen < 0) {
                _errcode = socket_error(sockfd_);
            }

            sendcount_--;
        }

        if (sel.Read_FD_ISSET(sockfd_) && readcount_ > 0) {
            if (TRAFFIC_LIMIT_RET_CODE == __recv()) {
                readcount_--;
                return TRAFFIC_LIMIT_RET_CODE;
            }

            readcount_--;
        }
    }

    return 0;
}

int PingQuery::RunPingQuery(int _querycount, int _interval/*S*/, int _timeout/*S*/, const char* _dest, unsigned int _packet_size) {
    xassert2(_querycount >= 0);
    xdebug2(TSF"dest=%0", _dest);

    if (_querycount <= 0)
        _querycount = 4;

    if (_interval <= 0)
        _interval = 1;

    if (_timeout <= 0)
        _timeout = 5;

    if (_packet_size >= ICMP_MINLEN && _packet_size <= MAXBUFSIZE/*4096*/) {  // packetSize is the length of ICMP packet, include ICMP header,but not include IP header
        DATALEN = _packet_size - ICMP_MINLEN;
    }

    if (NULL == _dest || 0 == strlen(_dest)) {
        struct  in_addr _addr;
        int ret = getdefaultgateway(&_addr);

        if (-1 == ret) {
            xerror2(TSF"get default gateway error.");
            return -1;
        }

        _dest = inet_ntoa(_addr);

        if (NULL == _dest || 0 == strlen(_dest)) {
            xerror2(TSF"ping dest host is NULL.");
            return -1;
        }

        xinfo2(TSF"get default gateway: %0", _dest);
    }

    sendcount_ = _querycount;
    readcount_ = _querycount;
    interval_ = _interval;
    timeout_  = _timeout;

    if (-1 == __initialize(_dest)) {
        __deinitialize();
        return -1;
    }

    int errcode;
    int ret = __runReadWrite(errcode);
    __deinitialize();

    //  if(0 >= m_vecRTTs.size())  return -1;
    return ret;
}

int PingQuery::GetPingStatus(struct PingStatus& _ping_status) {
    clearPingStatus(_ping_status);
    int size = (int)vecrtts_.size();
    const char* pingIP = socket_address(&sendaddr_).ip();
    xdebug2(TSF"pingIP=%0", pingIP);

    if (pingIP != NULL) {
        strncpy(_ping_status.ip, pingIP, 16);
    } else {
        xerror2(TSF"pingIP==NULL");
    }

    xinfo2(TSF"getPingStatus():size = %0; m_readCount=%1,m_sendTimes=%2", size, readcount_, sendtimes_);

    _ping_status.loss_rate = (1 - (double)size / sendtimes_) < 0 ? 0 : (1 - (double)size / sendtimes_);
    char temp[1024] = {0};
    snprintf(temp, 1024, "\n%d packets transmitted,%d packets received,lossRate=%f%%.\n ", sendtimes_, size, _ping_status.loss_rate * 100.0);
    pingresult_.append(std::string(temp));

    if (size > 0) {
        _ping_status.minrtt = vecrtts_.at(0);
        _ping_status.maxrtt = vecrtts_.at(0);
        std::vector<double>::iterator iter = vecrtts_.begin();
        double sum = 0.0;

        for (; iter != vecrtts_.end(); ++iter) {
            if (_ping_status.minrtt > *iter)
                _ping_status.minrtt = *iter;

            if (_ping_status.maxrtt < *iter)
                _ping_status.maxrtt = *iter;

            sum += *iter;
        }

        _ping_status.avgrtt = sum / size;
    } else {
        _ping_status.res = pingresult_;
        return -1;
    }

    memset(temp, 0, 1024);
    snprintf(temp, 1024, " MaxRTT=%f ms, MinRTT=%f ms, AverageRTT=%f ms", _ping_status.maxrtt, _ping_status.minrtt, _ping_status.avgrtt);
    pingresult_.append(std::string(temp));
    _ping_status.res = pingresult_;
    return 0;
}


int doPing(const std::string& _destaddr, std::string& _real_pingip, std::string& _resultstr, unsigned int _packet_size, int _pingcount, int _interval/*s*/, int _timeout/*s*/) {
    PingQuery pingObj;
    int ret = pingObj.RunPingQuery(_pingcount, _interval, _timeout, _destaddr.c_str(), _packet_size);

    if (ret != 0) {
        xinfo2(TSF"ret=%0", ret);
        return ret;
    }

    struct PingStatus ping_status;

    int ret2 = pingObj.GetPingStatus(ping_status);


    _real_pingip.clear();

    _real_pingip.append(ping_status.ip);

    _resultstr.clear();

    _resultstr.append(ping_status.res);

    xdebug2(TSF"realPingIP=%0,resultStr=%1,destAddr=%2", _real_pingip.c_str(), _resultstr.c_str(), _destaddr.c_str());

    return ret2;
}
#elif defined _WIN32
// empty implement
int PingQuery::RunPingQuery(int _querycount, int interval/*S*/, int timeout/*S*/, const char* dest, unsigned int packetSize) {
    xerror2(TSF"ping query is not support on win32 now!");
    return 0;
}
int PingQuery::GetPingStatus(struct PingStatus& pingStatus) {
    xerror2(TSF"ping query is not support on win32 now!");
    return 0;
}
#else

int PingQuery::RunPingQuery(int _querycount, int interval/*S*/, int timeout/*S*/, const char* dest, unsigned int packetSize) {
    xerror2(TSF"ping query is not support  now!");
    return 0;
}
int PingQuery::GetPingStatus(struct PingStatus& pingStatus) {
    xerror2(TSF"ping query is not support now!");
    return 0;
}
// #error "no support!"

#endif

