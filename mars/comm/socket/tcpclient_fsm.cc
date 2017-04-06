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
 * TcpClientFSM.cpp
 *
 *  Created on: 2014-8-19
 *      Author: yerungui
 */

#include "tcpclient_fsm.h"

#include <stdlib.h>
#include <limits.h>

#include "comm/xlogger/xlogger.h"
#include "comm/socket/socketselect.h"
#include "comm/socket/unix_socket.h"
#include "comm/time_utils.h"

#include "comm/platform_comm.h"

TcpClientFSM::TcpClientFSM(const sockaddr& _addr):addr_(&_addr) {
    status_ = EStart;
    last_status_ = EStart;
    error_ = 0;
    remote_close_ = false;
    request_send_ = false;

    sock_ = INVALID_SOCKET;
    start_connecttime_ = 0;
    end_connecttime_ = 0;
}

TcpClientFSM::~TcpClientFSM() {
    Close(false);
    xassert2(INVALID_SOCKET == sock_, "%d", sock_);
}

void TcpClientFSM::RequestSend() {
    request_send_ = true;
}

TcpClientFSM::TSocketStatus TcpClientFSM::Status() const {
    return status_;
}

void TcpClientFSM::Status(TSocketStatus _status) {
    last_status_ = _status;
    status_ = _status;
}

TcpClientFSM::TSocketStatus TcpClientFSM::LastStatus() const {
    return last_status_;
}

int TcpClientFSM::Error() const {
    return error_;
}

bool TcpClientFSM::IsEndStatus() const {
    return EEnd == status_;
}

SOCKET TcpClientFSM::Socket() const {
    return sock_;
}

void TcpClientFSM::Socket(SOCKET _sock) {
    sock_ = _sock;
}

const sockaddr& TcpClientFSM::Address() const {
    return addr_.address();
}

const char* TcpClientFSM::IP() const {
    return addr_.ip();
}

uint16_t TcpClientFSM::Port() const {
    return addr_.port();
}

void TcpClientFSM::Close(bool _notify) {
    if (INVALID_SOCKET == sock_) return;

    if (remote_close_ || 0 != error_) {
        socket_close(sock_);
        sock_ = INVALID_SOCKET;
        return;
    }

    xinfo2(TSF"sock:%_, (%_:%_), close local socket close, notify:%_", sock_, addr_.ip(), addr_.port(), _notify);

    socket_close(sock_);
    sock_ = INVALID_SOCKET;

    last_status_ = status_;
    status_ = EEnd;
    error_ = 0;

    if (_notify) _OnClose(last_status_, 0, remote_close_);
}

bool TcpClientFSM::RemoteClose() const {
    return remote_close_;
}

void TcpClientFSM::PreSelect(SocketSelect& _sel, XLogger& _log) {
    
    switch(status_) {
        case EStart: {
            PreConnectSelect(_sel, _log);
            break;
        }
        case EConnecting: {
            _sel.Write_FD_SET(sock_);
            _sel.Exception_FD_SET(sock_);
            break;
        }
        case EReadWrite: {
            PreReadWriteSelect(_sel, _log);
            break;
        }
        default:
            xassert2(false, "preselect status error");
    }
}

void TcpClientFSM::AfterSelect(SocketSelect& _sel, XLogger& _log) {
    if (EConnecting == status_) AfterConnectSelect(_sel, _log);
    else if (EReadWrite == status_)  AfterReadWriteSelect(_sel, _log);

    if (EEnd == status_ && INVALID_SOCKET != sock_) {
        _OnClose(last_status_, error_, false);
    }
}

int TcpClientFSM::Timeout() const {
    if (EConnecting == status_) return ConnectTimeout();

    if (EReadWrite == status_) return ReadWriteTimeout();

    if (EEnd == status_) return 0;

    return INT_MAX;
}

