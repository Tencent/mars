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

namespace mars {
namespace comm {

struct DnsInfo;

struct DNSBreaker {
    DNSBreaker() : isbreak(false), dnsstatus(nullptr) {
    }
    bool isbreak;
    int* dnsstatus;

    void Clear() {
        isbreak = false;
        dnsstatus = nullptr;
    }
};

class DNS {
 public:
    typedef std::function<std::vector<std::string>(const std::string& _host, bool _longlink_host)> DNSFunc;
    typedef boost::function<void(int _key)> MonitorFunc;

 public:
    DNS(DNSFunc  _dnsfunc = nullptr);
    ~DNS();

 public:
    bool GetHostByName(const std::string& _host_name,
                       std::vector<std::string>& ips,
                       long millsec = 2000,
                       DNSBreaker* _breaker = nullptr,
                       bool _longlink_host = false);
    void Cancel(const std::string& _host_name = std::string());
    static void Cancel(DNSBreaker& _breaker);

    void SetMonitorFunc(const MonitorFunc& _monitor_func) {
        monitor_func_ = _monitor_func;
    }

    void SetDnsFunc(const DNSFunc& _dnsfunc) {
        dnsfunc_ = _dnsfunc;
    }

 private:
    static void __GetIP();

 private:
    DNSFunc dnsfunc_;
    MonitorFunc monitor_func_;
    static const int kDNSThreadIDError = 0;
};
}  // namespace comm
}  // namespace mars

#endif /* COMM_COMM_DNS_H_ */
