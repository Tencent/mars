/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * dns.cc
 *
 *  Created on: 2016-12-25
 *      Author: yerungui
 */

#include "./dns.h"

#include "boost/smart_ptr.hpp"

#include "./coro_async.h"

#include "xlogger/xlogger.h"
#include "thread/lock.h"
#include "socket/unix_socket.h"
#include "socket/socket_address.h"

#include "network/getdnssvraddrs.h"

using namespace coroutine;

static void __GetIP(const std::string& host_name, DNS::DNSFunc dnsfunc, std::vector<std::string>& _result) {
    xverbose_function();

    if (NULL == dnsfunc) {
        
        struct addrinfo hints, *single, *result;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        //in iOS work fine, in Android ipv6 stack get ipv4-ip fail
        //and in ipv6 stack AI_ADDRCONFIGd will filter ipv4-ip but we ipv4-ip can use by nat64
    //    hints.ai_flags = AI_V4MAPPED|AI_ADDRCONFIG;
        int error = getaddrinfo(host_name.c_str(), NULL, &hints, &result);

        if (error != 0) {
            xwarn2(TSF"error, error:%0, hostname:%1", error, host_name.c_str());
        } else {
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

                _result.push_back(ip);
            }

            if(_result.empty()) {
                xgroup2_define(log_group);
                std::vector<socket_address> dnssvraddrs;
                getdnssvraddrs(dnssvraddrs);
                
                xinfo2("dns server:") >> log_group;
                for (std::vector<socket_address>::iterator iter = dnssvraddrs.begin(); iter != dnssvraddrs.end(); ++iter) {
                    xinfo2(TSF"%_:%_ ", iter->ip(), iter->port()) >> log_group;
                }
            }
            
            freeaddrinfo(result);
        }
    } else {
        _result = dnsfunc(host_name);
    }
}

///////////////////////////////////////////////////////////////////

namespace coroutine {
struct dnsinfo {
    std::string     host_name;
    DNSBreaker*     breaker;
    WaitThread*      async_func;
};
}

static Mutex gs_mutex;

DNS::DNS(DNSFunc _dnsfunc):dnsfunc_(_dnsfunc) {
}

DNS::~DNS() {
    Cancel();
}

bool DNS::GetHostByName(const std::string& _host_name, std::vector<std::string>& ips, long millsec, DNSBreaker* _breaker) {

    xassert2(!_host_name.empty());
    if (_host_name.empty()) { return false; }
    if (_breaker && _breaker->isbreak) return false;

    DNSFunc dnsfunc = dnsfunc_;
    WaitThread* async_func = new WaitThread;
    
    ScopedLock lock(gs_mutex);
    dnsinfo_vec_.resize(dnsinfo_vec_.size() + 1);
    dnsinfo_vec_.back().host_name = _host_name;
    dnsinfo_vec_.back().breaker = _breaker;
    dnsinfo_vec_.back().async_func = async_func;
    lock.unlock();
    ips = (*async_func)([_host_name, dnsfunc] (){
                      std::vector<std::string> temp_result;
                      ::__GetIP(_host_name, dnsfunc, temp_result);
                      return temp_result;
                    }, millsec);
    
    lock.lock();
    dnsinfo_vec_.erase(std::remove_if(dnsinfo_vec_.begin(), dnsinfo_vec_.end(),
                   [](const dnsinfo& _v){
                       if (RunningCoroutine() == _v.async_func->wrapper()) {
                           return true;
                       }
                       return false; })
                       , dnsinfo_vec_.end());
    
    delete async_func;
    return !ips.empty();
}

void DNS::Cancel(const std::string& _host_name) {
    ScopedLock lock(gs_mutex);
    for (auto& i : dnsinfo_vec_) {
        if (_host_name.empty() || i.host_name == _host_name) {
            i.async_func->Cancel();
        }
    }
}

void DNS::Cancel(DNSBreaker& _breaker) {
    ScopedLock lock(gs_mutex);
    _breaker.isbreak = true;
    for (auto& i : dnsinfo_vec_) {
        if (i.breaker == &_breaker) { i.async_func->Cancel();}
    }
}