void TcpClientFSM::PreConnectSelect(SocketSelect& _sel, XLogger& _log) {
    xassert2(EStart == status_, "%d", status_);
    _OnCreate();

    xinfo2(TSF"addr:(%_:%_), ", addr_.ip(), addr_.port()) >> _log;

    sock_ = socket(addr_.address().sa_family, SOCK_STREAM, IPPROTO_TCP);

    if (sock_ == INVALID_SOCKET) {
        error_ = socket_errno;
        last_status_ = status_;
        status_ = EEnd;
        _OnClose(last_status_, error_, false);
        xerror2(TSF"close socket err:(%_, %_)", error_, socket_strerror(error_)) >> _log;
        return;
    }

    if (::getNetInfo() == kWifi && socket_fix_tcp_mss(sock_) < 0) {
#ifdef ANDROID
        xinfo2(TSF"wifi set tcp mss error:%0", strerror(socket_errno));
#endif
    }
    
#ifdef _WIN32
    if (0 != socket_ipv6only(sock_, 0)){ xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno)); }
#endif
    
    if (0 != socket_set_nobio(sock_)) {
        error_ = socket_errno;
        xerror2(TSF"close socket_set_nobio:(%_, %_)", error_, socket_strerror(error_)) >> _log;
    } else {
        xinfo2(TSF"socket:%_, ", sock_) >> _log;
    }

    if (0 != error_) {
        last_status_ = status_;
        status_ = EEnd;
        return;
    }

    start_connecttime_ = gettickcount();

    int ret = connect(sock_, &(addr_.address()), addr_.address_length());

    if (0 != ret && !IS_NOBLOCK_CONNECT_ERRNO(socket_errno)) {
        end_connecttime_ = ::gettickcount();

        error_ = socket_errno;
        xwarn2(TSF"close connect err:(%_, %_), localip:%_", error_, socket_strerror(error_), socket_address::getsockname(sock_).ip()) >> _log;
    } else {
        xinfo2("connect") >> _log;
        _sel.Write_FD_SET(sock_);
        _sel.Exception_FD_SET(sock_);
    }

    last_status_ = status_;

    if (0 != error_)
        status_ = EEnd;
    else
        status_ = EConnecting;

    if (0 == error_) _OnConnect();
}

void TcpClientFSM::AfterConnectSelect(const SocketSelect& _sel, XLogger& _log) {
    xassert2(EConnecting == status_, "%d", status_);

    int timeout = ConnectTimeout();
    xinfo2(TSF"sock:%_, (%_:%_), ", sock_, addr_.ip(), addr_.port()) >> _log;

    if (_sel.Exception_FD_ISSET(sock_)) {
        socklen_t len = sizeof(error_);

        if (0 != getsockopt(sock_, SOL_SOCKET, SO_ERROR, &error_, &len)) { error_ = socket_errno; }

        xwarn2(TSF"close connect exception: (%_, %_)", sock_, error_, socket_strerror(error_)) >> _log;

        end_connecttime_ = gettickcount();
        last_status_ = status_;
        status_ = EEnd;
        return;
    }

    error_ = socket_error(sock_);
    
    if (0 != error_) {
        xwarn2(TSF"close connect error:(%_, %_), ", error_, socket_strerror(error_)) >> _log;
        end_connecttime_ = gettickcount();
        last_status_ = status_;
        status_ = EEnd;
        return;
    }
    
    if (0 == error_ && _sel.Write_FD_ISSET(sock_)){
        end_connecttime_ = gettickcount();
        last_status_ = status_;
        status_ = EReadWrite;
        xinfo2(TSF"connected Rtt:%_, ", Rtt()) >> _log;
        _OnConnected(Rtt());
        return;
    }

    if (0 >= timeout) {
        end_connecttime_ = gettickcount();
        xwarn2(TSF"close connect timeout:(%_, %_), (%_, %_)", ConnectAbsTimeout(), -timeout, SOCKET_ERRNO(ETIMEDOUT), socket_strerror(SOCKET_ERRNO(ETIMEDOUT))) >> _log;

        error_ = SOCKET_ERRNO(ETIMEDOUT);
        last_status_ = status_;
        status_ = EEnd;
        return;
    }
}

void TcpClientFSM::PreReadWriteSelect(SocketSelect& _sel, XLogger& _log) {
    xassert2(EReadWrite == status_, "%d", status_);

    _sel.Read_FD_SET(sock_);
    _sel.Exception_FD_SET(sock_);

    if (0 < send_buf_.Length() || request_send_) _sel.Write_FD_SET(sock_);
}

