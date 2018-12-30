#if (!UWP && !WIN32)
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#define USE_WIN32_CODE
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <vector>
#include "getaddrinfo_with_timeout.h"

#include "comm/thread/mutex.h"
#include "comm/thread/lock.h"
#include "comm/thread/thread.h"
#include "comm/thread/condition.h"
#include "comm/xlogger/xlogger.h"
#include "comm/time_utils.h"


enum {
    kGetADDRNotBegin,
    kGetADDRDoing,
    kGetADDRTimeout,
    kGetADDRSuc,
    kGetADDRFail,
};

enum {
    kRetCodeInternalStateError =  -888,
    kRetCodeParamNotMatch,
    kRetCodeDnsItemNotFound,
    kRetCodeGetADDRTimeout
};
struct DnsItem {
    thread_tid      threadid;
    //parameter
    const char *node;
    const char *service;
    const struct addrinfo *hints;
    struct addrinfo **res;
    //~parameter
    
    int error_code;
    int status;
    
    DnsItem(): threadid(0), node(NULL), service(NULL), hints(NULL), res(NULL), error_code(0), status(kGetADDRNotBegin) {}
    
    bool EqualParameter(const DnsItem& _item) {
        return _item.node == node
            && _item.service == service
            && _item.hints == hints
            && _item.res == res;
    }
    std::string ToString() const {
        XMessage xmsg;
        xmsg(TSF"node:%_, service:%_, hints:%_, res:%_, tid:%_, error_code:%_, status:%_",
             NULL==node?"NULL":node
            ,NULL==service?"NULL":service
            ,NULL==hints?(void*)0:hints
            ,NULL==res?(void*)0:res
            ,threadid
            ,error_code
            ,status);
        return xmsg.String();
    }
};

static std::vector<DnsItem> sg_dnsitem_vec;
static Condition sg_condition;
static Mutex sg_mutex;


static void __WorkerFunc() {
    xverbose_function();
    
    
    //parameter
    std::string worker_node;
    std::string worker_service;
    struct addrinfo worker_hints;
    struct addrinfo *worker_res0= NULL;
	memset(&worker_hints, 0, sizeof(worker_hints));

    ScopedLock lock(sg_mutex);
    std::vector<DnsItem>::iterator iter = sg_dnsitem_vec.begin();
    
    for (; iter != sg_dnsitem_vec.end(); ++iter) {
        if (iter->threadid == ThreadUtil::currentthreadid()) {
            worker_node = iter->node ? iter->node : "";
            worker_service =  iter->service ? iter->service : "";
            if(iter->hints) worker_hints = *iter->hints;
            iter->status = kGetADDRDoing;
            break;
        }
    }
    if (iter == sg_dnsitem_vec.end()) {
        xerror2(TSF"timeout before sys getaddrinfo");
        return;
    }
    lock.unlock();
    
    int error = getaddrinfo(worker_node.c_str(), worker_service.c_str(), &worker_hints, &worker_res0);
    xinfo2(TSF"sys getaddrinfo error:%_, node:%_, service:%_", error, worker_node, worker_service);
    
    lock.lock();
    
    iter = sg_dnsitem_vec.begin();
    for (; iter != sg_dnsitem_vec.end(); ++iter) {
        if (iter->threadid == ThreadUtil::currentthreadid()) {
            break;
        }
    }
    
    if (error != 0) {
        if (iter != sg_dnsitem_vec.end()) {
            iter->error_code = error;
            iter->status = kGetADDRFail;
            xassert2(NULL!=iter->res);
            *(iter->res) = worker_res0;
        } else {
            if (worker_res0!=NULL) {
                xinfo2(TSF"getaddrinfo fail and timeout. free worker_res0 @%_", worker_res0);
                freeaddrinfo(worker_res0);
            }
            xinfo2(TSF"getaddrinfo fail and timeout. worker_node:%_", worker_node);
        }
        sg_condition.notifyAll();
    } else {
        if (iter != sg_dnsitem_vec.end()) {
            if (iter->status==kGetADDRDoing) {
                iter->status = kGetADDRSuc;
                xassert2(NULL!=iter->res);
                *(iter->res) = worker_res0;
            } else {
                if (worker_res0!=NULL) {
                    xinfo2(TSF"getaddrinfo end but timeout. free worker_res0 @%_", worker_res0);
                    freeaddrinfo(worker_res0);
                }
                xinfo2(TSF"getaddrinfo end but timeout. worker_node:%_", worker_node);
            }
        } else {
            if (worker_res0!=NULL) {
                xinfo2(TSF"getaddrinfo end but timeout. free worker_res0 @%_", worker_res0);
                freeaddrinfo(worker_res0);
            }
        }
        
        sg_condition.notifyAll();
    }
}




