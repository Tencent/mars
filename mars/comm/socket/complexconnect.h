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
 * ComplexConnect.h
 *
 *  Created on: 2014-2-24
 *      Author: yerungui
 */

#ifndef COMPLEXCONNECT_H_
#define COMPLEXCONNECT_H_

#include <stddef.h>

#include <vector>

#include "comm_data.h"
#include "unix_socket.h"

class socket_address;
class AutoBuffer;

namespace mars {
namespace comm {

class SocketBreaker;

class MComplexConnect {
 public:
    virtual ~MComplexConnect() {
    }

    virtual void OnCreated(unsigned int _index, const socket_address& _addr, SOCKET _socket) {
    }
    virtual void OnConnect(unsigned int _index, const socket_address& _addr, SOCKET _socket) {
    }
    virtual void OnConnected(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _error, int _rtt) {
    }

    virtual bool OnShouldVerify(unsigned int _index, const socket_address& _addr) {
        return false;
    }
    virtual bool OnVerifySend(unsigned int _index,
                              const socket_address& _addr,
                              SOCKET _socket,
                              AutoBuffer& _buffer_send) {
        return false;
    }
    virtual bool OnVerifyRecv(unsigned int _index,
                              const socket_address& _addr,
                              SOCKET _socket,
                              const AutoBuffer& _buffer_recv) {
        return false;
    }
    virtual void OnVerifyTimeout(unsigned int _index, const socket_address& _addr, SOCKET _socket, int _timeout) {
    }

    virtual void OnFinished(unsigned int _index,
                            const socket_address& _addr,
                            SOCKET _socket,
                            int _error,
                            int _conn_rtt,
                            int _conn_totalcost,
                            int _complex_totalcost) {
    }
};

class ComplexConnect {
 public:
 public:
    ComplexConnect(unsigned int _timeout /*ms*/, unsigned int _interval /*ms*/);
    ComplexConnect(unsigned int _timeout /*ms*/,
                   unsigned int _interval /*ms*/,
                   unsigned int _error_interval /*ms*/,
                   unsigned int _max_connect);
    ComplexConnect(unsigned int _timeout /*ms*/,
                   unsigned int _interval /*ms*/,
                   unsigned int _error_interval /*ms*/,
                   unsigned int _v4_timeout,
                   unsigned int _v6_timeout);
    ~ComplexConnect();

    SOCKET ConnectImpatient(const std::vector<socket_address>& _vecaddr,
                            SocketBreaker& _breaker,
                            MComplexConnect* _observer = NULL,
                            mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone,
                            const socket_address* _proxy_addr = NULL,
                            const std::string& _proxy_username = "",
                            const std::string& _proxy_pwd = "");

    unsigned int TryCount() const {
        return trycount_;
    }
    int Index() const {
        return index_;
    }
    int ErrorCode() const {
        return errcode_;
    }

    unsigned int IndexRtt() const {
        return index_conn_rtt_;
    }
    unsigned int IndexTotalCost() const {
        return index_conn_totalcost_;
    }
    unsigned int TotalCost() const {
        return totalcost_;
    }
    bool IsInterrupted() const {
        return is_interrupted_;
    }
    bool IsConnectiveCheckFailed() const {
        return is_connective_check_failed_;
    }
    void SetNeedDetailLog(bool _need) {
        need_detail_log_ = _need;
    }

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
    int index_;              // used ip index
    int errcode_;            // errcode

    int index_conn_rtt_;
    int index_conn_totalcost_;
    int totalcost_;
    bool is_interrupted_;
    bool is_connective_check_failed_;
    bool need_detail_log_;
    unsigned int v4_timeout_;
    unsigned int v6_timeout_;
    bool indepent_timeout_;
};

}  // namespace comm
}  // namespace mars

#endif
