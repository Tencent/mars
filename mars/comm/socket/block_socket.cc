// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  block_socket.cpp
//  comm
//
//  Created by yerungui on 16/3/30.
//

#include "comm/socket/socketselect.h"
#include "comm/socket/socket_address.h"
#include "comm/autobuffer.h"
#include "comm/time_utils.h"
#include "comm/xlogger/xlogger.h"
#include "comm/platform_comm.h"

/*
 * param: timeoutInMs if set 0, then select timeout param is NULL, not timeval(0)
 * return value:
 */

SOCKET  block_socket_connect(const socket_address& _address, SocketSelectBreaker& _breaker, int& _errcode, int32_t _timeout/*ms*/) {
    //socket
    SOCKET sock = socket(_address.address().sa_family, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock == INVALID_SOCKET) {
        _errcode = socket_errno;
        return INVALID_SOCKET;
    }
    
    if (0 != socket_ipv6only(sock, 0)){
        xwarn2(TSF"set ipv6only failed. error %_",strerror(socket_errno));
    }
    
    int ret = socket_set_nobio(sock);
    if (ret != 0) {
        _errcode = socket_errno;
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    if (::getNetInfo() == kWifi && socket_fix_tcp_mss(sock) < 0) {
#ifdef ANDROID
        xinfo2(TSF"wifi set tcp mss error:%0", strerror(socket_errno));
#endif
    }
    
    //connect
    ret = connect(sock, &_address.address(), _address.address_length());
    if (ret != 0 && !IS_NOBLOCK_CONNECT_ERRNO(socket_errno)) {
        _errcode = socket_errno;
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    SocketSelect sel(_breaker);
    sel.PreSelect();
    sel.Write_FD_SET(sock);
    sel.Exception_FD_SET(sock);
    
    ret = (_timeout >= 0) ? (sel.Select(_timeout)) : (sel.Select());
    if (ret == 0) {
        _errcode = SOCKET_ERRNO(ETIMEDOUT);
        ::socket_close(sock);
        return INVALID_SOCKET;
    } else if (ret < 0) {
        _errcode = sel.Errno();
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    if (sel.IsException()) {
        _errcode = 0;
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    if (sel.IsBreak()) {
        _errcode = 0;
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    if (sel.Exception_FD_ISSET(sock)) {
        _errcode = socket_error(sock);
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    if (!sel.Write_FD_ISSET(sock)) {
        _errcode = socket_error(sock);
        ::socket_close(sock);
        //        xassert2(false);
        return INVALID_SOCKET;
    }
    _errcode = socket_error(sock);
    if (0 != _errcode) {
        ::socket_close(sock);
        return INVALID_SOCKET;
    }
    
    return sock;
}


/*
 * return value:
 */
int block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketSelectBreaker& _breaker, int &_errcode, int _timeout) {
    uint64_t start = gettickcount();
    int32_t cost_time = 0;
    size_t sent_len = 0;
    
    SocketSelect sel(_breaker);
    
    while (true) {
        if (sent_len >= _len) {
            _errcode = 0;
            return (int)sent_len;
        }
        sel.PreSelect();
        sel.Write_FD_SET(_sock);
        sel.Exception_FD_SET(_sock);
        int ret = (0 <= _timeout)
                ? (sel.Select((_timeout > cost_time) ? (_timeout-cost_time) : 0))
                : (sel.Select());
        cost_time = (int32_t)(gettickcount() - start);
        
        if (ret < 0) {
            _errcode = sel.Errno();
            return -1;
        }
        
        if (ret == 0) {
            _errcode = SOCKET_ERRNO(ETIMEDOUT);
            return (int)sent_len;
        }
        
        if (sel.IsException() || sel.IsBreak()) {
            _errcode = 0;
            return (int)sent_len;
        }
        
        if (sel.Exception_FD_ISSET(_sock)) {
            _errcode = socket_error(_sock);
            return -1;
        }
        
        if (sel.Write_FD_ISSET(_sock)) {
            ssize_t nwrite =::send(_sock, (const char*)_buffer+sent_len, _len-sent_len, 0);
            if (nwrite == 0 || (0 > nwrite && !IS_NOBLOCK_SEND_ERRNO(socket_errno))) {
                _errcode = socket_errno;
                return -1;
            }
            
            if (0 < nwrite) sent_len += nwrite;
        }
    }
}

int block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketSelectBreaker& _breaker, int &_errcode, int _timeout, bool _wait_full_size) {
    
    uint64_t start = gettickcount();
    int32_t cost_time = 0;
    size_t recv_len = 0;
    
    if (_buffer.Capacity() - _buffer.Length() < _max_size) {
        _buffer.AddCapacity(_max_size - (_buffer.Capacity() - _buffer.Length()));
    }
    
    SocketSelect sel(_breaker);
    while (true) {
        if (recv_len >= _max_size){
            _buffer.Length(_buffer.Pos(), _buffer.Length()+recv_len);
            _errcode = 0;
            return (int)recv_len;
        }
        
        if (recv_len > 0 && !_wait_full_size){
            _buffer.Length(_buffer.Pos(), _buffer.Length()+recv_len);
            _errcode = 0;
            return (int)recv_len;
        }
        
        sel.PreSelect();
        sel.Read_FD_SET(_sock);
        sel.Exception_FD_SET(_sock);
        int ret = (0 <= _timeout)
                ? (sel.Select((_timeout > cost_time) ? (_timeout-cost_time) : 0))
                : (sel.Select());
        cost_time = (int32_t)(gettickcount() - start);
        
        if (ret < 0) {
            _errcode = sel.Errno();
            return -1;
        }
        
        if (ret == 0) {
            _errcode = SOCKET_ERRNO(ETIMEDOUT);
            _buffer.Length(_buffer.Pos(), _buffer.Length()+recv_len);
            return (int)recv_len;
        }
        
        if (sel.IsException() || sel.IsBreak()) {
            _errcode = sel.Errno();
            _buffer.Length(_buffer.Pos(), _buffer.Length()+recv_len);
            return (int)recv_len;
        }
        
        if (sel.Exception_FD_ISSET(_sock)) {
            _errcode = socket_error(_sock);
            return -1;
        }
        
        if (sel.Read_FD_ISSET(_sock)) {
            ssize_t nrecv = ::recv(_sock, _buffer.Ptr(_buffer.Length()+recv_len), _max_size-recv_len, 0);
            
            if (0 == nrecv) {
                _errcode = 0;
                _buffer.Length(_buffer.Pos(), _buffer.Length()+recv_len);
                return (int)recv_len;
            }
            
            if (0 > nrecv && !IS_NOBLOCK_READ_ERRNO(socket_errno)) {
                _errcode = socket_errno;
                return -1;
            }
            
            if (0 < nrecv) recv_len += nrecv;
        }
    }
}
