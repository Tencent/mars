//
// Created by astro zhou on 2020/7/30.
//

#include "tcp_socket_operator.h"

#include <comm/socket/block_socket.h>
#include <comm/xlogger/xlogger.h>

#include <memory>

#include "mars/comm/socket/unix_socket.h"  // for socket_close
#include "mars/stn/config.h"

using namespace mars::comm;

namespace mars {
namespace stn {

bool ContainIPv6(const std::vector<socket_address>& _vecaddr) {
    if (_vecaddr.empty()) {
        return false;
    }
    for (auto address : _vecaddr) {
        in6_addr addr6 = IN6ADDR_ANY_INIT;
        if (socket_inet_pton(AF_INET6, address.ip(), &addr6)) {  // first ip is ipv6
            xinfo2(TSF "ip %_ is v6", address.ip());
            return true;
        }
    }
    return false;
}

class TcpBreaker : public OPBreaker {
 public:
    TcpBreaker(SocketBreaker& _breaker) : breaker_(_breaker) {
    }
    bool IsBreak() {
        return breaker_.IsBreak();
    }
    bool Break() {
        return breaker_.Break();
    }

 private:
    SocketBreaker& breaker_;
};

TcpSocketOperator::TcpSocketOperator(std::shared_ptr<MComplexConnect> _observer)
: SocketOperator(), observer_(_observer) {
    breaker_ = std::make_unique<TcpBreaker>(sBreaker_);
}

SOCKET TcpSocketOperator::Connect(const std::vector<socket_address>& _vecaddr,
                                  mars::comm::ProxyType _proxy_type,
                                  const socket_address* _proxy_addr,
                                  const std::string& _proxy_username,
                                  const std::string& _proxy_pwd) {
    std::shared_ptr<ComplexConnect> conn;
    if (ContainIPv6(_vecaddr) && v4_timeout_ > 0 && v6_timeout_ > 0) {
        conn = std::make_shared<ComplexConnect>(kShortlinkConnTimeout,
                                                kShortlinkConnInterval,
                                                kShortlinkConnInterval,
                                                v4_timeout_,
                                                v6_timeout_);
    } else {
        conn = std::make_shared<ComplexConnect>(kShortlinkConnTimeout, kShortlinkConnInterval);
    }
    SOCKET sock = conn->ConnectImpatient(_vecaddr,
                                         sBreaker_,
                                         observer_.get(),
                                         _proxy_type,
                                         _proxy_addr,
                                         _proxy_username,
                                         _proxy_pwd);
    xinfo2(TSF "connect result socket: %_", sock);
    profile_.index = conn->Index();
    profile_.rtt = conn->IndexRtt();
    profile_.errorCode = conn->ErrorCode();
    profile_.totalCost = conn->TotalCost();
    return sock;
}

void TcpSocketOperator::Close(SOCKET _sock) {
    socket_close(_sock);
}

int TcpSocketOperator::Send(SOCKET _sock, const void* _buffer, size_t _len, int& _errcode, int _timeout) {
    return block_socket_send(_sock, _buffer, _len, sBreaker_, _errcode);
}

int TcpSocketOperator::Recv(SOCKET _sock,
                            AutoBuffer& _buffer,
                            size_t _max_size,
                            int& _errcode,
                            int _timeout,
                            bool _wait_full_size) {
    return block_socket_recv(_sock, _buffer, _max_size, sBreaker_, _errcode, _timeout);
}

std::string TcpSocketOperator::ErrorDesc(int _errcode) {
    return strerror(_errcode);
}

std::string TcpSocketOperator::Identify(SOCKET _sock) const {
    char szmsg[64];
    snprintf(szmsg, sizeof(szmsg), "%d@TCP", _sock);

    return std::string(szmsg);
}

}  // namespace stn
}  // namespace mars
