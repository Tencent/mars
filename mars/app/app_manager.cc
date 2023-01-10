//
// Created by Changpeng Pan on 2022/8/29.
//

#include "mars/app/app_manager.h"

#ifdef __APPLE__
#include <TargetConditionals.h>

#include "mars/comm/objc/data_protect_attr.h"
#endif

//#include "boost/signals2.hpp"
#include "mars/comm/dns/dns.h"
#include "mars/comm/xlogger/xlogger.h"

#ifdef ANDROID
#include <android/log.h>
#endif

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace app {

AppManager::AppManager(BaseContext* context) : context_(context) {
    xdebug_function(TSF "mars2 context id %_", context_->GetContextId());
}

AppManager::~AppManager() {
}

void AppManager::Init() {
}

void AppManager::UnInit() {
}

void AppManager::SetCallback(Callback* _callback) {
    xdebug_function(TSF "mars2 SetCallback");
    sg_callback = _callback;
}

mars::comm::ProxyInfo AppManager::GetProxyInfo(const std::string& _host) {
    xdebug_function();
    xassert2(sg_callback != NULL);

#if !TARGET_OS_IPHONE
    mars::comm::ProxyInfo proxy_info;
    sg_callback->GetProxyInfo(_host, proxy_info);
    return proxy_info;
#endif

    if (sg_gotProxy) {
        return sg_proxyInfo;
    }

    ScopedLock lock(sg_slproxymutex, false);
    if (!lock.timedlock(500))
        return mars::comm::ProxyInfo();

    //TODO mars2
//    if (sg_slproxycount < 3 || 5 * 1000 > gettickspan(sg_slporxytimetick)) {
//        sg_slproxyThread.start(boost::bind(&AppManager::__GetProxyInfo, this, _host, sg_slporxytimetick));
//    }

    if (sg_gotProxy) {
        return sg_proxyInfo;
    }

    return mars::comm::ProxyInfo();
}

std::string AppManager::GetAppFilePath() {
    xdebug_function();
    xassert2(sg_callback != NULL);

    std::string path = sg_callback->GetAppFilePath();

#ifdef __APPLE__
    setAttrProtectionNone(path.c_str());
#endif

    return path;
}

AccountInfo AppManager::GetAccountInfo() {
    xdebug_function();
    xassert2(sg_callback != NULL);
    return sg_callback->GetAccountInfo();
}

std::string AppManager::GetUserName() {
    xdebug_function();
    xassert2(sg_callback != NULL);
    AccountInfo info = sg_callback->GetAccountInfo();
    return info.username;
}

std::string AppManager::GetRecentUserName() {
    xdebug_function();
    xassert2(sg_callback != NULL);
    if (sg_callback == NULL) {
        return "";
    }
    return GetUserName();
}

unsigned int AppManager::GetClientVersion() {
    xdebug_function();
    xassert2(sg_callback != NULL);
    if (sg_callback == NULL) {
        return 0;
    }
    return sg_callback->GetClientVersion();
}

DeviceInfo AppManager::GetDeviceInfo() {
    xdebug_function();
    xassert2(sg_callback != NULL);

    DeviceInfo device_info;
    if (!device_info.devicename.empty() || !device_info.devicetype.empty()) {
        return device_info;
    }

    device_info = sg_callback->GetDeviceInfo();
    return device_info;
}

double AppManager::GetOsVersion() {
    //TODO mars2
    return 0;
}

#ifdef NATIVE_CALLBACK
void AppManager::SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb) {
}
#endif

//TODO mars2 cpan
//#if !defined(ANDROID) || defined(CPP_CALL_BACK)
void AppManager::__GetProxyInfo(const std::string& _host, uint64_t _timetick) {
    xinfo_function(TSF "timetick:%_, host:%_", _timetick, _host);
    mars::comm::ProxyInfo proxy_info;
    if (!sg_callback->GetProxyInfo(_host, proxy_info)) {
        mars::comm::ScopedLock lock(sg_slproxymutex);
        if (_timetick != sg_slporxytimetick) {
            return;
        }
        ++sg_slproxycount;
        return;
    }

    mars::comm::ScopedLock lock(sg_slproxymutex);
    if (_timetick != sg_slporxytimetick) {
        return;
    }

    ++sg_slproxycount;

    sg_proxyInfo = proxy_info;

    if (mars::comm::kProxyNone == sg_proxyInfo.type || !sg_proxyInfo.ip.empty() || sg_proxyInfo.host.empty()) {
        sg_gotProxy = true;
        return;
    }

    std::string host = sg_proxyInfo.host;
    lock.unlock();

    mars::comm::DNS dns;
    std::vector<std::string> ips;
    dns.GetHostByName(host, ips);

    if (ips.empty()) {
        return;
    }

    lock.lock();
    sg_proxyInfo.ip = ips.front();
    sg_gotProxy = true;
}
//#endif

//TODO mars2 cpan
//#if TARGET_OS_IPHONE
void AppManager::__ClearProxyInfo() {
    mars::comm::ScopedLock lock(sg_slproxymutex);
    sg_slporxytimetick = gettickcount();
    sg_slproxycount = 0;
    sg_gotProxy = false;
    sg_proxyInfo.type = mars::comm::kProxyNone;
}
//#endif

}  // namespace app
}  // namespace mars
