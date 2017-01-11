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
 * TcpClient.h
 *
 *  Created on: 2013-5-7
 *      Author: yerungui && jiahong
 */

#include "tcpclient.h"

#include "boost/bind.hpp"

#include "autobuffer.h"
#include "xlogger/xlogger.h"
#include "thread/thread.h"
#include "platform_comm.h"
#include "socket_address.h"


#ifdef _WIN32
#define strdup _strdup
#endif

TcpClient::TcpClient(const char* _ip, uint16_t _port, MTcpEvent& _event, int _timeout)
    : ip_(strdup(_ip)) , port_(_port) , event_(_event)
    , socket_(INVALID_SOCKET) , have_read_date_(false) , will_disconnect_(false) , writedbufid_(0)
    , thread_(boost::bind(&TcpClient::__RunThread, this))
    , timeout_(_timeout), status_(kTcpInit) {
    if (!pipe_.IsCreateSuc()) status_ = kTcpInitErr;
}

TcpClient::~TcpClient() {
    DisconnectAndWait();

    for (std::list<AutoBuffer* >::iterator it = lst_buffer_.begin();
            it != lst_buffer_.end(); ++it) {
        delete(*it);
    }

    lst_buffer_.clear();
    free(ip_);
}

bool TcpClient::Connect() {
    ScopedLock lock(connect_mutex_);

    if (kTcpInit != status_) {
        xassert2(kTcpInitErr == status_, "%d", status_);
        return false;
    }

    status_ = kSocketThreadStart;
    int ret = thread_.start();

    if (0 != ret) {
        status_ = kSocketThreadStartErr;
        xassert2(0 != ret, "%d", ret);
        return false;
    }

    return true;
}

void TcpClient::Disconnect() {
    if (will_disconnect_) return;

    ScopedLock lock(read_disconnect_mutex_);
    will_disconnect_ = true;
    __SendBreak();
}


void TcpClient::DisconnectAndWait() {
    Disconnect();

    if (thread_.isruning())
        thread_.join();
}

bool TcpClient::HaveDataRead() const {
    if (kTcpConnected != status_) return false;

    return have_read_date_;
}

ssize_t TcpClient::Read(void* _buf, unsigned int _len) {
    if (kTcpConnected != status_) return -1;

    xassert2(INVALID_SOCKET != socket_);
    ScopedLock lock(read_disconnect_mutex_);
    ssize_t ret = recv(socket_, (char*)_buf, _len, 0);

    have_read_date_ = false;
    __SendBreak();

    return ret;
}

bool TcpClient::HaveDataWrite() const {
    if (kTcpConnected != status_) return false;

    ScopedLock lock(write_mutex_);
    return !lst_buffer_.empty();
}

int TcpClient::Write(const void* _buf, unsigned int _len) {
    if (kTcpConnected != status_) return -1;

    AutoBuffer* tmpbuff = new AutoBuffer;
    tmpbuff->Write(0, _buf, _len);

    ScopedLock lock(write_mutex_);
    lst_buffer_.push_back(tmpbuff);
    writedbufid_++;
    __SendBreak();

    return writedbufid_;
}

int TcpClient::WritePostData(void* _buf, unsigned int _len) {
    if (kTcpConnected != status_) return -1;

    AutoBuffer* tmpbuff = new AutoBuffer;
    tmpbuff->Attach((void*)_buf, _len);

    ScopedLock lock(write_mutex_);
    lst_buffer_.push_back(tmpbuff);
    writedbufid_++;
    __SendBreak();
    return writedbufid_;
}

