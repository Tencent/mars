// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * net_check_logic.cc
 *
 *  Created on: 2014-7-3
 *      Author: yanguoyue caoshaokun
 */

#include "net_check_logic.h"

#include <map>
#include <vector>

#include "boost/bind.hpp"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/comm_frequency_limit.h"
#include "mars/comm/time_utils.h"
#include "mars/sdt/sdt_logic.h"
#include "mars/sdt/constants.h"

#include "net_source.h"

using namespace mars::stn;
using namespace mars::sdt;

static const unsigned long kLimitTimeSpan = 60 * 60 * 1000;  // 60 min
static const size_t kLimitCount = 1;

static const long long kMinCheckTimeSpan = 5 * 60 * 1000; //5 min
static const long long kCheckTimeSpanIncrementStep = 10 * 60 * 1000; //10 min


static const uint32_t kValidBitsFilter = 0xFFFFFFFF;  //32 bits stands for recent 32 tasks status
static const uint32_t kMostRecentTaskStartN[2]			={24, 8};
static const uint32_t kSecondRecentTaskStartN[2] 		={16, 8};
//static const uint32_t MOST_OLD_TASK_START_N[2]				={1, 16};

static const uint32_t kCheckifBelowCount = 3;
static const uint32_t kCheckifAboveCount = 5;

#define SET_BIT(IS_TRUE, RECORDS, VALID_BITS) \
    do \
    { \
    	RECORDS <<= 1; \
        RECORDS &= VALID_BITS; \
        if (IS_TRUE) \
            RECORDS |= 1; \
        else \
            RECORDS &= ~1; \
    } \
    while(false)

//calc how many 1 in records
#define CAL_BIT_COUNT(RECORDS, COUNT) \
    do \
    { \
        uint32_t eva = RECORDS; \
        while(eva) \
        { \
            eva = eva & (eva-1); \
            COUNT++; \
        } \
    } \
    while(false)

//is the last n bits all zero
#define IS_LAST_N_BIT_ZERO(RECORDS, N, RET) \
	do \
	{ \
		uint32_t N_ = N; \
		if (N_>32) {N_=0; }\
		uint32_t mask = (0xFFFFFFFF >> (32-N_)); \
		RET = (0==((RECORDS & mask) ^ 0x00000000));\
	} \
	while(false)

//extact the N bits from RECORDS, which is started by START_POS, and return the result by RETURN_RECORD
#define EXTRACT_N_BITS(RECORDS, START_POS, N, RETURN_RECORD) \
	do \
	{ \
		uint32_t N_ = (N); \
		uint32_t START_POS_ = (START_POS);\
		if (START_POS_>32 || START_POS_<1) {START_POS_=1;} \
		if (START_POS_+N_>32) {N_=32-START_POS_;}\
		RETURN_RECORD = ((RECORDS << (START_POS_-1)) >> (32-N_)); \
	}\
	while(false)

NetCheckLogic::NetCheckLogic()
    : frequency_limit_(new CommFrequencyLimit(kLimitCount, kLimitTimeSpan))
	, last_netcheck_time_(0) {
    xinfo_function();
}

NetCheckLogic::~NetCheckLogic() {
    xinfo_function();

    delete frequency_limit_;
}

void NetCheckLogic::UpdateLongLinkInfo(unsigned int _continues_fail_count, bool _task_succ) {
	if (!_task_succ ) longlink_taskstatus_item_.last_failedtime = ::gettickcount();
	SET_BIT(_task_succ, longlink_taskstatus_item_.records, kValidBitsFilter);
	xdebug2("shortlink:_continueFailCount=%d, _isTaskSucc=%d, records=0x%x", _continues_fail_count, _task_succ, longlink_taskstatus_item_.records);

    if (__ShouldNetCheck()) {
        __StartNetCheck();
    }
}

