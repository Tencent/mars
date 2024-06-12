//
// Created by Changpeng Pan on 2023/9/26.
//

#ifndef MMNET_ALARM_MANAGER_CALLBACK_WRAPPER_H
#define MMNET_ALARM_MANAGER_CALLBACK_WRAPPER_H

#include <jni.h>

#include "mars/alarm/alarm_manager.h"

namespace mars {
namespace alarm {
class AlarmManagerJniCallback : public Callback {
 public:
    AlarmManagerJniCallback(JNIEnv* env, jobject callback);
    virtual ~AlarmManagerJniCallback();

 private:
    jobject callback_inst_;
    jclass callbacks_class_;

 public:
    virtual bool startAlarm(int type, int64_t id, int after) override;
    virtual bool stopAlarm(int64_t id) override;
};
}  // namespace alarm
}  // namespace mars

#endif  // MMNET_ALARM_MANAGER_CALLBACK_WRAPPER_H
