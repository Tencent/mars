//
//  block_socket.h
//  comm
//
//  Created by yerungui on 16/3/30.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef block_socket_h
#define block_socket_h

#include "comm/socket/unix_socket.h"

class SocketSelect;
class SocketSelectBreaker;
class socket_address;
class AutoBuffer;

/*
 * param: timeoutInMs if set 0, then select timeout param is NULL, not timeval(0)
 * return value:
 */
SOCKET  block_socket_connect(socket_address& _address, SocketSelectBreaker& _breaker, int& _errcode, int32_t _timeout=-1/*ms*/);
int     block_socket_send(SOCKET _sock, const void* _buffer, size_t _len, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1);
int     block_socket_recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, SocketSelectBreaker& _breaker, int &_errcode, int _timeout=-1, bool _wait_full_size=false);
#endif

