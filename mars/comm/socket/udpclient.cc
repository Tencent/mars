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
 * UdpClient.cpp
 *
 *  Created on: 2014-9-5
 *      Author: zhouzhijie
 */

#include "udpclient.h"

#include "comm/xlogger/xlogger.h"
#include "mars/boost/bind.hpp"
#include "comm/socket/socket_address.h"

#define DELETE_AND_NULL(a) {if (a) delete a; a = NULL;}
#define MAX_DATAGRAM 65536

struct UdpSendData
{
    UdpSendData() {}
    UdpSendData(const UdpSendData& rhs)
    {
    }
    AutoBuffer data;
};


UdpClient::UdpClient(const std::string& _ip, int _port)
:fd_socket_(INVALID_SOCKET)
, event_(NULL)
, selector_(breaker_, true)
, thread_(NULL)
{
    __InitSocket(_ip, _port);
}

UdpClient::UdpClient(const std::string& _ip, int _port, IAsyncUdpClientEvent* _event)
:fd_socket_(INVALID_SOCKET)
, event_(_event)
, selector_(breaker_, true)
{
    thread_ = new Thread(boost::bind(&UdpClient::__RunLoop, this));
    
    __InitSocket(_ip, _port);
}

UdpClient::~UdpClient()
{
    if (thread_ && thread_->isruning())
    {
        event_ = NULL;
        breaker_.Break();
        thread_->join();
    }
    breaker_.Break();
    DELETE_AND_NULL(thread_);
    
    list_buffer_.clear();
    
    if (fd_socket_ != INVALID_SOCKET)
        socket_close(fd_socket_);
}

int UdpClient::SendBlock(void* _buf, size_t _len)
{
    xassert2((fd_socket_ != INVALID_SOCKET && event_ == NULL), "socket invalid");
    if (fd_socket_ == INVALID_SOCKET || event_ != NULL)
        return -1;
    
    int err = 0;
    return __DoSelect(false, true, _buf, _len, err, -1);
}

int UdpClient::ReadBlock(void* _buf, size_t _len, int _timeOutMs)
{
    xassert2((fd_socket_ != INVALID_SOCKET && event_ == NULL), "socket invalid");
    if (fd_socket_ == INVALID_SOCKET || event_ != NULL)
        return -1;
    
    int err = 0;
    return __DoSelect(true, false, _buf, _len, err, -1);
}


bool UdpClient::HasBuuferToSend()
{
    ScopedLock lock(mutex_);
    return !list_buffer_.empty();
}

void UdpClient::SendAsync(void* _buf, size_t _len)
{
    xassert2((fd_socket_ != INVALID_SOCKET && event_ != NULL), "socket invalid");
    if (fd_socket_ == INVALID_SOCKET || event_ == NULL)
        return;
    
    ScopedLock lock(mutex_);
    list_buffer_.push_back(UdpSendData());
    list_buffer_.back().data.Write(_buf, _len);
    
    if (!thread_->isruning())
        thread_->start();
    breaker_.Break();
}

void UdpClient::SetIpPort(const std::string& _ip, int _port)
{
    bzero(&addr_, sizeof(addr_));
    addr_ = *(struct sockaddr_in*)(&socket_address(_ip.c_str(), _port).address());
}

void UdpClient::__InitSocket(const std::string& _ip, int _port)
{
    int errCode = 0;
    
    bzero(&addr_, sizeof(addr_));
    addr_ = *(struct sockaddr_in*)(&socket_address(_ip.c_str(), _port).address());
    
    fd_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_socket_ == INVALID_SOCKET)
    {
        errCode = socket_errno;
        xerror2(TSF"udp socket create error, error: %0", socket_strerror(errCode));
        return;
    }
    
    if (IPV4_BROADCAST_IP == _ip)
    {
        int on = 1;
        if (setsockopt(fd_socket_, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(on)) != 0)
        {
            errCode = socket_errno;
            xerror2(TSF"udp set broadcast error: %0", socket_strerror(errCode));
            return;
        }
    }
}

