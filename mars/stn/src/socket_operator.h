//
// Created by astro zhou on 2020/7/30.
//

#ifndef MMNET_SOCKET_OPERATER_H
#define MMNET_SOCKET_OPERATER_H

#include <memory>
#include <string>
#include <vector>

#include "comm/autobuffer.h"
#include "comm/comm_data.h"
#include "comm/socket/socket_address.h"
#include "comm/socket/unix_socket.h"

namespace mars {
namespace stn {
typedef int (*SocketCloseFunc)(SOCKET);
typedef SOCKET (*CreateStreamFunc)(SOCKET);
typedef bool (*IsSubStreamFunc)(SOCKET);
struct SocketProfile {
    uint32_t rtt = 0;
    int index = 0;
    int errorCode = 0;
    uint32_t totalCost = 0;
    int is0rtt = 0;
};

class OPBreaker {
 public:
    virtual ~OPBreaker() {
    }
    virtual bool IsBreak() = 0;
    virtual bool Break() = 0;
};

class SocketOperator {
 public:
    SocketOperator() {
    }
    virtual ~SocketOperator() {
    }
    virtual SOCKET Connect(const std::vector<socket_address>& _vecaddr,
                           mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone,
                           const socket_address* _proxy_addr = NULL,
                           const std::string& _proxy_username = "",
                           const std::string& _proxy_pwd = "") = 0;
    virtual int Send(SOCKET _sock, const void* _buffer, size_t _len, int& _errcode, int _timeout = -1) = 0;
    virtual int Recv(SOCKET _sock,
                     AutoBuffer& _buffer,
                     size_t _max_size,
                     int& _errcode,
                     int _timeout,
                     bool _wait_full_size = false) = 0;
    virtual void Close(SOCKET _sock) = 0;
    virtual SocketCloseFunc GetCloseFunction() const = 0;
    virtual CreateStreamFunc GetCreateStreamFunc() const = 0;
    virtual IsSubStreamFunc GetIsSubStreamFunc() const = 0;
    virtual std::string ErrorDesc(int _errcode) = 0;
    virtual const SocketProfile& Profile() {
        return profile_;
    }
    virtual OPBreaker& Breaker() {
        return *breaker_.get();
    }
    virtual std::string Identify(SOCKET _sock) const = 0;
    virtual int Protocol() const = 0;  // return Task::kTransportProtocol xxx
    virtual SOCKET CreateStream(SOCKET _sock) = 0;
    virtual void SetIpConnectionTimeout(uint32_t _v4_timeout, uint32_t _v6_timeout) {
    }

 protected:
    SocketProfile profile_;
    std::unique_ptr<OPBreaker> breaker_;
};
}  // namespace stn
}  // namespace mars

#endif  // MMNET_SOCKET_OPERATER_H
