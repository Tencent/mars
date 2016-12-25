/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * TcpQuery.h
 *
 *  Created on: 2014年6月30日
 *      Author: wutianqiang
 */

#ifndef SDT_SRC_CHECKIMPL_TCPQUERY_H_
#define SDT_SRC_CHECKIMPL_TCPQUERY_H_

#include "mars/sdt/sdt.h"

#include "mars/comm/thread/mutex.h"
#include "mars/comm/socket/unix_socket.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/socket/socketselect.h"

class NetCheckTrafficMonitor;

namespace mars {
namespace sdt {

class TcpQuery {

    enum TcpStatus {
        kTcpInit = 0,
        kTcpInitErr,
        kTcpConnectErr,
        kTcpConnected,
        kTcpDisConnected,
    };

  public:
    TcpQuery(const char* _ip, uint16_t _port, unsigned int connTimeout, NetCheckTrafficMonitor* trafficMonitor = NULL);
    virtual ~TcpQuery();

  public:
    TcpErrCode tcp_send(const unsigned char* buff, unsigned int unSize, int timeoutMs);
    TcpErrCode tcp_receive(AutoBuffer& recvBuf, unsigned int unSize, int timeoutMs);

    void send_break();
    std::string getStatus();

  private:
    char* ip_;
    uint16_t port_;
    SOCKET sock_;
    SocketSelectBreaker pipe_;
    SocketSelect select_;
    TcpStatus status_;
    int errcode_;
    unsigned int conn_timeout_;
};

}}

#endif /* SDT_SRC_CHECKIMPL_TCPQUERY_H_ */
