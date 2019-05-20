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
 * TcpServer.cpp
 *
 *  Created on: 2014-10-30
 *      Author: yerungui
 */

#include <stdlib.h>

#include "tcpserver.h"

#include "boost/bind.hpp"

#include "comm/thread/lock.h"
#include "comm/xlogger/xlogger.h"
#include "comm/socket/socket_address.h"

TcpServer::TcpServer(const char* _ip, uint16_t _port, MTcpServer& _observer, int _backlog)
    : observer_(_observer)
    , thread_(boost::bind(&TcpServer::__ListenThread, this))
    , listen_sock_(INVALID_SOCKET), backlog_(_backlog) {
    memset(&bind_addr_, 0, sizeof(bind_addr_));
    bind_addr_ = *(struct sockaddr_in*)(&socket_address(_ip, _port).address());
}

TcpServer::TcpServer(uint16_t _port, MTcpServer& _observer, int _backlog)
    : observer_(_observer)
    , thread_(boost::bind(&TcpServer::__ListenThread, this))
    , listen_sock_(INVALID_SOCKET), backlog_(_backlog) {
    memset(&bind_addr_, 0, sizeof(bind_addr_));
    bind_addr_.sin_family = AF_INET;
    bind_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr_.sin_port = htons(_port);
}

TcpServer::TcpServer(const sockaddr_in& _bindaddr, MTcpServer& _observer, int _backlog)
    : observer_(_observer)
    , thread_(boost::bind(&TcpServer::__ListenThread, this))
    , listen_sock_(INVALID_SOCKET), bind_addr_(_bindaddr), backlog_(_backlog)
{}

TcpServer::~TcpServer() {
    StopAndWait();
}

SOCKET TcpServer::Socket() const {
    return listen_sock_;
}

const sockaddr_in& TcpServer::Address() const {
    return bind_addr_;
}

bool TcpServer::StartAndWait(bool* _newone) {
    ScopedLock lock(mutex_);
    bool newone = false;
    thread_.start(&newone);

    if (_newone) *_newone = newone;

    if (newone) {
        breaker_.Clear();
        cond_.wait(lock);
    }

    return INVALID_SOCKET != Socket();
}

void TcpServer::StopAndWait() {
    ScopedLock lock(mutex_);

    if (!breaker_.Break()) {
        xassert2(false);
        breaker_.Close();
        breaker_.ReCreate();
    }

    if (INVALID_SOCKET != listen_sock_)
    {
#ifdef WIN32
		shutdown(listen_sock_, SD_BOTH);
#endif
        socket_close(listen_sock_);
		listen_sock_ = INVALID_SOCKET;
	}
    lock.unlock();

    if (thread_.isruning())
        thread_.join();
}

void TcpServer::__ListenThread() {
    char ip[16] = {0};
	socket_inet_ntop(AF_INET, &(bind_addr_.sin_addr), ip, sizeof(ip));

    xgroup2_define(break_group);

    do {
        ScopedLock lock(mutex_);

        xassert2(INVALID_SOCKET == listen_sock_, TSF"m_listen_sock:%_", listen_sock_);

        SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

        if (INVALID_SOCKET == listen_sock) {
            xerror2(TSF"socket create err:(%_, %_)", socket_errno, socket_strerror(socket_errno)) >> break_group;
            cond_.notifyAll(lock);
            break;
        }

        if (0 > socket_reuseaddr(listen_sock, 1)) { // make sure before than bind
        
            xerror2(TSF"socket reuseaddr err:(%_, %_)", socket_errno, socket_strerror(socket_errno)) >> break_group;
            socket_close(listen_sock);
            cond_.notifyAll(lock);
            break;
        }

        if (0 > bind(listen_sock, (struct sockaddr*) &bind_addr_, sizeof(bind_addr_))) {
            xerror2(TSF"socket bind err:(%_, %_)", socket_errno, socket_strerror(socket_errno)) >> break_group;
            socket_close(listen_sock);
            cond_.notifyAll(lock);
            break;
        }

        if (0 > listen(listen_sock, backlog_)) {
            xerror2(TSF"socket listen err:(%_, %_)", socket_errno, socket_strerror(socket_errno)) >> break_group;
            socket_close(listen_sock);
            cond_.notifyAll(lock);
            break;
        }

        listen_sock_ = listen_sock;
        cond_.notifyAll(lock);
        lock.unlock();

        xinfo2(TSF"listen start sock:(%_, %_:%_)", listen_sock_, ip, ntohs(bind_addr_.sin_port));
        observer_.OnCreate(this);

        while (true) {
#ifndef WIN32
            SocketSelect sel(breaker_);
            sel.PreSelect();
            sel.Exception_FD_SET(listen_sock_);
            sel.Read_FD_SET(listen_sock_);

            int selret = sel.Select();

            if (0 > selret) {
                xerror2(TSF"select ret:%_, err:(%_, %_)", selret, sel.Errno(), socket_strerror(sel.Errno())) >> break_group;
                break;
            }

            if (sel.IsException()) {
                xerror2(TSF"breaker exception") >> break_group;
                break;
            }

            if (sel.IsBreak()) {
                xinfo2(TSF"breaker by user") >> break_group;
                break;
            }

            if (sel.Exception_FD_ISSET(listen_sock_)) {
                xerror2(TSF"socket exception err:(%_, %_)", socket_error(listen_sock_), socket_strerror(socket_error(listen_sock_))) >> break_group;
                break;
            }

            if (!sel.Read_FD_ISSET(listen_sock_)) {
                xerror2(TSF"socket unreadable but break by unknown") >> break_group;
                break;
            }
#endif

            struct sockaddr_in client_addr = {0};

            socklen_t client_addr_len = sizeof(client_addr);

            SOCKET client = accept(listen_sock_, (struct sockaddr*) &client_addr, &client_addr_len);

            if (INVALID_SOCKET == client) {
                xerror2(TSF"accept return client invalid:%_, err:(%_, %_)", client, socket_errno, socket_strerror(socket_errno)) >> break_group;
                break;
            }

            char cli_ip[16] = {0};
#ifdef WIN32
			socket_inet_ntop(AF_INET, &(client_addr.sin_addr), cli_ip, sizeof(cli_ip));
#else
			inet_ntop(AF_INET, &(client_addr.sin_addr), cli_ip, sizeof(cli_ip));
#endif // WIN32
            
            xinfo2(TSF"listen accept sock:(%_, %_:%_) cli:(%_, %_:%_)", listen_sock_, ip, ntohs(bind_addr_.sin_port), client, cli_ip, ntohs(client_addr.sin_port));

            observer_.OnAccept(this, client, client_addr);
        }
    } while (false);

    xinfo2(TSF"listen end sock:(%_, %_:%_), ", listen_sock_, ip, ntohs(bind_addr_.sin_port)) << break_group;

    if (INVALID_SOCKET != listen_sock_) {
        socket_close(listen_sock_);
        listen_sock_ = INVALID_SOCKET;
    }

    observer_.OnError(this, socket_errno);
}