void TcpClientFSM::AfterReadWriteSelect(const SocketSelect& _sel, XLogger& _log) {
    xassert2(EReadWrite == status_, "%d", status_);

    int timeout = ReadWriteTimeout();

    xinfo2(TSF"sock:%_, (%_:%_), ", sock_, IP(), Port()) >> _log;

    if (_sel.Exception_FD_ISSET(sock_)) {
        socklen_t len = sizeof(error_);

        if (0 != getsockopt(sock_, SOL_SOCKET, SO_ERROR, &error_, &len)) { error_ = socket_errno; }

        xwarn2(TSF"close exception:(%_, %_), ", error_, socket_strerror(error_)) >> _log;
        last_status_ = status_;
        status_ = EEnd;
        return;
    }

    if (_sel.Write_FD_ISSET(sock_)) {
        if (request_send_ && 0 == send_buf_.Length()) {
            request_send_ = false;
            _OnRequestSend(send_buf_);
        }

        ssize_t ret = send(sock_, send_buf_.Ptr(), send_buf_.Length(), 0);

        if (0 < ret) {
            send_buf_.Move(-ret);
            xinfo2_if(0 == send_buf_.Length(), TSF"all buffer send:%_, m_send_buf:%_", ret, send_buf_.Length()) >> _log;
            _OnSend(send_buf_, ret);
        } else if (IS_NOBLOCK_SEND_ERRNO(socket_errno)) {
            xwarn2(TSF"buffer full wait for next select, send err:(%_, %_, %_)", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
        } else {
            error_ = socket_errno;
            last_status_ = status_;
            status_ = EEnd;
            xwarn2(TSF"close send err:(%_, %_, %_), localip:%_", ret, error_, socket_strerror(error_),socket_address::getsockname(sock_).ip()) >> _log;
            return;
        }
    }

    if (_sel.Read_FD_ISSET(sock_)) {
        if (8 * 1024 > recv_buf_.Capacity() - recv_buf_.Length()) {
            recv_buf_.AddCapacity(16 * 1024 - (recv_buf_.Capacity() - recv_buf_.Length()));
        }

        ssize_t ret = recv(sock_, ((char*) recv_buf_.Ptr() + recv_buf_.Length()), recv_buf_.Capacity() - recv_buf_.Length(), 0);

        if (0 < ret) {
            xinfo2_if(0 == recv_buf_.Length(), TSF"first buffer recv:%_, m_recv_buf:%_", ret, recv_buf_.Length()) >> _log;
            recv_buf_.Length(recv_buf_.Pos(), recv_buf_.Length() + ret);
            _OnRecv(recv_buf_, ret);
        } else if (0 == ret) {
            error_ = 0;
            last_status_ = status_;
            status_ = EEnd;
            remote_close_ = true;
            xwarn2(TSF"close recv %_:(%_, %_, %_)", "remote socket close", ret, 0, socket_strerror(0)) >> _log;
            return;
        } else if (IS_NOBLOCK_READ_ERRNO(socket_errno)) {
            xwarn2(TSF"buffer empty wait for next select, recv err:(%_, %_, %_)", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
        } else {
            error_ = socket_errno;
            last_status_ = status_;
            status_ = EEnd;
            xwarn2(TSF"close recv %_:(%_, %_, %_), localip:%_", "err", ret, error_, socket_strerror(error_), socket_address::getsockname(sock_).ip()) >> _log;
            return;
        }
    }

    if (!_sel.Write_FD_ISSET(sock_) && !_sel.Read_FD_ISSET(sock_) && 0 >= timeout) {
        xwarn2(TSF"close readwrite timeout:(%_, %_), (%_, %_)", ReadWriteAbsTimeout(), -timeout, SOCKET_ERRNO(ETIMEDOUT), socket_strerror(SOCKET_ERRNO(ETIMEDOUT))) >> _log;

        error_ =  SOCKET_ERRNO(ETIMEDOUT);
        last_status_ = status_;
        status_ = EEnd;
        return;
    }
}

int TcpClientFSM::ConnectTimeout() const { return INT_MAX; }
int TcpClientFSM::ReadWriteTimeout() const { return INT_MAX; }
int TcpClientFSM::ConnectAbsTimeout() const { return INT_MAX; }
int TcpClientFSM::ReadWriteAbsTimeout() const { return INT_MAX;}
int TcpClientFSM::Rtt() const { return int(end_connecttime_ - start_connecttime_);}
