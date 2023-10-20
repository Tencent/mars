// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _SOCKSTSELECT_
#define _SOCKSTSELECT_

#include <map>
#include <vector>

#include "socket/unix_socket.h"
#include "thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"
#include "socketbreaker.h"

namespace mars {
namespace comm {

class SocketSelect {
 public:
    SocketSelect(SocketBreaker& _breaker, bool _autoclear = false);
    ~SocketSelect();

    void PreSelect();
    void Read_FD_SET(SOCKET _socket);
    void Write_FD_SET(SOCKET _socket);
    void Exception_FD_SET(SOCKET _socket);
    void Event_FD_SET(WSAEVENT event);
    int Select();
    int Select(int _msec);
    int Select(int _sec, int _usec);

    int Errno() const;

    int Read_FD_ISSET(SOCKET _socket) const;
    int Write_FD_ISSET(SOCKET _socket) const;
    int Exception_FD_ISSET(SOCKET _socket) const;
    bool Event_FD_ISSET(WSAEVENT event);

    bool IsBreak() const;
    bool IsException() const;

    SocketBreaker& Breaker();

 private:
    SocketSelect(const SocketSelect&);
    SocketSelect& operator=(const SocketSelect&);

 private:
    const bool autoclear_;
    SocketBreaker& breaker_;
    bool m_broken;

    std::map<SOCKET, int> m_filter_map;
    std::vector<WSAEVENT> vec_events_;
    int errno_;

    fd_set writefd_;
    fd_set readfd_;
    fd_set exceptionfd_;
};
}  // namespace comm
}  // namespace mars
#endif
