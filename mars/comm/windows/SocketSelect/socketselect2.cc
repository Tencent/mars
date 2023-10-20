// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#include "socketselect2.h"

#include "xlogger/xlogger.h"

static DWORD __SO_RCVTIMEO(SOCKET _sock) {
    DWORD optval = 0;
    int optlen = sizeof(optval);
    ASSERT2(0 == getsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, &optval, &optlen),
            "%d, %s",
            WSAGetLastError(),
            gai_strerror(socket_errno));
    return optval;
}

static void __SO_RCVTIMEO(SOCKET _sock, DWORD _ms) {
    DWORD optval = _ms;
    int optlen = sizeof(optval);
    ASSERT2(0 == setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, optlen),
            "%d, %s",
            socket_errno,
            gai_strerror(socket_errno));
}

static BOOL __WOULDBLOCK(SOCKET _sock) {
    DWORD ret = __SO_RCVTIMEO(_sock);

    if (0 == ret) {
        return true;
    }

    return ret & 0x1;
}

static void __WOULDBLOCK(SOCKET _sock, bool _block) {
    DWORD ret = __SO_RCVTIMEO(_sock);

    if (0 == ret) {
        ret = 60 * 1000;
    }

    __SO_RCVTIMEO(_sock, (ret & (~0x1)) + _block);
}

