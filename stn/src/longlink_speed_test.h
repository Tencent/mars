/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * longlink_speed_test.h
 *
 *  Created on: 2013-5-13
 *      Author: yanguoyue
 */

#ifndef STN_SRC_LONGLINK_SPEED_TEST_H_
#define STN_SRC_LONGLINK_SPEED_TEST_H_

#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "mars/comm/autobuffer.h"
#include "mars/comm/socket/socketselect.h"
#include "mars/comm/socket/unix_socket.h"

#include "net_source.h"

enum ELongLinkSpeedTestState {
    kLongLinkSpeedTestConnecting,
    kLongLinkSpeedTestReq,
    kLongLinkSpeedTestResp,
    kLongLinkSpeedTestOOB,
    kLongLinkSpeedTestSuc,
    kLongLinkSpeedTestFail,
};

namespace mars {
    namespace stn {

class LongLinkSpeedTestItem {
  public:
    LongLinkSpeedTestItem(const std::string& _ip, uint16_t _port);
    ~LongLinkSpeedTestItem();

    void HandleFDISSet(SocketSelect& _sel);
    void HandleSetFD(SocketSelect& _sel);

    int GetSocket();
    std::string GetIP();
    unsigned int GetPort();
    unsigned long GetConnectTime();
    int GetState();

    void CloseSocket();

  private:
    int __HandleSpeedTestReq();
    int __HandleSpeedTestResp();

  private:
    std::string ip_;
    unsigned int port_;
    SOCKET socket_;
    int state_;

    uint64_t before_connect_time_;
    uint64_t after_connect_time_;

    AutoBuffer req_ab_;
    AutoBuffer resp_ab_;
};

class LongLinkSpeedTest {
  public:
    LongLinkSpeedTest(const boost::shared_ptr<NetSource>& _netsource);
    ~LongLinkSpeedTest();

    bool GetFastestSocket(int& _fdSocket, std::string& _strIp, unsigned int& _port, IPSourceType& _type, unsigned long& _connectMillSec);

    boost::shared_ptr<NetSource> GetNetSource();
  private:
    boost::shared_ptr<NetSource> netsource_;
    SocketSelectBreaker breaker_;
    SocketSelect selector_;
};
        
    }
}


#endif // STN_SRC_LONGLINK_SPEED_TEST_H_
