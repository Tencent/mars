/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * ComplexConnect.h
 *
 *  Created on: 2014-2-24
 *      Author: yerungui
 */

#ifndef COMPLEXCONNECT_H_
#define COMPLEXCONNECT_H_

#include <stddef.h>
#include <vector>

#include "unix_socket.h"

class SocketSelectBreaker;
class socket_address;
class AutoBuffer;

#ifdef COMPLEX_CONNECT_NAMESPACE
namespace COMPLEX_CONNECT_NAMESPACE {
#endif

class MComplexConnect {
  public:
    virtual ~MComplexConnect() {}

    virtual void OnCreated(unsigned int _index, const socket_address& _addr, SOCKET _socket) {}
    virtual void OnConnect(unsigned int _index, const socket_address& _addr, SOCKET _socket)  {}
    virtual void OnConnected(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _error, int _rtt) {}

    virtual bool OnShouldVerify(unsigned int _index, const socket_address& _addr) { return false;}
    virtual bool OnVerifySend(unsigned int _index, const socket_address& _addr, SOCKET _socket, AutoBuffer& _buffer_send) { return false;}
    virtual bool OnVerifyRecv(unsigned int _index, const socket_address& _addr, SOCKET _socket, const AutoBuffer& _buffer_recv) { return false;}
    virtual void OnVerifyTimeout(int _usedtime) {}

    virtual void OnFinished(unsigned int _index, const socket_address& _addr, SOCKET _socket,
                            int _error, int _conn_rtt, int _conn_totalcost, int _complex_totalcost) {}
};

class ComplexConnect {
  public:
    ComplexConnect(unsigned int _timeout /*ms*/, unsigned int _interval /*ms*/);
    ComplexConnect(unsigned int _timeout /*ms*/, unsigned int _interval /*ms*/, unsigned int _error_interval /*ms*/, unsigned int _max_connect);
    ~ComplexConnect();

    SOCKET ConnectImpatient(const std::vector<socket_address>& _vecaddr, SocketSelectBreaker& _breaker, MComplexConnect* _observer = NULL);

    unsigned int TryCount() const { return trycount_;}
    int Index() const { return index_;}
    int ErrorCode() const { return errcode_;}

    unsigned int IndexRtt() const { return index_conn_rtt_;}
    unsigned int IndexTotalCost() const { return index_conn_totalcost_;}
    unsigned int TotalCost() const { return totalcost_;}

  private:
    int __ConnectTime(unsigned int _index) const;
    int __ConnectTimeout(unsigned int _index) const;

  private:
    ComplexConnect(const ComplexConnect&);
    ComplexConnect& operator=(const ComplexConnect&);

  private:
    const unsigned int timeout_;
    const unsigned int interval_;
    const unsigned int error_interval_;
    const unsigned int max_connect_;

    unsigned int trycount_;  // tried ip count
    int index_;  // used ip index
    int errcode_;  // errcode

    int index_conn_rtt_;
    int index_conn_totalcost_;
    int totalcost_;
};

#ifdef COMPLEX_CONNECT_NAMESPACE
}
#endif

#endif