void UdpClient::__RunLoop()
{
    xassert2(fd_socket_ != INVALID_SOCKET, "socket invalid");
    if (fd_socket_ == INVALID_SOCKET)
        return;
    
    char* readBuffer = new char[MAX_DATAGRAM];
    void* buf = NULL;
    size_t len = 0;
    int ret = 0;
    while (true)
    {
        mutex_.lock();
        bool bWriteSet = list_buffer_.size() > 0;
        if (bWriteSet)
        {
            buf = list_buffer_.front().data.Ptr();
            len = list_buffer_.front().data.Length();
        } else {
            bzero(readBuffer, MAX_DATAGRAM);
            buf = readBuffer;
            len = MAX_DATAGRAM - 1;
        }
        mutex_.unlock();
        
        int err = 0;
        ret = __DoSelect(bWriteSet ? false : true, bWriteSet, buf, len, err, -1);    // only read or write can be true
        if (ret == -1)
        {
            xerror2(TSF"select error");
            if (event_)
                event_->OnError(this, err);
            break;
        }
        
        if (ret == -2 && event_ == NULL)
        {
            xinfo2(TSF"normal break");
            break;
        }
        if (ret == -2)
            continue;
        
        if (bWriteSet)
        {
            ScopedLock lock(mutex_);
            list_buffer_.pop_front();
            continue;
        }
    }
    
    delete[] readBuffer;
}

/*
 * return -2 break, -1 error, 0 timeout, else handle size
 */
int UdpClient::__DoSelect(bool _bReadSet, bool _bWriteSet, void* _buf, size_t _len, int& _errno, int _timeoutMs)
{
    xassert2((!(_bReadSet && _bWriteSet) && (_bReadSet || _bWriteSet)), "only read or write can be true, not both");
    
    selector_.PreSelect();
    if (_bWriteSet)
        selector_.Write_FD_SET(fd_socket_);
    else if (_bReadSet)
        selector_.Read_FD_SET(fd_socket_);
    selector_.Exception_FD_SET(fd_socket_);
    
    int ret = ((_timeoutMs == -1) ? selector_.Select() : selector_.Select(_timeoutMs));
    if (ret < 0)
    {
        xerror2(TSF"udp select error: %0", socket_strerror(selector_.Errno()));
        _errno = selector_.Errno();
        return -1;
    }
    if (ret == 0)
    {
        xinfo2(TSF"udp select timeout:%0 ms", _timeoutMs);
        return 0;
    }
    
    // user break
    if (selector_.IsException())
    {
        _errno = selector_.Errno();
        xerror2(TSF"sel exception");
        return -1;
    }
    if (selector_.IsBreak())
    {
        xinfo2(TSF"sel breaker");
        return -2;
    }
    if (selector_.Exception_FD_ISSET(fd_socket_))
    {
        _errno = socket_errno;
        xerror2(TSF"socket exception error");
        return -1;
    }
    
    if (selector_.Write_FD_ISSET(fd_socket_))
    {
        int ret = (int)sendto(fd_socket_, (const char *)_buf, _len, 0, (sockaddr*)&addr_, sizeof(sockaddr_in));
        if (ret == -1)
        {
            _errno = socket_errno;
            xerror2(TSF"sendto error: %0", socket_strerror(_errno));
            return -1;
        }
        if (event_)
            event_->OnDataSent(this);
        return ret;
    }
    
    if (selector_.Read_FD_ISSET(fd_socket_))
    {
        int ret = (int)recvfrom(fd_socket_, (char *)_buf, _len, 0, NULL, NULL);
        if (ret == -1)
        {
            _errno = socket_errno;
            xerror2(TSF"recvfrom error: %0", socket_strerror(_errno));
            return -1;
        }
        
        if (event_)
            event_->OnDataGramRead(this, _buf, ret);
        return ret;
    }
    
    return -1;
}

