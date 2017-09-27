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

#include "dns/dns.h"
#include "socket/unix_socket.h"
#include "xlogger/xlogger.h"
#include "time_utils.h"
#include "socket/socket_address.h"
#include "thread/condition.h"
#include "thread/thread.h"
#include "thread/lock.h"

#include "network/getdnssvraddrs.h"

enum {
    kGetIPDoing,
    kGetIPTimeout,
    kGetIPCancel,
    kGetIPSuc,
    kGetIPFail,
};

struct dnsinfo {
    thread_tid      threadid;
    DNS*            dns;
    DNS::DNSFunc    dns_func;
    std::string     host_name;
    std::vector<std::string> result;
    int status;
};

static std::string DNSInfoToString(const struct dnsinfo& _info) {
	XMessage msg;
	msg(TSF"info:%_, threadid:%_, dns:%_, host_name:%_, status:%_", &_info, _info.threadid, _info.dns, _info.host_name, _info.status);
	return msg.Message();
}
static std::vector<dnsinfo> sg_dnsinfo_vec;
static Condition sg_condition;
static Mutex sg_mutex;

static void __GetIP() {
    xverbose_function();


    std::string host_name;
    DNS::DNSFunc dnsfunc = NULL;

    ScopedLock lock(sg_mutex);
    std::vector<dnsinfo>::iterator iter = sg_dnsinfo_vec.begin();

    for (; iter != sg_dnsinfo_vec.end(); ++iter) {
        if (iter->threadid == ThreadUtil::currentthreadid()) {
            host_name = iter->host_name;
            dnsfunc = iter->dns_func;
            break;
        }
    }

    lock.unlock();

    if (NULL == dnsfunc) {
        
        struct addrinfo hints, *single, *result;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        //in iOS work fine, in Android ipv6 stack get ipv4-ip fail
        //and in ipv6 stack AI_ADDRCONFIGd will filter ipv4-ip but we ipv4-ip can use by nat64
    //    hints.ai_flags = AI_V4MAPPED|AI_ADDRCONFIG;
        int error = getaddrinfo(host_name.c_str(), NULL, &hints, &result);

        lock.lock();

        iter = sg_dnsinfo_vec.begin();
        for (; iter != sg_dnsinfo_vec.end(); ++iter) {
            if (iter->threadid == ThreadUtil::currentthreadid()) {
                break;
            }
        }

        if (error != 0) {
            xwarn2(TSF"error, error:%0, hostname:%1", error, host_name.c_str());

            if (iter != sg_dnsinfo_vec.end()) iter->status = kGetIPFail;

            sg_condition.notifyAll();
            return;
        } else {
            if (iter == sg_dnsinfo_vec.end()) {
                freeaddrinfo(result);
                return;
            }

            for (single = result; single; single = single->ai_next) {
                if (PF_INET != single->ai_family) {
                    xassert2(false);
                    continue;
                }

                sockaddr_in* addr_in = (sockaddr_in*)single->ai_addr;
                struct in_addr convertAddr;

                // In Indonesia, if there is no ipv6's ip, operators return 0.0.0.0.
                if (INADDR_ANY == addr_in->sin_addr.s_addr || INADDR_NONE == addr_in->sin_addr.s_addr) {
                    xwarn2(TSF"hehe, addr_in->sin_addr.s_addr:%0", addr_in->sin_addr.s_addr);
                    continue;
                }

                convertAddr.s_addr = addr_in->sin_addr.s_addr;
    			const char* ip = socket_address(convertAddr).ip();

                if (!socket_address(ip, 0).valid()) {
                    xerror2(TSF"ip is invalid, ip:%0", ip);
                    continue;
                }

                iter->result.push_back(ip);
            }

            if (iter->result.empty()) {
                xgroup2_define(log_group);
                std::vector<socket_address> dnssvraddrs;
                getdnssvraddrs(dnssvraddrs);
                
                xinfo2("dns server:") >> log_group;
                for (std::vector<socket_address>::iterator iter = dnssvraddrs.begin(); iter != dnssvraddrs.end(); ++iter) {
                    xinfo2(TSF"%_:%_ ", iter->ip(), iter->port()) >> log_group;
                }
            }
            
            freeaddrinfo(result);
            iter->status = kGetIPSuc;
            sg_condition.notifyAll();
        }
    } else {
        std::vector<std::string> ips = dnsfunc(host_name);
        lock.lock();
        
        iter = sg_dnsinfo_vec.begin();
        for (; iter != sg_dnsinfo_vec.end(); ++iter) {
            if (iter->threadid == ThreadUtil::currentthreadid()) {
                break;
            }
        }
        
        if (iter != sg_dnsinfo_vec.end()) {
            iter->status = ips.empty()? kGetIPFail:kGetIPSuc;
            iter->result = ips;
        }
        sg_condition.notifyAll();
    }
}