namespace mars {
namespace comm {

SocketSelect::SocketSelect(SocketBreaker& _breaker, bool _autoclear)
: autoclear_(_autoclear), breaker_(_breaker), m_broken(false), errno_(0) {
    // inital FD
    FD_ZERO(&writefd_);

    FD_ZERO(&readfd_);
    FD_ZERO(&exceptionfd_);
}

SocketSelect::~SocketSelect() {
}

void SocketSelect::PreSelect() {
    m_broken = false;
    m_filter_map.clear();
    errno_ = 0;

    FD_ZERO(&writefd_);

    FD_ZERO(&readfd_);
    FD_ZERO(&exceptionfd_);
}

int SocketSelect::Select() {
    return Select(WSA_INFINITE);
}

int SocketSelect::Select(int _msec) {
    xverbose_function();
    xgroup2_define(group);
    ASSERT(-1 <= _msec);

    // create eventarray and socketarray
    int eventfd_count = 1 + vec_events_.size();

    WSAEVENT* eventarray = (WSAEVENT*)calloc(m_filter_map.size() + eventfd_count, sizeof(WSAEVENT));
    SOCKET* socketarray = (SOCKET*)calloc(m_filter_map.size() + eventfd_count, sizeof(SOCKET));
    eventarray[0] = Breaker().BreakerFD();
    socketarray[0] = INVALID_SOCKET;

    for (size_t i = 0; i < vec_events_.size(); i++) {
        eventarray[1 + i] = vec_events_[i];
        socketarray[1 + i] = INVALID_SOCKET;
    }

    int index = eventfd_count;

    for (std::map<SOCKET, int>::iterator it = m_filter_map.begin(); it != m_filter_map.end(); ++it) {
        eventarray[index] = WSACreateEvent();
        socketarray[index] = it->first;

        ASSERT2(WSA_INVALID_EVENT != eventarray[index], "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
        ASSERT2(SOCKET_ERROR != WSAEventSelect(it->first, eventarray[index], it->second),
                "%d, %s",
                WSAGetLastError(),
                gai_strerror(WSAGetLastError()));

        ++index;
    }

    // select
    SOCKET maxsocket = INVALID_SOCKET;

    for (std::map<SOCKET, int>::iterator it = m_filter_map.begin(); it != m_filter_map.end(); ++it) {
        maxsocket = maxsocket == INVALID_SOCKET ? it->first : maxsocket;
        maxsocket = it->first > maxsocket ? it->first : maxsocket;

        if (it->second & (FD_READ | FD_ACCEPT | FD_CLOSE)) {
            FD_SET(it->first, &readfd_);
        }

        if (it->second & (FD_WRITE | FD_CONNECT)) {
            FD_SET(it->first, &writefd_);
        }

        if (it->second & (FD_CLOSE)) {
            FD_SET(it->first, &exceptionfd_);
        }
    }

    int ret = 0;

    if (INVALID_SOCKET != maxsocket) {
        timeval timeout = {0, 0};
        ret = select(maxsocket + 1, &readfd_, &writefd_, &exceptionfd_, &timeout);

        if (0 > ret) {
            errno_ = WSAGetLastError();
            m_broken = Breaker().m_broken;

            if (autoclear_)
                Breaker().Clear();

            xdebug2(TSF "return select, ret=%_", ret);
            goto END;  // free eventarray and socketarrary
        }

        if (0 < ret) {
            m_broken = Breaker().m_broken;

            if (autoclear_)
                Breaker().Clear();
            goto END;  // free eventarray and socketarrary
        }
    }

    // check socket first write select
    int new_WOULDBLOCK_count = 0;

    for (std::map<SOCKET, int>::iterator it = m_filter_map.begin(); it != m_filter_map.end(); ++it) {
        if (it->second & (FD_WRITE) && !__WOULDBLOCK(it->first)) {
            FD_SET(it->first, &writefd_);
            __WOULDBLOCK(it->first, true);
            ++new_WOULDBLOCK_count;
            xinfo2(TSF ", %_", it->first) >> group;
        }
    }

    if (0 < new_WOULDBLOCK_count) {
        xinfo2(TSF "WOULDBLOCK FD_WRITE wait count:%_", new_WOULDBLOCK_count) << group;
        m_broken = Breaker().m_broken;

        if (autoclear_)
            Breaker().Clear();

        ret = new_WOULDBLOCK_count;
        goto END;  // free eventarray and socketarrary
    }

    // WSAWaitForMultipleEvents
    ret = WSAWaitForMultipleEvents(m_filter_map.size() + eventfd_count, eventarray, FALSE, _msec, FALSE);
    ASSERT2(WSA_WAIT_FAILED != ret, "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));

    if (WSA_WAIT_FAILED == ret)
        errno_ = WSAGetLastError();

    // different ms, eventarray[0] is fd, so ret > WSA_WAIT_EVENT_0
    if (WSA_WAIT_FAILED != ret && WSA_WAIT_TIMEOUT != ret && ret > WSA_WAIT_EVENT_0
        && ret < WSA_WAIT_EVENT_0 + m_filter_map.size() + eventfd_count) {
        WSANETWORKEVENTS networkevents = {0};
        int event_index = ret;
        ret = WSAEnumNetworkEvents(socketarray[event_index - WSA_WAIT_EVENT_0],
                                   eventarray[event_index - WSA_WAIT_EVENT_0],
                                   &networkevents);

        if (ret == SOCKET_ERROR)
            errno_ = WSAGetLastError();
        else {
            SOCKET sock = socketarray[event_index - WSA_WAIT_EVENT_0];

            if (networkevents.lNetworkEvents & (FD_WRITE | FD_CONNECT) && 0 != __SO_RCVTIMEO(sock)) {
                xinfo2(TSF "WOULDBLOCK FD_WRITE notify sock:%_", sock);
            }

            if (m_filter_map[sock] & (FD_WRITE | FD_CONNECT)
                && networkevents.lNetworkEvents & (FD_WRITE | FD_CONNECT)) {
                FD_SET(sock, &writefd_);
                __WOULDBLOCK(sock, false);
                xverbose2(TSF "FD_WRITE | FD_CONNECT");
            }

            if (m_filter_map[sock] & (FD_READ | FD_ACCEPT) && networkevents.lNetworkEvents & (FD_READ | FD_ACCEPT)) {
                FD_SET(sock, &readfd_);
                xverbose2(TSF "FD_READ | FD_ACCEPT");
            }

            if (m_filter_map[sock] & (FD_READ | FD_ACCEPT) && networkevents.lNetworkEvents & FD_CLOSE
                && networkevents.iErrorCode[FD_CLOSE_BIT] == 0) {
                FD_SET(sock, &readfd_);
                xverbose2(TSF "FD_READ | FD_ACCEPT");
            }

            if (m_filter_map[sock] & (FD_CLOSE)) {
                for (int i = 0; i < FD_MAX_EVENTS; ++i) {
                    if (networkevents.iErrorCode[i] != 0) {
                        xerror2(TSF "selector exception, sock %_ err %_", sock, networkevents.iErrorCode[i]);
                        FD_SET(sock, &exceptionfd_);
                        break;
                    }
                }
            }
        }
    } else {
        xinfo2(TSF "return WSAWaitForMultipleEvents, ret=%_", ret);
    }

    if (ret == WSA_WAIT_FAILED)
        ret = -1;
    else if (ret == SOCKET_ERROR)
        ret = -1;
    else if (ret == WSA_WAIT_TIMEOUT)
        ret = 0;
    else
        ret = 1;

    m_broken = Breaker().m_broken;

    if (autoclear_)
        Breaker().Clear();

END:
    // free eventarray and socketarray
    index = eventfd_count;

    for (std::map<SOCKET, int>::iterator it = m_filter_map.begin(); it != m_filter_map.end(); ++it) {
        ASSERT2(WSACloseEvent(eventarray[index]), "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
        ++index;
    }
    // for (size_t i = 0; i < vec_events_.size(); i++) {
    //     //.event 句柄对象.
    //     ASSERT2(WSACloseEvent(eventarray[i + 1]), "%d, %s", WSAGetLastError(), gai_strerror(WSAGetLastError()));
    // }
    vec_events_.clear();
    free(eventarray);
    free(socketarray);

    return ret;
}

int SocketSelect::Select(int _sec, int _usec) {
    ASSERT(0 <= _sec);
    ASSERT(0 <= _usec);

    return Select(_sec * 1000 + _usec / 1000);
}

void SocketSelect::Read_FD_SET(SOCKET _socket) {
    m_filter_map[_socket] |= (FD_READ | FD_ACCEPT | FD_CLOSE);
}

void SocketSelect::Write_FD_SET(SOCKET _socket) {
    m_filter_map[_socket] |= (FD_WRITE | FD_CONNECT);
}

void SocketSelect::Exception_FD_SET(SOCKET _socket) {
    m_filter_map[_socket] |= (FD_CLOSE);
}

void SocketSelect::Event_FD_SET(WSAEVENT event) {
    vec_events_.push_back(event);
}

int SocketSelect::Read_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &readfd_);
}

int SocketSelect::Write_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &writefd_);
}

int SocketSelect::Exception_FD_ISSET(SOCKET _socket) const {
    return FD_ISSET(_socket, &exceptionfd_);
}

bool SocketSelect::Event_FD_ISSET(WSAEVENT event) {
    return WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
}

bool SocketSelect::IsException() const {
    return 0 != breaker_.m_exception;
}

bool SocketSelect::IsBreak() const {
    return m_broken;
}

SocketBreaker& SocketSelect::Breaker() {
    return breaker_;
}

int SocketSelect::Errno() const {
    return errno_;
}

}  // namespace comm
}  // namespace mars