void NetCheckLogic::UpdateShortLinkInfo(unsigned int _continues_fail_count, bool _task_succ) {

	if (!_task_succ ) shortlink_taskstatus_item_.last_failedtime = ::gettickcount();
	SET_BIT(_task_succ, shortlink_taskstatus_item_.records, kValidBitsFilter);
	xdebug2("shortlink:_continues_fail_count: %d, _task_succ: %d, records=0x%x", _continues_fail_count, _task_succ, shortlink_taskstatus_item_.records);

    if (__ShouldNetCheck()) {
        __StartNetCheck();
    }
}

bool NetCheckLogic::__ShouldNetCheck() {

    bool ret = false;

	bool shortlink_shouldcheck = false;
	uint32_t succ_count = 0;
	uint32_t most_recent_shorttasks_status = 0;

	EXTRACT_N_BITS(shortlink_taskstatus_item_.records, kMostRecentTaskStartN[0], kMostRecentTaskStartN[1], most_recent_shorttasks_status);
	CAL_BIT_COUNT(most_recent_shorttasks_status, succ_count);

	bool is_shortlink_bad = succ_count < kCheckifBelowCount;
	bool is_shortlink_good = succ_count > kCheckifAboveCount;
	if (is_shortlink_bad) { // 最近八次坏，前八次好，就进入netcheck
		unsigned int valid_record_taskcount = 0;
		CAL_BIT_COUNT(kValidBitsFilter, valid_record_taskcount);
		xinfo2(TSF"netcheck: shortlink succ_count: %_, is most recent %_ times. valid_record_taskcount: %_.", succ_count, kMostRecentTaskStartN[1], valid_record_taskcount);
		succ_count = 0;
		uint32_t second_recent_shorttasks_status = 0;
		EXTRACT_N_BITS(shortlink_taskstatus_item_.records, kSecondRecentTaskStartN[0], kSecondRecentTaskStartN[1], second_recent_shorttasks_status);
		CAL_BIT_COUNT(second_recent_shorttasks_status, succ_count);
		shortlink_shouldcheck = succ_count > kCheckifAboveCount;
		xinfo2(TSF"netcheck: shortlink_shouldcheck=%_, shortlink succ_count=%_, in sub-recent %_ times. ", shortlink_shouldcheck,
				succ_count, kSecondRecentTaskStartN[1]);
	}

	//long link
	bool longlink_shouldcheck = false;
	succ_count = 0;
	uint32_t most_recent_longtasks_status = 0;

	EXTRACT_N_BITS(longlink_taskstatus_item_.records, kMostRecentTaskStartN[0], kMostRecentTaskStartN[1], most_recent_longtasks_status);
	CAL_BIT_COUNT(most_recent_longtasks_status, succ_count);

	bool is_longlink_bad = succ_count < kCheckifBelowCount;
	bool is_longlink_good = succ_count > kCheckifAboveCount;
	if (is_longlink_bad) { // 最近八次坏，前八次好，就进入netcheck
		unsigned int valid_record_taskcount = 0;
		CAL_BIT_COUNT(kValidBitsFilter, valid_record_taskcount);
		xinfo2(TSF"netcheck: longlink succ_count: %_, in most recent %_ times. valid_record_taskcount: %_.", succ_count, kMostRecentTaskStartN[1], valid_record_taskcount);
		succ_count = 0;
		uint32_t second_recent_longtasks_status = 0;
		EXTRACT_N_BITS(longlink_taskstatus_item_.records, kSecondRecentTaskStartN[0], kSecondRecentTaskStartN[1], second_recent_longtasks_status);
		CAL_BIT_COUNT(second_recent_longtasks_status, succ_count);
		longlink_shouldcheck = succ_count > kCheckifAboveCount;
		xinfo2(TSF"netcheck: longlink_shouldcheck: %_, longlink succ_count: %_, in sub-recent %_ times. ", longlink_shouldcheck,
				succ_count, kSecondRecentTaskStartN[1]);
	}


	ret = longlink_shouldcheck || shortlink_shouldcheck;
	static int increment_steps = 0;
	if (ret) {
		 if (::gettickspan(last_netcheck_time_) < (kMinCheckTimeSpan + increment_steps * kCheckTimeSpanIncrementStep)) {
			 ret = false;
			 xinfo2(TSF"continous hit netcheck strategy, skip this. last_netcheck_time_=%_", last_netcheck_time_);
		 } else {
			 increment_steps ++;
		 }
	}
	if (is_shortlink_good && is_longlink_good) {// network is stable now.
		increment_steps = 0;
	}

    if (ret && !frequency_limit_->Check()) {
        xinfo2(TSF"limit, wait!");
        return false;
    }

    return ret;
}

