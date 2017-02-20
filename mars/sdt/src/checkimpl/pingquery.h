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
 * pingquery.h
 *
 *  Created on: 2014/06/18
 *      Author: wutianqiang
 */

#ifndef SDT_SRC_CHECKIMPL_PINGQUERY_H_
#define SDT_SRC_CHECKIMPL_PINGQUERY_H_

#include <string>
#include <vector>

#include "boost/bind.hpp"

#include "mars/comm/socket/unix_socket.h"

#ifdef __APPLE__
#include "mars/comm/alarm.h"
#include "mars/comm/socket/socketselect.h"
#endif

class NetCheckTrafficMonitor;

#define DISALLOW_COPY_AND_ASSIGN(cls)    \
    private:\
    cls(const cls&);    \
    cls& operator=(const cls&);

namespace mars {
namespace sdt {

struct PingStatus {
    std::string res;
    double loss_rate;
    double minrtt;  // ms
    double avgrtt;  // ms
    double maxrtt;  // ms
    char ip[16];
};

class PingQuery {
  public:
    PingQuery(NetCheckTrafficMonitor* trafficMonitor = NULL): pingresult_("")
#ifdef __APPLE__
        , nsent_(0),
        sockfd_(-1),
        sendtimes_(0),
        sendcount_(0),
        readcount_(0),
        interval_(0),
        timeout_(0),
        alarm_(boost::bind(&PingQuery::__onAlarm, this), false),
        readwrite_breaker_()
#endif
        , traffic_monitor_(trafficMonitor)
    {}

    ~PingQuery() {
    }

  public:
    /**
     * return value:
     * 0---->success
     * -1--->error
     */
    int GetPingStatus(struct PingStatus& pingStatus);

    /**
     * return value:
     * 0---->success
     * -1--->error
     */
    int RunPingQuery(int queryCount, int interval/*S*/, int timeout/*S*/,
                       const char* dest, unsigned int packetSize = 0);

#ifdef __APPLE__
  private:
    void proc_v4(char* ptr, ssize_t len, struct msghdr* msg, struct timeval* tvrecv);
    int  __prepareSendAddr(const char* dest);
    int  __runReadWrite(int& errCode);
    void __onAlarm();
    void __preparePacket(char* sendbuffer, int& len);
    int  __send();
    int  __recv();
    int  __initialize(const char* dest);
    void  __deinitialize();
#endif

    DISALLOW_COPY_AND_ASSIGN(PingQuery);

  private:
    std::string                pingresult_;

#ifdef __APPLE__
    int                     nsent_;                /* add 1 for each sendto() */
    int                     sockfd_;
    std::vector<double>     vecrtts_;
    int                     sendtimes_;
    int                     sendcount_;
    int                     readcount_;
    int                     interval_;
    int                     timeout_;
    struct sockaddr          sendaddr_;
    struct sockaddr            recvaddr_;
    Alarm                   alarm_;
    SocketBreaker     readwrite_breaker_;
#endif
    NetCheckTrafficMonitor* traffic_monitor_;
};

}}

#endif /* SDT_SRC_CHECKIMPL_PINGQUERY_H_ */
