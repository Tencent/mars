//
// Created by Changpeng Pan on 2022/8/29.
//

#include "mars/app/app_manager.h"

#ifdef __APPLE__
#include <TargetConditionals.h>

#include "mars/comm/objc/data_protect_attr.h"
#endif

#include "mars/baseevent/baseprjevent.h"
#include "mars/comm/dns/dns.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/app/src/app_config.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace app {

AppManager::AppManager(Context* context) : context_(context), app_config_(new AppConfig()) {
    xverbose_function(TSF "mars2 context id %_", context_->GetContextId());
}

AppManager::~AppManager() {
}

std::string AppManager::GetName() {
    return typeid(AppManager).name();
}

void AppManager::SetCallback(Callback* _callback) {
    xinfo_function();
    callback_ = _callback;
}

mars::comm::ProxyInfo AppManager::GetProxyInfo(const std::string& _host) {
    xassert2(callback_ != NULL);

#if !TARGET_OS_IPHONE
    mars::comm::ProxyInfo proxy_info;
    callback_->GetProxyInfo(_host, proxy_info);
    return proxy_info;
#endif

    if (got_proxy_) {
        return proxy_info_;
    }

    ScopedLock lock(slproxymutex_, false);
    if (!lock.timedlock(500))
        return mars::comm::ProxyInfo();

    if (slproxycount_ < 3 || 5 * 1000 > gettickspan(slproxytimetick_)) {
        slproxythread_.start(boost::bind(&AppManager::GetProxyInfo, this, _host, slproxytimetick_));
    }

    if (got_proxy_) {
        return proxy_info_;
    }

    return mars::comm::ProxyInfo();
}

std::string AppManager::GetAppFilePath() {
    xassert2(callback_ != NULL);

    std::string path = callback_->GetAppFilePath();

#ifdef __APPLE__
    setAttrProtectionNone(path.c_str());
#endif

    return path;
}

AccountInfo AppManager::GetAccountInfo() {
    xassert2(callback_ != NULL);
    return callback_->GetAccountInfo();
}

std::string AppManager::GetUserName() {
    xassert2(callback_ != NULL);
    AccountInfo info = callback_->GetAccountInfo();
    return info.username;
}

std::string AppManager::GetRecentUserName() {
    xassert2(callback_ != NULL);
    if (callback_ == NULL) {
        return "";
    }
    return GetUserName();
}

unsigned int AppManager::GetClientVersion() {
    xassert2(callback_ != NULL);
    if (callback_ == NULL) {
        return 0;
    }
    return callback_->GetClientVersion();
}

DeviceInfo AppManager::GetDeviceInfo() {
    xassert2(callback_ != NULL);

    DeviceInfo device_info;
    if (!device_info.devicename.empty() || !device_info.devicetype.empty()) {
        return device_info;
    }

    device_info = callback_->GetDeviceInfo();
    return device_info;
}

void AppManager::GetProxyInfo(const std::string& _host, uint64_t _timetick) {
    xinfo_function(TSF "time tick:%_, host:%_", _timetick, _host);
    mars::comm::ProxyInfo proxy_info;
    if (!callback_->GetProxyInfo(_host, proxy_info)) {
        mars::comm::ScopedLock lock(slproxymutex_);
        if (_timetick != slproxytimetick_) {
            return;
        }
        ++slproxycount_;
        return;
    }

    mars::comm::ScopedLock lock(slproxymutex_);
    if (_timetick != slproxytimetick_) {
        return;
    }

    ++slproxycount_;

    proxy_info_ = proxy_info;

    if (mars::comm::kProxyNone == proxy_info_.type || !proxy_info_.ip.empty() || proxy_info_.host.empty()) {
        got_proxy_ = true;
        return;
    }

    std::string host = proxy_info_.host;
    lock.unlock();

    mars::comm::DNS dns;
    std::vector<std::string> ips;
    dns.GetHostByName(host, ips);

    if (ips.empty()) {
        return;
    }

    lock.lock();
    proxy_info_.ip = ips.front();
    got_proxy_ = true;
}

// #if TARGET_OS_IPHONE
void AppManager::ClearProxyInfo() {
    mars::comm::ScopedLock lock(slproxymutex_);
    slproxytimetick_ = gettickcount();
    slproxycount_ = 0;
    got_proxy_ = false;
    proxy_info_.type = mars::comm::kProxyNone;
}
// #endif

void AppManager::UpdateAppConfig(Config _app_config) {
    app_config_->UpdateConfig(_app_config);
}

AppConfig* AppManager::GetAppConfig() {
    return app_config_;
}

}  // namespace app
}  // namespace mars