int getaddrinfo_with_timeout(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res, bool& _is_timeout, unsigned long long _timeout_msec) {
    xverbose_function();
    //Check param
    
    ScopedLock lock(sg_mutex);
    
    
    Thread thread(&__WorkerFunc, node);
    int start_ret = thread.start();
    
    if (start_ret != 0) {
        xerror2(TSF"start the thread fail, host:%_", node);
        return kRetCodeInternalStateError;
    }
    
    DnsItem dns_item;
    dns_item.threadid = thread.tid();
    dns_item.node = node;
    dns_item.service = service;
    dns_item.hints = hints;
    dns_item.res = res;
    dns_item.status = kGetADDRNotBegin;
    sg_dnsitem_vec.push_back(dns_item);
    
    
    uint64_t time_end = gettickcount() + (uint64_t)_timeout_msec;
    
    while (true) {
        uint64_t time_cur = gettickcount();
        uint64_t time_wait = time_end > time_cur ? time_end - time_cur : 0;
        
        int wait_ret = sg_condition.wait(lock, (long)time_wait);
        
        std::vector<DnsItem>::iterator it = sg_dnsitem_vec.begin();
        
        for (; it != sg_dnsitem_vec.end(); ++it) {
            if (dns_item.threadid == it->threadid)
                break;
        }
        
        xassert2(it != sg_dnsitem_vec.end());
        
        if (it != sg_dnsitem_vec.end()){
            
            if (ETIMEDOUT == wait_ret) {
                it->status = kGetADDRTimeout;
            }
            
            if (kGetADDRNotBegin== it->status || kGetADDRDoing == it->status) {
                continue;
            }
            
            if (kGetADDRSuc == it->status) {
                if (it->EqualParameter(dns_item)) {
                    sg_dnsitem_vec.erase(it);
                    return 0;
                } else {
                    std::vector<DnsItem>::iterator iter = sg_dnsitem_vec.begin();
                    int i = 0;
                    for (; iter != sg_dnsitem_vec.end(); ++iter) {
                        xerror2(TSF"sg_dnsitem_vec[%_]:%_", i++, iter->ToString());
                    }
                    xassert2(false, TSF"dns_item:%_", dns_item.ToString());
                    return kRetCodeParamNotMatch;
                }
            }
            
            if (kGetADDRTimeout == it->status ) {
                xinfo2(TSF "dns get ip status:kGetADDRTimeout item:%_", it->ToString());
                sg_dnsitem_vec.erase(it);
                _is_timeout = true;
                return kRetCodeGetADDRTimeout;
            } else if (kGetADDRFail == it->status) {
                xinfo2(TSF "dns get ip status:kGetADDRFail item:%_", it->ToString());
                int ret_code = it->error_code;
                sg_dnsitem_vec.erase(it);
                return ret_code;
            }
            
            xassert2(false, TSF"%_", it->status);
            
            
            sg_dnsitem_vec.erase(it);
        }
        return kRetCodeDnsItemNotFound;
    }
    
    return kRetCodeInternalStateError;
}
