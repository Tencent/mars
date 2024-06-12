//
// Created by Changpeng Pan on 2023/9/26.
//

#ifndef MMNET_ALARM_MANAGER_H
#define MMNET_ALARM_MANAGER_H

#include "comm/alarm.h"

namespace mars {
namespace alarm {

class Callback {
 public:
    virtual bool startAlarm(int type, int64_t id, int after) {
        return false;
    }
    virtual bool stopAlarm(int64_t id) {
        return false;
    }
};

class AlarmManager : public mars::boot::BaseManager {
 public:
    explicit AlarmManager(mars::boot::Context* context);
    ~AlarmManager() override;
    std::string GetName() override;

 public:
    void SetCallback(Callback* _callback);
    bool IsAlarmCallbackSet();
    bool StartAlarm(int type, int64_t id, int after);
    bool StopAlarm(int64_t id);

 private:
    mars::boot::Context* context_;
    Callback* alarm_callback_;
};
}  // namespace alarm
}  // namespace mars

#endif  // MMNET_ALARM_MANAGER_H
