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

#ifdef ANDROID
#include <android/log.h>
#endif

using namespace mars::comm;

namespace mars {
namespace app {


AppManager::AppManager(Context* context) {
}


AppManager::AppManager(const std::string& context_id) {
    context_id_ = context_id;
}

AppManager::~AppManager() {
}



void AppManager::Init() {

}

void AppManager::UnInit() {

}

/** transition logic for app_logic */
std::map<std::string, AppManager *> AppManager::s_app_manager_map_;

std::recursive_mutex AppManager::s_mutex_;

AppManager* AppManager::CreateAppManager(const std::string& context_id) {
    if (!context_id.empty()) {
        if (s_app_manager_map_.find(context_id) != s_app_manager_map_.end()) {
            return s_app_manager_map_[context_id];
        } else {
            auto app_manager = new AppManager(context_id);
            s_app_manager_map_[context_id] = app_manager;
            return app_manager;
        }
    }
    return nullptr;
}

void AppManager::DestroyAppManager(AppManager* manager) {
    if (manager != nullptr) {
        auto* temp = dynamic_cast<AppManager*>(manager);
        auto context_id = temp->context_id_;
        if (s_app_manager_map_.find(context_id) != s_app_manager_map_.end()) {
            s_app_manager_map_.erase(context_id);
        }
        delete temp;
        manager = nullptr;
    }
}
/** transition logic for app_logic */


void AppManager::SetCallback(Callback* callback) {
    callback_ = callback;
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
    return GetUserName();
}

unsigned int AppManager::GetClientVersion() {
    xassert2(callback_ != NULL);
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

// TODO GetOsVersion no implement in original mars
double AppManager::GetOsVersion() {
    return 0;
}

void AppManager::GetProxyInfo(const std::string& _host, uint64_t _timetick) {
    xinfo_function(TSF "timetick:%_, host:%_", _timetick, _host);
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

//#if TARGET_OS_IPHONE
void AppManager::ClearProxyInfo() {
    mars::comm::ScopedLock lock(slproxymutex_);
    slproxytimetick_ = gettickcount();
    slproxycount_ = 0;
    got_proxy_ = false;
    proxy_info_.type = mars::comm::kProxyNone;
}
//#endif

}  // namespace app
}  // namespace mars
