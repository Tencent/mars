//
// Created by astro zhou on 2020/7/30.
//

#ifndef MMNET_SOCKET_OPERATER_H
#define MMNET_SOCKET_OPERATER_H

#include "comm/autobuffer.h"
#include "comm/socket/unix_socket.h"
#include "comm/socket/socket_address.h"
#include "comm/comm_data.h"
#include <memory>
#include <vector>
#include <string>

namespace mars {
	namespace stn {
        const int kOperatorProtocolTCP = 0;
        const int kOperatorProtocolQUIC = 1;
        
        typedef int(*SocketCloseFunction)(int);
		struct SocketProfile {
			SocketProfile():rtt(0),index(-1),errorCode(-1),totalCost(0) {}
			uint32_t rtt;
			int index;
			int errorCode;
			uint32_t totalCost;
		};

		class OPBreaker {
		public:
			virtual ~OPBreaker() {}
			virtual bool IsBreak() = 0;
			virtual bool Break() = 0;
		};

		class SocketOperator {
		public:
            SocketOperator(int _protocol):protocol_(_protocol){}
			virtual ~SocketOperator() {}
			virtual SOCKET Connect(const std::vector<socket_address>& _vecaddr, mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone, const socket_address* _proxy_addr = NULL,
			                       const std::string& _proxy_username = "", const std::string& _proxy_pwd = "") = 0;
			virtual int Send(SOCKET _sock, const void* _buffer, size_t _len, int &_errcode, int _timeout = -1) = 0;
			virtual int Recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, int &_errcode, int _timeout, bool _wait_full_size=false) = 0;
            virtual void Close(SOCKET _sock) = 0;
            virtual SocketCloseFunction GetCloseFunction() const = 0;
			virtual std::string ErrorDesc(int _errcode) = 0;
			virtual const SocketProfile& Profile() { return profile_; }
			virtual OPBreaker& Breaker() { return *breaker_.get(); }
            virtual std::string Identify(SOCKET _sock) const = 0;
            int Protocol() const {  return protocol_;   }
		protected:
            int protocol_;
            SocketProfile profile_;
            std::unique_ptr<OPBreaker> breaker_;
		};
	}
}


#endif //MMNET_SOCKET_OPERATER_H
