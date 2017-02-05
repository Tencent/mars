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
 * dns.h
 *
 *  Created on: 2012-11-23
 *      Author: yanguoyue
 */

#ifndef COMM_COMM_DNS_H_
#define COMM_COMM_DNS_H_

#include <string>
#include <vector>

#include "boost/function.hpp"

struct DNSBreaker {
	DNSBreaker(): isbreak(false), dnsstatus(NULL) {}
	bool isbreak;
	int* dnsstatus;
};

class DNS {
  public:
   typedef std::vector<std::string> (*DNSFunc)(const std::string& host);

  public:
    DNS(DNSFunc _dnsfunc=NULL);
    ~DNS();
    
  public:
    bool GetHostByName(const std::string& _host_name, std::vector<std::string>& ips, long millsec = 2 * 1000, DNSBreaker* _breaker = NULL);
    void Cancel(const std::string& _host_name = std::string());
    void Cancel(DNSBreaker& _breaker);
    
    void SetMonitorFunc(const boost::function<void (int _key)>& _monitor_func) {
    	monitor_func_ = _monitor_func;
    }
  private:
    DNSFunc dnsfunc_;
    boost::function<void (int _key)> monitor_func_;
    static const int kDNSThreadIDError = 0;
};


#endif /* COMM_COMM_DNS_H_ */