///////////////////////////////////////////////////////////////////
DNS::DNS(DNSFunc _dnsfunc):dnsfunc_(_dnsfunc) {
}

DNS::~DNS() {
    Cancel();
}

bool DNS::GetHostByName(const std::string& _host_name, std::vector<std::string>& ips, long millsec, DNSBreaker* _breaker) {
    xverbose_function();

    xassert2(!_host_name.empty());

    if (_host_name.empty()) {
        return false;
    }

    ScopedLock lock(sg_mutex);

    if (_breaker && _breaker->isbreak) return false;

    Thread thread(&__GetIP, _host_name.c_str());
    int startRet = thread.start();

    if (startRet != 0) {
        xerror2(TSF"start the thread fail");
        return false;
    }

    dnsinfo info;
    info.threadid = thread.tid();
    info.host_name = _host_name;
    info.dns_func = dnsfunc_;
    info.dns = this;
    info.status = kGetIPDoing;
    sg_dnsinfo_vec.push_back(info);

    if (_breaker) _breaker->dnsstatus = &(sg_dnsinfo_vec.back().status);

    uint64_t time_end = gettickcount() + (uint64_t)millsec;

    while (true) {
        uint64_t time_cur = gettickcount();
        uint64_t time_wait = time_end > time_cur ? time_end - time_cur : 0;

        int wait_ret = sg_condition.wait(lock, (long)time_wait);

        std::vector<dnsinfo>::iterator it = sg_dnsinfo_vec.begin();

        for (; it != sg_dnsinfo_vec.end(); ++it) {
            if (info.threadid == it->threadid)
                break;
        }

        xassert2(it != sg_dnsinfo_vec.end());
        
        if (it != sg_dnsinfo_vec.end()){

            if (ETIMEDOUT == wait_ret) {
                it->status = kGetIPTimeout;
            }

            if (kGetIPDoing == it->status) {
                continue;
            }

            if (kGetIPSuc == it->status) {
            	if (_host_name==it->host_name) {
					ips = it->result;

					if (_breaker) _breaker->dnsstatus = NULL;

					sg_dnsinfo_vec.erase(it);
					return true;
            	} else {
                    std::vector<dnsinfo>::iterator iter = sg_dnsinfo_vec.begin();
                    int i = 0;
                    for (; iter != sg_dnsinfo_vec.end(); ++iter) {
                    	xerror2(TSF"sg_info_vec[%_]:%_", i++, DNSInfoToString(*iter));
                    }
                    if (monitor_func_)
                    	monitor_func_(kDNSThreadIDError);
            		xassert2(false, TSF"_host_name:%_, it->host_name:%_", _host_name, it->host_name);
            		return false;
            	}
            }

            if (kGetIPTimeout == it->status || kGetIPCancel == it->status || kGetIPFail == it->status) {
                if (_breaker) _breaker->dnsstatus = NULL;

                xinfo2(TSF "dns get ip status:%_ host:%_, func:%_", it->status, it->host_name, it->dns_func);
                sg_dnsinfo_vec.erase(it);
                return false;
            }

            xassert2(false, "%d", it->status);

            if (_breaker) _breaker->dnsstatus = NULL;

            sg_dnsinfo_vec.erase(it);
        }
        return false;
    }

    return false;
}

void DNS::Cancel(const std::string& _host_name) {
    xverbose_function();
    ScopedLock lock(sg_mutex);

    for (unsigned int i = 0; i < sg_dnsinfo_vec.size(); ++i) {
        dnsinfo& info = sg_dnsinfo_vec.at(i);

        if (_host_name.empty() && info.dns == this) {
            info.status = kGetIPCancel;
        }

        if (info.host_name.compare(_host_name) == 0 && info.dns == this) {
            info.status = kGetIPCancel;
        }
    }

    sg_condition.notifyAll();
}

void DNS::Cancel(DNSBreaker& _breaker) {
    ScopedLock lock(sg_mutex);
    _breaker.isbreak = true;

    if (_breaker.dnsstatus) *(_breaker.dnsstatus) = kGetIPCancel;

    sg_condition.notifyAll();
}
