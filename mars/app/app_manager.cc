//
// Created by Changpeng Pan on 2022/8/29.
//

#include "mars/app/app_manager.h"

#ifdef __APPLE__
#include <TargetConditionals.h>

#include "mars/comm/objc/data_protect_attr.h"
#endif

#include "mars/comm/dns/dns.h"
#include "mars/comm/xlogger/xlogger.h"
#ifdef ANDROID
#include "mars/comm/alarm.h"
#endif

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace app {

AppManager::AppManager(Context* context) : context_(context) {
    xinfo_function(TSF "mars2 context id %_", context_->GetContextId());
}

AppManager::~AppManager() {
    xinfo_function();
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

    if (!slproxymutex_.try_lock_for(std::chrono::milliseconds(500))) {
        return {};
    }

    if (slproxycount_ < 3 || 5 * 1000 > gettickspan(slproxytimetick_)) {
        std::thread([=]() {
            AppManager::GetProxyInfo(_host, slproxytimetick_);
        }).detach();
    }

    slproxymutex_.unlock();
    if (got_proxy_) {
        return proxy_info_;
    }

    return {};
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

std::string AppManager::GetAppUserName() {
    xassert2(callback_ != NULL);
    AccountInfo info = callback_->GetAccountInfo();
    return info.username;
}

std::string AppManager::GetRecentUserName() {
    xassert2(callback_ != NULL);
    if (callback_ == NULL) {
        return "";
    }
    return GetAppUserName();
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
    if (callback_) {
        return callback_->GetDeviceInfo();
    }

    DeviceInfo empty;
    empty.devicename = "DEFAULT";
    empty.devicetype = "UNKNOWN";

#ifdef ANDROID
    empty.devicetype = "ANDROID";
#elif defined(__WIN32__)
    empty.devicetype = "WINDOWS";
#elif defined(__APPLE__)
#if TARGET_OS_IPHONE
    empty.devicetype = "IPHONE";
#else
    empty.devicetype = "APPLE";
#endif
#endif

    return empty;
}

void AppManager::GetProxyInfo(const std::string& _host, uint64_t _timetick) {
    xinfo_function(TSF "time tick:%_, host:%_", _timetick, _host);
    mars::comm::ProxyInfo proxy_info;
    if (!callback_->GetProxyInfo(_host, proxy_info)) {
        std::lock_guard<std::timed_mutex> lock(slproxymutex_);
        if (_timetick != slproxytimetick_) {
            return;
        }
        ++slproxycount_;
        return;
    }

    {
        std::lock_guard<std::timed_mutex> lock(slproxymutex_);
        if (_timetick != slproxytimetick_) {
            return;
        }
        ++slproxycount_;
        proxy_info_ = proxy_info;
        if (mars::comm::kProxyNone == proxy_info_.type || !proxy_info_.ip.empty() || proxy_info_.host.empty()) {
            got_proxy_ = true;
            return;
        }
    }

    mars::comm::DNS dns;
    std::vector<std::string> ips;
    dns.GetHostByName(proxy_info_.host, ips);

    if (ips.empty()) {
        return;
    }

    {
        std::lock_guard<std::timed_mutex> lock(slproxymutex_);
        proxy_info_.ip = ips.front();
        got_proxy_ = true;
    }
}

void AppManager::__CheckCommSetting(const std::string& key) {
#ifdef ANDROID
    xinfo2(TSF "AppConfig CheckCommSetting key:%_", key);
    if (key == kKeyAlarmStartWakeupLook) {
        int wakeup = GetConfig<int>(kKeyAlarmStartWakeupLook, kAlarmStartWakeupLook);
        comm::Alarm::SetStartAlarmWakeLock(wakeup);
    } else if (key == kKeyAlarmOnWakeupLook) {
        int wakeup = GetConfig<int>(kKeyAlarmOnWakeupLook, kAlarmOnWakeupLook);
        comm::Alarm::SetOnAlarmWakeLock(wakeup);
    }
#endif
}

// #if TARGET_OS_IPHONE
void AppManager::ClearProxyInfo() {
    std::lock_guard<std::timed_mutex> lock(slproxymutex_);
    slproxytimetick_ = gettickcount();
    slproxycount_ = 0;
    got_proxy_ = false;
    proxy_info_.type = mars::comm::kProxyNone;
}
// #endif

AppManager* GetDefaultAppManager() {
    auto* context = mars::boot::Context::CreateContext("default");
    auto* manager = context->GetManager<mars::app::AppManager>();
    return manager;
}

}  // namespace app
}  // namespace mars
