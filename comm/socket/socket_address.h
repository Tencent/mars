/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * socket_address.h
 *
 *  Created on: 2014-12-1
 *      Author: yerungui
 */

#ifndef SOCKET_ADDRESS_H_
#define SOCKET_ADDRESS_H_

#include "unix_socket.h"

#include <string>

class socket_address {

  public:
    socket_address(const char* _url);
    explicit socket_address(const char* _ip, uint16_t _port);
    explicit socket_address(const sockaddr_in& _addr);
    explicit socket_address(const sockaddr_in6& _addr);
    explicit socket_address(const sockaddr* _addr);
    explicit socket_address(const struct in_addr& _in_addr);
    explicit socket_address(const struct in6_addr& _in6_addr);

    const sockaddr& address_fix();
    const sockaddr& address() const;
    socklen_t address_length() const;

    const char* url() const;
    const char* ip() const;
    const char* ipv6() const;
    uint16_t port() const;

    bool valid() const;
    bool valid_server_address(bool _allowloopback = false) const;
    bool valid_bind_address() const;
    bool valid_broadcast_address() const;

    bool valid_loopback_ip() const;
    bool valid_broadcast_ip() const;

    bool isv4mapped_address() const;
    socket_address& v4tov4mapped_address();

    socket_address& v4tonat64_address();

    socket_address& v4tov6_address(bool _nat64=false);

    bool is_ipport_equal(const socket_address& _sa) const {return 0==strncmp(ip(), _sa.ip(), sizeof(ip_)) && port()==_sa.port();}
	bool fix_current_nat64_addr();

    static bool update_nat64_prefix();
  public:
    static socket_address getsockname(SOCKET _sock);
	static socket_address getpeername(SOCKET _sock);

  private:
    //    socket_address(const socket_address&);
    //    const socket_address& operator=(const socket_address&);
    void  __init(const sockaddr*  _addr);

  private:

    union {
        struct sockaddr     sa;
        struct sockaddr_in  in;
        struct sockaddr_in6 in6;
    }                   addr_;
    char                ip_[64];
    char                url_[128];
};

#endif /* SOCKET_ADDRESS_H_ */
