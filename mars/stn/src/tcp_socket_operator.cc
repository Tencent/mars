//
// Created by astro zhou on 2020/7/30.
//

#include "tcp_socket_operator.h"
#include "config.h"
#include <comm/xlogger/xlogger.h>
#include <comm/socket/block_socket.h>

namespace mars {
	namespace stn {

bool ContainIPv6(const std::vector<socket_address>& _vecaddr) {
	if (!_vecaddr.empty()) {
		in6_addr addr6 = IN6ADDR_ANY_INIT;
		if (socket_inet_pton(AF_INET6, _vecaddr[0].ip(), &addr6)) { //first ip is ipv6
			xinfo2(TSF"ip %_ is v6", _vecaddr[0].ip());
			return true;
		}
	}
	return false;
}

SOCKET TcpSocketOperator::Connect(const std::vector<socket_address> &_vecaddr, SocketBreaker &_breaker,
               mars::comm::ProxyType _proxy_type,
               const socket_address *_proxy_addr,
               const std::string &_proxy_username, const std::string &_proxy_pwd) {
	ComplexConnect::EachIPConnectTimoutMode timoutMode = ComplexConnect::EachIPConnectTimoutMode::MODE_FIXED;
	bool contain_v6 = ContainIPv6(_vecaddr);
	if (contain_v6) {
		timoutMode = ComplexConnect::EachIPConnectTimoutMode::MODE_INCREASE;
	} else {
		xinfo2(TSF"address vector has no ipv6");
	}
	ComplexConnect conn(kShortlinkConnTimeout, kShortlinkConnInterval, timoutMode);
	SOCKET sock = conn.ConnectImpatient(_vecaddr, _breaker, observer_, _proxy_type, _proxy_addr, _proxy_username, _proxy_pwd);
	xinfo2(TSF"connect result socket: %_", sock);
	profile_.index = conn.Index();
	profile_.rtt = conn.IndexRtt();
	profile_.errorCode = conn.ErrorCode();
	profile_.totalCost = conn.TotalCost();
	return sock;
}

int TcpSocketOperator::Send(SOCKET _sock, const void *_buffer, size_t _len, SocketBreaker &_breaker, int &_errcode, int _timeout) {
	return block_socket_send(_sock, _buffer, _len, _breaker, _errcode);
}

int TcpSocketOperator::Recv(SOCKET _sock, AutoBuffer &_buffer, size_t _max_size, SocketBreaker &_breaker, int &_errcode, int _timeout,
bool _wait_full_size) {
	return block_socket_recv(_sock, _buffer, _max_size, _breaker, _errcode, _timeout);
}

std::string TcpSocketOperator::ErrorDesc(int _errcode) {
	return strerror(_errcode);
}

	}
}