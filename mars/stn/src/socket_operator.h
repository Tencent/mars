//
// Created by astro zhou on 2020/7/30.
//

#ifndef MMNET_SOCKET_OPERATER_H
#define MMNET_SOCKET_OPERATER_H

#include "comm/autobuffer.h"
#include "comm/socket/unix_socket.h"
#include "comm/socket/socket_address.h"
#include "comm/comm_data.h"
#include <vector>
#include <string>

namespace mars {
	namespace stn {
		struct SocketProfile {
			SocketProfile():rtt(0),index(-1),errorCode(-1),totalCost(0) {}
			uint32_t rtt;
			int index;
			int errorCode;
			uint32_t totalCost;
		};

		class Breaker {
		public:
			virtual ~Breaker() {}
			virtual bool IsBreak() = 0;
			virtual bool Break() = 0;
		};

		class SocketOperator {
		public:
			virtual ~SocketOperator() {}
			virtual SOCKET Connect(const std::vector<socket_address>& _vecaddr, mars::comm::ProxyType _proxy_type = mars::comm::kProxyNone, const socket_address* _proxy_addr = NULL,
			                       const std::string& _proxy_username = "", const std::string& _proxy_pwd = "") = 0;
			virtual int Send(SOCKET _sock, const void* _buffer, size_t _len, int &_errcode, int _timeout = -1) = 0;
			virtual int Recv(SOCKET _sock, AutoBuffer& _buffer, size_t _max_size, int &_errcode, int _timeout, bool _wait_full_size=false) = 0;
			virtual std::string ErrorDesc(int _errcode) = 0;
			virtual const SocketProfile& Profile() { return profile_; }
			virtual Breaker& Breaker() { return *breaker_; }

		protected:
			SocketProfile profile_;
			class Breaker* breaker_;
		};
	}
}


#endif //MMNET_SOCKET_OPERATER_H
