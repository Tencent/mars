//
// Created by Changpeng Pan on 2023/9/26.
//

#include "alarm_manager.h"

#include "mars/comm/xlogger/xlogger.h"

using namespace mars::boot;

namespace mars {
namespace alarm {

AlarmManager::AlarmManager(Context* context) : context_(context) {
    xverbose_function(TSF "mars2 context id %_", context_->GetContextId());
}

AlarmManager::~AlarmManager() {
}

std::string AlarmManager::GetName() {
    return typeid(AlarmManager).name();
}

void AlarmManager::SetCallback(Callback* _callback) {
    alarm_callback_ = _callback;
}

bool AlarmManager::IsAlarmCallbackSet() {
    if (alarm_callback_) {
        return true;
    }
    return false;
}

bool AlarmManager::StartAlarm(int type, int64_t id, int after) {
    if (alarm_callback_) {
        return alarm_callback_->startAlarm(type, id, after);
    } else {
        xwarn2(TSF "alarm callback is null.");
    }
    return false;
}

bool AlarmManager::StopAlarm(int64_t id) {
    if (alarm_callback_) {
        return alarm_callback_->stopAlarm(id);
    } else {
        xwarn2(TSF "alarm callback is null.");
    }
    return false;
}

}  // namespace alarm
}  // namespace mars