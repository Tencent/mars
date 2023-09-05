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
 * comm_dns.cc
 *
 *  Created on: 2012-11-23
 *      Author: yanguoyue
 */

#include <utility>

#include "dns/dns.h"

#include "mars/comm/macro.h"
#include "network/getdnssvraddrs.h"
#include "socket/local_ipstack.h"
#include "socket/socket_address.h"
#include "socket/unix_socket.h"
#include "thread/condition.h"
#include "thread/lock.h"
#include "thread/thread.h"
#include "time_utils.h"
#include "xlogger/xlogger.h"

namespace mars {
namespace comm {

enum {
    kGetIPDoing,
    kGetIPTimeout,
    kGetIPCancel,
    kGetIPSuc,
    kGetIPFail,
};

struct DnsInfo {
    thread_tid threadid{};
    DNS* dns{};
    DNS::DNSFunc dns_func;
    std::string host_name;
    std::vector<std::string> result;
    int status{};
    bool longlink_host = false;
};

std::string DNSInfoToString(const struct DnsInfo& _info) {
    XMessage msg;
    msg(TSF "info:%_, threadid:%_, dns:%_, host_name:%_, status:%_",
        &_info,
        _info.threadid,
        _info.dns,
        _info.host_name,
        _info.status);
    return msg.Message();
}
NO_DESTROY static std::vector<DnsInfo> sg_dnsinfo_vec;
NO_DESTROY static Condition sg_condition;
NO_DESTROY static Mutex sg_mutex;
void DNS::__GetIP() {
    xverbose_function();

    auto start_time = ::gettickcount();

    ScopedLock lock(sg_mutex);
    auto iter = std::find_if(sg_dnsinfo_vec.begin(), sg_dnsinfo_vec.end(), [](const DnsInfo&dnsinfo) {
        return dnsinfo.threadid == ThreadUtil::currentthreadid();
    });
    if (iter == sg_dnsinfo_vec.end()) {
        assert(false);
        return;
    }
    std::string host_name = iter->host_name;
    DNS::DNSFunc dnsfunc = iter->dns_func;
    bool longlink_host = iter->longlink_host;

    lock.unlock();
    xdebug2(TSF "dnsfunc is null: %_, %_", host_name, (dnsfunc == nullptr));
    if (dnsfunc) {
        std::vector<std::string> ips;
        if (iter->status != kGetIPCancel) {
            ips = dnsfunc(host_name, longlink_host);
        }

        lock.lock();

        iter = std::find_if(sg_dnsinfo_vec.begin(), sg_dnsinfo_vec.end(), [](const DnsInfo&dnsinfo) {
            return dnsinfo.threadid == ThreadUtil::currentthreadid();
        });

        if (iter != sg_dnsinfo_vec.end()) {
            iter->status = ips.empty() ? kGetIPFail : kGetIPSuc;
            iter->result = ips;
        }
        sg_condition.notifyAll();
        return;
    }

    xgroup2_define(log_group);
    std::vector<socket_address> dnssvraddrs;
    mars::comm::getdnssvraddrs(dnssvraddrs);
    xinfo2("dns server:") >> log_group;
    for (const auto &it : dnssvraddrs) {
        xinfo2(TSF "%_:%_ ", it.ip(), it.port()) >> log_group;
    }

    //
    struct addrinfo hints{}, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // in iOS work fine, in Android ipv6 stack get ipv4-ip fail
    // and in ipv6 stack AI_ADDRCONFIGd will filter ipv4-ip but we ipv4-ip can use by nat64
    //    hints.ai_flags = AI_V4MAPPED|AI_ADDRCONFIG;
    int error;
    TLocalIPStack ipstack = local_ipstack_detect();
    if (ELocalIPStack_IPv4 == ipstack) {
        error = getaddrinfo(host_name.c_str(), nullptr, &hints, &result);
    } else {
        error = getaddrinfo(host_name.c_str(), nullptr, /*&hints*/ nullptr, &result);
    }

    lock.lock();

    iter = std::find_if(sg_dnsinfo_vec.begin(), sg_dnsinfo_vec.end(), [](const DnsInfo&dnsinfo) {
        return dnsinfo.threadid == ThreadUtil::currentthreadid();
    });

    if (error != 0) {
        xwarn2(TSF "error, error:%_/%_, hostname:%_, ipstack:%_",
               error,
               strerror(error),
               host_name.c_str(),
               ipstack);

        if (iter != sg_dnsinfo_vec.end())
            iter->status = kGetIPFail;

        sg_condition.notifyAll();
        return;
    }
    if (iter == sg_dnsinfo_vec.end()) {
        freeaddrinfo(result);
        return;
    }

    for (struct addrinfo *single = result; single; single = single->ai_next) {
        // In Indonesia, if there is no ipv6's ip, operators return 0.0.0.0.
        if (PF_INET == single->ai_family) {
            auto* addr_in = (sockaddr_in*)single->ai_addr;
            if (INADDR_ANY == addr_in->sin_addr.s_addr || INADDR_NONE == addr_in->sin_addr.s_addr) {
                xwarn2(TSF "hehe, addr_in->sin_addr.s_addr:%0", addr_in->sin_addr.s_addr);
                continue;
            }
        }

        socket_address sock_addr(single->ai_addr);
        const char* ip = sock_addr.ip();

        if (!socket_address(ip, 0).valid_server_address(false, true)) {
            xerror2(TSF "ip is invalid, ip:%0", ip);
            continue;
        }

        iter->result.emplace_back(ip);
    }

    //
    xgroup2_define(ip_group);
    xinfo2(TSF "host %_ resolved iplist: ", host_name) >> ip_group;
    for (auto ip : iter->result) {
        xinfo2(TSF "%_,", ip) >> ip_group;
    }

    freeaddrinfo(result);
    iter->status = kGetIPSuc;
    xinfo2(TSF "cost time: %_", (::gettickcount() - start_time)) >> ip_group;
    sg_condition.notifyAll();
}

///////////////////////////////////////////////////////////////////
DNS::DNS(DNSFunc  _dnsfunc)
: dnsfunc_(std::move(_dnsfunc)) {
}

DNS::~DNS() {
    Cancel();
}

bool DNS::GetHostByName(const std::string& _host_name,
                        std::vector<std::string>& ips,
                        long millsec,
                        DNSBreaker* _breaker,
                        bool _longlink_host) {
    xverbose_function("host: %s, longlink: %d", _host_name.c_str(), _longlink_host);

    xassert2(!_host_name.empty());

    if (_host_name.empty()) {
        return false;
    }

    ScopedLock lock(sg_mutex);

    if (_breaker && _breaker->isbreak)
        return false;

    Thread thread(std::bind(&DNS::__GetIP, this), _host_name.c_str());
    int startRet = thread.start();

    if (startRet != 0) {
        xerror2(TSF "start the thread fail");
        return false;
    }

    DnsInfo info;
    info.threadid = thread.tid();
    info.host_name = _host_name;
    info.dns_func = dnsfunc_;
    info.dns = this;
    info.status = kGetIPDoing;
    info.longlink_host = _longlink_host;
    sg_dnsinfo_vec.push_back(info);

    if (_breaker)
        _breaker->dnsstatus = &(sg_dnsinfo_vec.back().status);

    uint64_t time_end = gettickcount() + (uint64_t)millsec;

    while (true) {
        uint64_t time_cur = gettickcount();
        uint64_t time_wait = time_end > time_cur ? time_end - time_cur : 0;

        int wait_ret = sg_condition.wait(lock, (long)time_wait);

        auto it = std::find_if(sg_dnsinfo_vec.begin(), sg_dnsinfo_vec.end(), [&info](const DnsInfo&it) {
            return it.threadid == info.threadid;
        });

        xassert2(it != sg_dnsinfo_vec.end());

        if (it == sg_dnsinfo_vec.end()) {
            return false;
        }

        if (ETIMEDOUT == wait_ret) {
            it->status = kGetIPTimeout;
        }

        if (kGetIPDoing == it->status) {
            continue;
        }

        if (kGetIPSuc == it->status) {
            if (_host_name == it->host_name) {
                ips = it->result;

                if (_breaker)
                    _breaker->dnsstatus = nullptr;

                sg_dnsinfo_vec.erase(it);
                return true;
            } else {
                for (size_t i = 0; i < sg_dnsinfo_vec.size(); ++i) {
                    xerror2(TSF "sg_info_vec[%_]:%_", i, DNSInfoToString(sg_dnsinfo_vec[i]));
                }
                if (monitor_func_)
                    monitor_func_(kDNSThreadIDError);
                xassert2(false, TSF "_host_name:%_, it->host_name:%_", _host_name, it->host_name);
                return false;
            }
        }

        if (kGetIPTimeout == it->status || kGetIPCancel == it->status || kGetIPFail == it->status) {
            if (_breaker)
                _breaker->dnsstatus = nullptr;

            // xinfo2(TSF "dns get ip status:%_ host:%_, func:%_", it->status, it->host_name, it->dns_func);
            xinfo2(TSF "dns get ip status:%_ host:%_", it->status, it->host_name);
            sg_dnsinfo_vec.erase(it);
            return false;
        }

        xassert2(false, "%d", it->status);

        if (_breaker)
            _breaker->dnsstatus = nullptr;

        sg_dnsinfo_vec.erase(it);
    }
}

void DNS::Cancel(const std::string& _host_name) {
    xverbose_function();
    ScopedLock lock(sg_mutex);

    for (auto & info : sg_dnsinfo_vec) {
        if (_host_name.empty() && info.dns == this) {
            info.status = kGetIPCancel;
            info.dns_func = nullptr;
        }

        if (info.host_name == _host_name && info.dns == this) {
            info.status = kGetIPCancel;
            info.dns_func = nullptr;
        }
    }

    sg_condition.notifyAll();
}

void DNS::Cancel(DNSBreaker& _breaker) {
    ScopedLock lock(sg_mutex);
    _breaker.isbreak = true;

    if (_breaker.dnsstatus)
        *(_breaker.dnsstatus) = kGetIPCancel;

    sg_condition.notifyAll();
}

}  // namespace comm
}  // namespace mars
