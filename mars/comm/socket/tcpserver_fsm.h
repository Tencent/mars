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
 * TcpServerFSM.h
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#ifndef TcpServerFSM_H_
#define TcpServerFSM_H_

#include "comm/autobuffer.h"
#include "comm/socket/unix_socket.h"
#include "comm/xlogger/xlogger.h"
#include "comm/thread/mutex.h"
#include "comm/thread/lock.h"

class XLogger;
class SocketSelect;

class TcpServerFSM {
  public:
    enum TSocketStatus {
        kAccept,
        kReadWrite,
        kEnd,
    };

  public:
    TcpServerFSM(SOCKET _socket);
    TcpServerFSM(SOCKET _socket, const sockaddr_in& _addr);
    virtual ~TcpServerFSM();

    TSocketStatus Status() const;
    void Status(TSocketStatus _status);
    bool IsEndStatus() const;

    SOCKET Socket() const;
    const sockaddr_in& Address() const;
    const char* IP() const;
    uint16_t Port() const;
    size_t SendBufLen() {return send_buf_.Length();}
    void Close(bool _notify = true);

    bool WriteFDSet()
    {
    	ScopedLock lock (write_fd_set_mutex_);
    	return is_write_fd_set_;
    }
    void WriteFDSet(bool _is_set) {
    	xverbose_function(TSF"_is_set:%_, is_write_fd_set_:%_", _is_set, is_write_fd_set_);
    	ScopedLock lock (write_fd_set_mutex_);
    	is_write_fd_set_  = _is_set;
    }

    virtual TSocketStatus PreSelect(SocketSelect& _sel, XLogger& _log);
    virtual TSocketStatus AfterSelect(SocketSelect& _sel, XLogger& _log);
    virtual int Timeout() const;

  private:
    TcpServerFSM(const TcpServerFSM&);
    TcpServerFSM& operator=(const TcpServerFSM&);

  protected:
    virtual TSocketStatus PreReadWriteSelect(SocketSelect& _sel, XLogger& _log);
    virtual TSocketStatus AfterReadWriteSelect(const SocketSelect& _sel, XLogger& _log);

    virtual int ReadWriteTimeout() const;
    virtual int ReadWriteAbsTimeout() const;

    virtual void _OnAccept() = 0;
    virtual void _OnRecv(AutoBuffer& _recv_buff, ssize_t _recv_len) = 0;
    virtual void _OnSend(AutoBuffer& _send_buff, ssize_t _send_len) = 0;
    virtual void _OnClose(TSocketStatus _status, int _error, bool _userclose) = 0;


  protected:
    TSocketStatus status_;
    SOCKET sock_;
    sockaddr_in addr_;
    char ip_[16];

    AutoBuffer send_buf_;
    AutoBuffer recv_buf_;

    bool is_write_fd_set_;
    Mutex write_fd_set_mutex_;
};

#endif /* TcpServerFSM_H_ */
