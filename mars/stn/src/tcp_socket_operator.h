//
// Created by astro zhou on 2020/7/30.
//

#ifndef MMNET_TCPSOCKETOPERATOR_H
#define MMNET_TCPSOCKETOPERATOR_H

#include "socket_operator.h"
#include <memory>

#include "comm/socket/socketbreaker.h"
#include "comm/socket/complexconnect.h"

namespace mars {
	namespace stn {

class TcpSocketOperator : public SocketOperator {
public:
	TcpSocketOperator(std::shared_ptr<MComplexConnect> _observer);
	virtual ~TcpSocketOperator() override{
	}

	virtual SOCKET Connect(const std::vector<socket_address> &_vecaddr,
	                       mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone,
	                       const socket_address *_proxy_addr = NULL,
	                       const std::string &_proxy_username = "", const std::string &_proxy_pwd = "") override;
    
    virtual void Close(SOCKET _sock) override;
    virtual SocketCloseFunction GetCloseFunction() const override;

	virtual int
	Send(SOCKET _sock, const void *_buffer, size_t _len, int &_errcode, int _timeout) override;

	virtual int
	Recv(SOCKET _sock, AutoBuffer &_buffer, size_t _max_size, int &_errcode, int _timeout,
	     bool _wait_full_size) override;

	virtual std::string ErrorDesc(int _errcode) override;
    
    std::string Identify(SOCKET _sock) const override;
private:
    std::shared_ptr<MComplexConnect> observer_;
	SocketBreaker sBreaker_;
};

}
}


#endif //MMNET_TCPSOCKETOPERATOR_H
