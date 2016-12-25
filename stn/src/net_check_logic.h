/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * net_check_logic.h
 *
 *  Created on: 2014-7-1
 *      Author: yanguoyue
 */

#ifndef STN_SRC_NET_CHECK_LOGIC_H_
#define STN_SRC_NET_CHECK_LOGIC_H_

#include <list>
#include <string>

#include "net_source.h"

class CommFrequencyLimit;

namespace mars {
    namespace stn {
    
class NetSource;

class NetCheckLogic {
  public:
    NetCheckLogic();
    ~NetCheckLogic();

    void UpdateLongLinkInfo(unsigned int _continues_fail_count, bool _task_succ);
    void UpdateShortLinkInfo(unsigned int _continue_fail_count, bool _task_succ);

  private:
    struct NetTaskStatusItem{
		uint32_t records;
		uint64_t last_failedtime;
		NetTaskStatusItem(): records(0xFFFFFFFF), last_failedtime(0) {}
	};

  private:
    bool __ShouldNetCheck();
    void __StartNetCheck();

  private:

    CommFrequencyLimit* frequency_limit_;
    NetSource::DnsUtil dns_util_;

    unsigned long long last_netcheck_time_;

    NetTaskStatusItem longlink_taskstatus_item_;;
    NetTaskStatusItem shortlink_taskstatus_item_;
};

    }
}

#endif // STN_SRC_NET_CHECK_LOGIC_H_
