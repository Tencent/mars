/*
 * dns_profile.h
 *
 *  Created on: 2016年8月12日
 *      Author: caoshaokun
 */

#ifndef MARS_OPEN_MARS_STN_DNS_PROFILE_H_
#define MARS_OPEN_MARS_STN_DNS_PROFILE_H_

#include <string>

#include "mars/stn/stn.h"

namespace mars {
namespace stn {

enum DnsType {
	kType_NewDns = 1,
	kType_Dns = 2
};

struct DnsProfile {

	DnsProfile() {
		Reset();
	}

	void Reset() {
		start_time = gettickcount();
		end_time = 0;

		host.clear();

		err_type = 0;
		err_code = 0;

		dnstype = kType_NewDns;
	}

	void OnFailed() {
		err_type = kEctLocal;
		err_code = -1;
	}

	uint64_t start_time;
	uint64_t end_time;

	std::string host;

	int err_type;
	int err_code;

	int dnstype;

};

}
}


#endif /* MARS_OPEN_MARS_STN_DNS_PROFILE_H_ */
