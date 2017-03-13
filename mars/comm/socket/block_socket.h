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
//  block_socket.h
//  comm
//
//  Created by yerungui on 16/3/30.
//

#ifndef block_socket_h
#define block_socket_h

#include "comm/socket/unix_socket.h"

class SocketSelect;
class SocketBreaker;
class socket_address;
class AutoBuffer;

/*
 * param: timeoutInMs if set 0, then select timeout param is NULL, not timeval(0)
 * return value:
 */
SOCKET  block_socket_connect(socket_address& _address, SocketBreaker& _breaker, int& _errcode, int32_t _timeout=-1/*ms*/);
int     block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketBreaker& _breaker, int &_errcode, int _timeout=-1);
int     block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketBreaker& _breaker, int &_errcode, int _timeout=-1, bool _wait_full_size=false);
#endif