void NetCheckLogic::__StartNetCheck() {

	//get longlink check map
	CheckIPPorts longlink_check_items;
	std::vector<std::string> longlink_hosts = NetSource::GetLongLinkHosts();
	if (longlink_hosts.empty()) {
		xerror2(TSF"longlink host is empty.");
		return;
	}

	std::vector<uint16_t> longlink_portlist;
	NetSource::GetLonglinkPorts(longlink_portlist);
	if (longlink_portlist.empty()) {
		xerror2(TSF"longlink no port");
		return;
	}

	for (std::vector<std::string>::iterator host_iter = longlink_hosts.begin(); host_iter != longlink_hosts.end(); ++host_iter) {
		std::vector<std::string> longlink_iplist;
		dns_util_.GetNewDNS().GetHostByName(*host_iter, longlink_iplist);
		if (longlink_iplist.empty()) dns_util_.GetDNS().GetHostByName(*host_iter, longlink_iplist);
		if (longlink_iplist.empty()) {
			xerror2(TSF"no dns ip for longlink host: %_", *host_iter);
			continue;
		}

		std::vector<CheckIPPort> check_ipport_list;
		for (std::vector<uint16_t>::iterator port_iter = longlink_portlist.begin(); port_iter != longlink_portlist.end(); ++port_iter) {
			for (std::vector<std::string>::iterator ip_iter = longlink_iplist.begin(); ip_iter != longlink_iplist.end(); ++ip_iter) {
				CheckIPPort ipport_item(*ip_iter, *port_iter);
				check_ipport_list.push_back(ipport_item);
			}
		}

		if (!check_ipport_list.empty()) longlink_check_items.insert(std::pair< std::string, std::vector<CheckIPPort> >(*host_iter, check_ipport_list));
	}

	//shortlink check map
	CheckIPPorts shortlink_check_items;
	std::vector<std::string> shortlink_hostlist;
	RequestNetCheckShortLinkHosts(shortlink_hostlist);
	uint16_t shortlink_port = NetSource::GetShortLinkPort();


	for (std::vector<std::string>::iterator iter = shortlink_hostlist.begin(); iter != shortlink_hostlist.end(); ++iter) {
		if (longlink_portlist.empty()) {
			xerror2(TSF"longlink no port");
			break;
		}

		std::vector<std::string> shortlink_iplist;
		dns_util_.GetNewDNS().GetHostByName(*iter, shortlink_iplist);
		if (shortlink_iplist.empty()) dns_util_.GetDNS().GetHostByName(*iter, shortlink_iplist);
		if (shortlink_iplist.empty()) {
			xerror2(TSF"no dns ip for longlink host: %_", *iter);
			continue;
		}

		std::vector<CheckIPPort> check_ipport_list;
		for (std::vector<std::string>::iterator ip_iter = shortlink_iplist.begin(); ip_iter != shortlink_iplist.end(); ++ip_iter) {
			CheckIPPort ipport_item(*ip_iter, shortlink_port);
			check_ipport_list.push_back(ipport_item);
		}

		if (!check_ipport_list.empty()) shortlink_check_items.insert(std::pair< std::string, std::vector<CheckIPPort> >(*iter, check_ipport_list));
	}


    int mode = (NET_CHECK_BASIC | NET_CHECK_LONG | NET_CHECK_SHORT);
    if (!longlink_check_items.empty() || !shortlink_check_items.empty()) StartActiveCheck(longlink_check_items, shortlink_check_items, mode, UNUSE_TIMEOUT);
}