void TcpClient::__Run() {
    status_ = kTcpConnecting;

    struct sockaddr_in _addr;
    in_addr_t ip  = ((sockaddr_in*)&socket_address(ip_, 0).address())->sin_addr.s_addr;

    if ((in_addr_t)(-1) == ip) {
        status_ = kTcpConnectIpErr;
        event_.OnError(status_, SOCKET_ERRNO(EADDRNOTAVAIL));
        return;
    }

    memset(&_addr, 0, sizeof(_addr));
    _addr = *(struct sockaddr_in*)(&socket_address(ip_, port_).address());

    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_ == INVALID_SOCKET) {
        status_ = kTcpConnectingErr;
        xerror2("m_socket errno=%d",  socket_errno);
        event_.OnError(status_,  socket_errno);
        return;
    }

    if (::getNetInfo() == kWifi && socket_fix_tcp_mss(socket_) < 0) {
#ifdef ANDROID
        xinfo2(TSF"wifi set tcp mss error:%0", strerror(socket_errno));
#endif
    }

    if (0 != socket_ipv6only(socket_, 0)){
        xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno));
    }
    
    xerror2_if(0 != socket_set_nobio(socket_), TSF"socket_set_nobio:%_, %_", socket_errno, socket_strerror(socket_errno));

    int ret = ::connect(socket_, (sockaddr*)&_addr, sizeof(_addr));

    std::string local_ip = socket_address::getsockname(socket_).ip();
    unsigned int local_port = socket_address::getsockname(socket_).port();

    xinfo2(TSF"sock:%_, local_ip:%_, local_port:%_, svr_ip:%_, svr_port:%_", socket_, local_ip, local_port, ip_, port_);

    if (0 > ret && !IS_NOBLOCK_CONNECT_ERRNO(socket_errno)) {
        xerror2("connect errno=%d", socket_errno);
        status_ = kTcpConnectingErr;
        event_.OnError(status_, socket_errno);
        return;
    }

    SocketSelect select_connect(pipe_, true);
    select_connect.PreSelect();
    select_connect.Exception_FD_SET(socket_);
    select_connect.Write_FD_SET(socket_);

    int selectRet = 0 < timeout_ ? select_connect.Select(timeout_) : select_connect.Select();

    if (0 == selectRet) {
        status_ = kTcpConnectTimeoutErr;
        event_.OnError(status_, SOCKET_ERRNO(ETIMEDOUT));
        return;
    }

    if (selectRet < 0) {
        xerror2("select errno=%d", socket_errno);
        status_ = kTcpConnectingErr;
        event_.OnError(status_, socket_errno);
        return;
    }

    if (select_connect.Exception_FD_ISSET(socket_)) {
        status_ = kTcpConnectingErr;
        event_.OnError(status_, socket_error(socket_));
        return;
    }

    if (select_connect.Write_FD_ISSET(socket_) && 0 != socket_error(socket_)) {
        status_ = kTcpConnectingErr;
        event_.OnError(status_, socket_error(socket_));
        return;
    }

    if (will_disconnect_) {
        status_ = kTcpDisConnected;
        event_.OnDisConnect(false);
        return;
    }

    status_ = kTcpConnected;
    event_.OnConnect();

    int write_id = 0;

    while (true) {
        SocketSelect select_readwrite(pipe_, true);
        select_readwrite.PreSelect();
        select_readwrite.Exception_FD_SET(socket_);

        if (!have_read_date_) select_readwrite.Read_FD_SET(socket_);

        {
            ScopedLock lock(write_mutex_);

            if (!lst_buffer_.empty())select_readwrite.Write_FD_SET(socket_);
        }
        selectRet = select_readwrite.Select();

        if (0 == selectRet) {
            xassert2(false);
            continue;
        }

        if (0 > selectRet) {
            xerror2("select errno=%d", socket_errno);
            status_ = kTcpIOErr;
            event_.OnError(status_, socket_errno);
            return;
        }

        if (will_disconnect_) {
            status_ = kTcpDisConnected;
            event_.OnDisConnect(false);
            return;
        }

        if (select_readwrite.Exception_FD_ISSET(socket_)) {
            int error_opt = 0;
            socklen_t error_len = sizeof(error_opt);
            if (0 == getsockopt(socket_, SOL_SOCKET, SO_ERROR, (char*)&error_opt, &error_len)){
                xerror2("error_opt=%d", error_opt);
            }else{
                xerror2("getsockopt error=%d", socket_errno);
            }
            status_ = kTcpIOErr;
            event_.OnError(status_, error_opt);
            return;
        }

        if (select_readwrite.Read_FD_ISSET(socket_)) {
            ScopedLock lock(read_disconnect_mutex_);
            char buf_test;
            ret = (int)recv(socket_, &buf_test, 1, MSG_PEEK);

            if (0 < ret) {
                have_read_date_ = true;

                lock.unlock();
                event_.OnRead();
            } else if (0 == ret) {
                have_read_date_ = false;
                status_ = kTcpDisConnected;
                lock.unlock();
                event_.OnDisConnect(true);
                return;
            } else if (IS_NOBLOCK_RECV_ERRNO(socket_errno)) {
                xwarn2(TSF"IS_NOBLOCK_RECV_ERRNO err:%_, %_", socket_errno, socket_strerror(socket_errno));
            } else {
                status_ = kTcpIOErr;
                lock.unlock();
                event_.OnError(status_, socket_errno);
                return;
            }
        }

        if (select_readwrite.Write_FD_ISSET(socket_)) {
            ScopedLock lock(write_mutex_);
            AutoBuffer& buf = *lst_buffer_.front();
            size_t len = buf.Length();

            if (buf.Pos() < (off_t)len) {
                int send_len = (int)send(socket_, (char*)buf.PosPtr(), (size_t)(len - buf.Pos()), 0);

                if ((0 == send_len) || (0 > send_len && !IS_NOBLOCK_SEND_ERRNO(socket_errno))) {
                    status_ = kTcpIOErr;
                    lock.unlock();
                    event_.OnError(status_, errno);
                    return;
                }

                if (0 < send_len) buf.Seek(send_len, AutoBuffer::ESeekCur);
            } else {
                delete lst_buffer_.front();
                lst_buffer_.pop_front();

                lock.unlock();
                event_.OnWrote(++write_id, (unsigned int)len);

                lock.lock();

                if (lst_buffer_.empty()) {
                    lock.unlock();
                    event_.OnAllWrote();
                }
            }
        }
    }
}

void TcpClient::__RunThread() {
    __Run();

    if (INVALID_SOCKET != socket_) {
        socket_close(socket_);
        socket_ = INVALID_SOCKET;
    }
}

void TcpClient::__SendBreak() {
    pipe_.Break();
}
