//
// Created by astro zhou on 2020/7/30.
//

#ifndef MMNET_TCPSOCKETOPERATOR_H
#define MMNET_TCPSOCKETOPERATOR_H

#include "socket_operator.h"
#include "comm/socket/complexconnect.h"

namespace mars {
	namespace stn {

class TcpSocketOperator : public SocketOperator {
public:
	TcpSocketOperator(MComplexConnect* _observer) : observer_(_observer) {}
	~TcpSocketOperator() {
		delete observer_;
	}

	virtual SOCKET Connect(const std::vector<socket_address> &_vecaddr, SocketBreaker &_breaker,
	                       mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone,
	                       const socket_address *_proxy_addr = NULL,
	                       const std::string &_proxy_username = "", const std::string &_proxy_pwd = "");

	virtual int
	Send(SOCKET _sock, const void *_buffer, size_t _len, SocketBreaker &_breaker, int &_errcode, int _timeout);

	virtual int
	Recv(SOCKET _sock, AutoBuffer &_buffer, size_t _max_size, SocketBreaker &_breaker, int &_errcode, int _timeout,
	     bool _wait_full_size);

	virtual std::string ErrorDesc(int _errcode);

private:
	MComplexConnect* observer_;
};

}
}


#endif //MMNET_TCPSOCKETOPERATOR_H
