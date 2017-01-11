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
 * TcpServerFSM.cpp
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#include <stdlib.h>
#include <limits.h>

#include "comm/xlogger/xlogger.h"
#include "comm/socket/socketselect.h"
#include "comm/socket/tcpserver_fsm.h"

TcpServerFSM::TcpServerFSM(SOCKET _socket)
    : status_(kAccept), sock_(_socket), is_write_fd_set_(false) {
    xassert2(INVALID_SOCKET != sock_);
    socklen_t addr_len = sizeof(addr_);
    xerror2_if(0 > getpeername(sock_, (sockaddr*)&addr_, &addr_len), TSF"getpeername:%_, %_", socket_errno, socket_strerror(socket_errno));

    memset(ip_, 0, sizeof(ip_));
    inet_ntop(addr_.sin_family, &(addr_.sin_addr), ip_, sizeof(ip_));
}

TcpServerFSM::TcpServerFSM(SOCKET _socket, const sockaddr_in& _addr)
    : status_(kAccept), sock_(_socket), addr_(_addr) , is_write_fd_set_(false){
    memset(ip_, 0, sizeof(ip_));
    inet_ntop(addr_.sin_family, &(addr_.sin_addr), ip_, sizeof(ip_));
}

TcpServerFSM::~TcpServerFSM() {
    Close(false);
    xassert2(INVALID_SOCKET == sock_, "%d", sock_);
}

TcpServerFSM::TSocketStatus TcpServerFSM::Status() const {
    return status_;
}

void TcpServerFSM::Status(TSocketStatus _status) {
    status_ = _status;
}

bool TcpServerFSM::IsEndStatus() const {
    return kEnd == status_;
}

SOCKET TcpServerFSM::Socket() const {
    return sock_;
}

const sockaddr_in& TcpServerFSM::Address() const {
    return addr_;
}

const char* TcpServerFSM::IP() const {
    return ip_;
}

uint16_t TcpServerFSM::Port() const {
    return ntohs(addr_.sin_port);
}

void TcpServerFSM::Close(bool _notify) {
    char ip[16];
    xinfo2(TSF"sock:%_, (%_:%_), onclose local socket close, notify:%_", sock_, socket_inet_ntop(AF_INET, &(addr_.sin_addr), ip, sizeof(ip)), ntohs(addr_.sin_port), _notify);

    if (INVALID_SOCKET == sock_) return;

    socket_close(sock_);
    sock_ = INVALID_SOCKET;

    TSocketStatus status = status_;
    status_ = kEnd;

    if (_notify) _OnClose(status, 0, true);
}

TcpServerFSM::TSocketStatus TcpServerFSM::PreSelect(SocketSelect& _sel, XLogger& _log) {
    if (kAccept == status_) {
        _OnAccept();
        status_ = kReadWrite;
    }

    if (kReadWrite == status_) return PreReadWriteSelect(_sel, _log);

    return status_;
}

TcpServerFSM::TSocketStatus TcpServerFSM::AfterSelect(SocketSelect& _sel, XLogger& _log) {
    if (kReadWrite == status_) return AfterReadWriteSelect(_sel, _log);

    return status_;
}

int TcpServerFSM::Timeout() const {
    if (kReadWrite == status_) return ReadWriteTimeout();

    if (kEnd == status_) return 0;

    return INT_MAX;
}

TcpServerFSM::TSocketStatus TcpServerFSM::PreReadWriteSelect(SocketSelect& _sel, XLogger& _log) {
    xassert2(kReadWrite == status_, "%d", status_);

    _sel.Read_FD_SET(sock_);
    _sel.Exception_FD_SET(sock_);

    if (0 < send_buf_.Length()) {
    	WriteFDSet(true);
    	_sel.Write_FD_SET(sock_);
    } else {
    	WriteFDSet(false);
    }

    return status_;
}

TcpServerFSM::TSocketStatus TcpServerFSM::AfterReadWriteSelect(const SocketSelect& _sel, XLogger& _log) {
    xassert2(kReadWrite == status_, "%d", status_);

    char ip[16] = { 0 };
    int timeout = ReadWriteTimeout();

    xinfo2(TSF"sock:%_, (%_:%_), ", sock_, socket_inet_ntop(AF_INET, &(addr_.sin_addr), ip, sizeof(ip)), ntohs(addr_.sin_port)) >> _log;

    if (_sel.Exception_FD_ISSET(sock_)) {
        int error = 0;
        socklen_t len = sizeof(error);

        if (0 != getsockopt(sock_, SOL_SOCKET, SO_ERROR, &error, &len)) { error = socket_errno; }

        xwarn2(TSF"onclose exception:(%_, %_), ", error, socket_strerror(error)) >> _log;

        socket_close(sock_);
        sock_ = INVALID_SOCKET;

        status_ = kEnd;
        _OnClose(kReadWrite, error, false);
        return kEnd;
    }

    if (_sel.Write_FD_ISSET(sock_)) {
        ssize_t ret = send(sock_, send_buf_.Ptr(), send_buf_.Length(), 0);

        if (0 < ret) {
            send_buf_.Move(-ret);
            xinfo2_if(0 == send_buf_.Length(), TSF"all buffer send:%_, m_send_buf:%_", ret, send_buf_.Length()) >> _log;
            _OnSend(send_buf_, ret);
        } else if (IS_NOBLOCK_SEND_ERRNO(socket_errno)) {
            xwarn2(TSF"buffer full wait for next select, send err:(%_, %_, %_)", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
        } else {
            xwarn2(TSF"onclose send err:(%_, %_, %_)", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
            socket_close(sock_);
            sock_ = INVALID_SOCKET;
            status_ = kEnd;
            _OnClose(kReadWrite, socket_errno, false);
            return kEnd;
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
            xwarn2(TSF"onclose recv %_:(%_, %_, %_)", "remote socket close", ret, 0, socket_strerror(0)) >> _log;
            socket_close(sock_);
            sock_ = INVALID_SOCKET;
            status_ = kEnd;
            _OnClose(kReadWrite, 0, false);
            return kEnd;
        } else if (IS_NOBLOCK_READ_ERRNO(socket_errno)) {
            xwarn2(TSF"buffer empty wait for next select, recv err:(%_, %_, %_)", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
        } else {
            xwarn2(TSF"onclose recv %_:(%_, %_, %_)", "err", ret, socket_errno, socket_strerror(socket_errno)) >> _log;
            socket_close(sock_);
            sock_ = INVALID_SOCKET;
            status_ = kEnd;
            _OnClose(kReadWrite, socket_errno, false);
            return kEnd;
        }
    }

    if (!_sel.Write_FD_ISSET(sock_) && !_sel.Read_FD_ISSET(sock_) && 0 >= timeout) {
        xwarn2(TSF"onclose readwrite timeout:(%_, %_), (%_, %_)", ReadWriteAbsTimeout(), -timeout, SOCKET_ERRNO(ETIMEDOUT), socket_strerror(SOCKET_ERRNO(ETIMEDOUT))) >> _log;

        socket_close(sock_);
        sock_ = INVALID_SOCKET;
        status_ = kEnd;
        TSocketStatus status = status_;
        _OnClose(kReadWrite, SOCKET_ERRNO(ETIMEDOUT), false);
        return status;
    }

    return status_;
}

int TcpServerFSM::ReadWriteTimeout() const { return INT_MAX; }
int TcpServerFSM::ReadWriteAbsTimeout() const { return INT_MAX;}
