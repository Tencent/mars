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
#include <map>

#include "boost/function.hpp"

namespace mars {
namespace comm {

struct dnsinfo;

struct DNSBreaker {
    DNSBreaker() : isbreak(false), dnsstatus(NULL) {
    }
    bool isbreak;
    int* dnsstatus;

    void Clear() {
        isbreak = false;
        dnsstatus = NULL;
    }
};

class DNS {
 public:
    //    typedef std::vector<std::string> (*DNSFunc)(const std::string& _host, bool _longlink_host);

 public:
    DNS(const std::function<std::vector<std::string>(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _extra_info)>& _dnsfunc = NULL);
    ~DNS();

 public:
    bool GetHostByName(const std::string& _host_name,
                       std::vector<std::string>& ips,
                       long millsec = 2000,
                       DNSBreaker* _breaker = NULL,
                       bool _longlink_host = false,
                       const std::map<std::string, std::string>& _extra_info = {});
    void Cancel(const std::string& _host_name = std::string());
    void Cancel(DNSBreaker& _breaker);

    void SetMonitorFunc(const boost::function<void(int _key)>& _monitor_func) {
        monitor_func_ = _monitor_func;
    }

    void SetDnsFunc(
        const std::function<std::vector<std::string>(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _extra_info)>& _dnsfunc) {
        dnsfunc_ = _dnsfunc;
    }

 private:
    void __GetIP();

 private:
    //    DNSFunc dnsfunc_;
    std::function<std::vector<std::string>(const std::string& _host, bool _longlink_host, const std::map<std::string, std::string>& _extra_info)> dnsfunc_;
    boost::function<void(int _key)> monitor_func_;
    static const int kDNSThreadIDError = 0;
};
}  // namespace comm
}  // namespace mars

#endif /* COMM_COMM_DNS_H_ */
