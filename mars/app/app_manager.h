//
// Created by Changpeng Pan on 2022/8/29.
//

#ifndef MMNET_APP_MANAGER_H
#define MMNET_APP_MANAGER_H

#include <memory>
#include <thread>
#include <typeindex>
#include <unordered_map>

#include "mars/app/app.h"
#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "mars/comm/comm_data.h"
#include "mars/comm/time_utils.h"
#include "mars/xlog/xlogger.h"
#ifdef ANDROID
#include "mars/comm/alarm.h"
#endif
namespace mars {
namespace app {

class IAppConfigObserver {
 public:
    virtual ~IAppConfigObserver() {
    }
    virtual void OnAppConfigUpdate(const std::string& key) = 0;
};

class AppManager : public mars::boot::BaseManager {
 public:
    explicit AppManager(mars::boot::Context* context);
    ~AppManager() override;
    std::string GetName() override;

 private:
    mars::boot::Context* context_;

 public:
    void SetCallback(Callback* callback);
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host);
    std::string GetAppFilePath();
    AccountInfo GetAccountInfo();
    std::string GetAppUserName();  // WinBase.h里面定义了GetUserName这个宏
    std::string GetRecentUserName();
    unsigned int GetClientVersion();
    DeviceInfo GetDeviceInfo();

 public:
    void GetProxyInfo(const std::string& _host, uint64_t _timetick);
    //    #if TARGET_OS_IPHONE
    void ClearProxyInfo();
    //    #endif

    template <typename T>
    T GetConfig(const std::string& key, T default_value) {
        xinfo2(TSF "AppConfig GetConfig key:%_, default value:%_", key, default_value);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(key);
        auto type_it = types_.find(key);
        if (it == config_.end() || type_it == types_.end() || types_.at(key).empty()
            || types_.at(key) != std::type_index(typeid(T)).name()) {
            xwarn2(TSF "AppConfig GetConfig return default value. ");
            return default_value;
        }
        return *static_cast<T*>(it->second);
    }

    template <typename T>
    void SetConfig(const std::string& key, T value) {
        xinfo2(TSF "AppConfig SetConfig key:%_, value:%_", key, value);
        std::unique_lock<std::mutex> lock(mutex_);
        config_[key] = new T(value);
        types_[key] = std::type_index(typeid(T)).name();
        lock.unlock();
        __CheckCommSetting(key);
        __NotifyObservers(key);
    }

    void addAppConfigObserver(const std::string& key, IAppConfigObserver* observer) {
        xdebug2(TSF "add observer key:%_", key);
        std::lock_guard<std::mutex> lock(observer_mutex_);
        std::vector<IAppConfigObserver*> observers;
        if (observers_.find(key) != observers_.end()) {
            observers = observers_[key];
        }
        observers.push_back(observer);
        observers_[key] = observers;
    }

    void removeAppConfigObserver(const std::string& key, IAppConfigObserver* observer) {
        xdebug2(TSF "remove observer key:%_", key);
        std::lock_guard<std::mutex> lock(observer_mutex_);
        if (observers_.find(key) != observers_.end()) {
            std::vector<IAppConfigObserver*> observers = observers_[key];
            observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
        } else {
            xwarn2(TSF "remove observer key fail. key:%_", key);
        }
    }

 private:
    void __CheckCommSetting(const std::string& key) {
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

    void __NotifyObservers(const std::string& key) {
        xdebug2(TSF "notfiy config update key:%_", key);
        std::lock_guard<std::mutex> lock(observer_mutex_);
        if (observers_.find(key) != observers_.end()) {
            for (IAppConfigObserver* observer : observers_[key]) {
                observer->OnAppConfigUpdate(key);
            }
        }
    }

 private:
    Callback* callback_;
    mars::comm::ProxyInfo proxy_info_;
    bool got_proxy_ = false;
    std::timed_mutex slproxymutex_;
    std::thread slproxythread_;
    uint64_t slproxytimetick_ = gettickcount();
    int slproxycount_ = 0;

    std::mutex mutex_;
    std::unordered_map<std::string, void*> config_;
    std::unordered_map<std::string, std::string> types_;
    std::mutex observer_mutex_;
    std::unordered_map<std::string, std::vector<IAppConfigObserver*>> observers_;
};

}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_MANAGER_H
