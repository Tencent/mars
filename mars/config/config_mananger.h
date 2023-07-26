//
// Created by Changpeng Pan on 2023/7/25.
//

#ifndef MMNET_CONFIG_MANANGER_H
#define MMNET_CONFIG_MANANGER_H

#include <iostream>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "mars/boost/any.hpp"
#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "mars/comm/alarm.h"

namespace mars {
namespace cfg {

class ConfigManager : public mars::boot::BaseManager {
 public:
    explicit ConfigManager(mars::boot::Context* context) : context_(context) {
        xinfo_function(TSF "context:%_", context_->GetContextId());
    }
    ~ConfigManager() override {
        xinfo_function(TSF "");
    }
    std::string GetName() override {
        return typeid(ConfigManager).name();
    }

 public:
    template <typename T>
    T GetConfig(const std::string& key, const T& default_value) {
        auto it = _config.find(key);
        if (it == _config.end() && _types.at(key) != std::type_index(typeid(T))) {
            return default_value;
        }
        return boost::any_cast<T>(it->second);
    }

    template <typename T>
    void SetConfig(const std::string& key, const T& value) {
        xinfo2(TSF "SetConfig key:%_, value:%_", key, value);
        _config[key] = value;
        _types[key] = std::type_index(typeid(T));
#ifdef ANDROID
        if (key == kKeyAlarmStartWakeupLook) {
            if (std::is_convertible<T, int>::value) {
                comm::Alarm::SetStartAlarmWakeLock(static_cast<int>(value));
            }
        } else if (key == kKeyAlarmOnWakeupLook) {
            if (std::is_convertible<T, int>::value) {
                comm::Alarm::SetOnAlarmWakeLock(static_cast<int>(value));
            }
        }
#endif
    }

 private:
    mars::boot::Context* context_;
    std::unordered_map<std::string, boost::any> _config;
    std::unordered_map<std::string, std::type_index> _types;
};

}  // namespace cfg
}  // namespace mars
#endif  // MMNET_CONFIG_MANANGER_H
