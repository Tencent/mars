//
// Created by Changpeng Pan on 2023/9/26.
//

#include "alarm_manager_callback_wrapper.h"

#include "mars/comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace alarm {

DEFINE_FIND_CLASS(KC2Java, "com/tencent/mars/comm/alarm/AlarmManager$CallBack")

AlarmManagerJniCallback::AlarmManagerJniCallback(JNIEnv* env, jobject callback) {
    callback_inst_ = env->NewGlobalRef(callback);
    jclass objClass = env->GetObjectClass(callback);
    if (objClass) {
        callbacks_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(objClass));
        env->DeleteLocalRef(objClass);
    }
}

AlarmManagerJniCallback::~AlarmManagerJniCallback() {
    jnienv_ptr env;
    env->DeleteGlobalRef(callback_inst_);
}

DEFINE_FIND_METHOD(KC2Java_startAlarm, KC2Java, "startAlarm", "(III)Z")
bool AlarmManagerJniCallback::startAlarm(int type, int64_t id, int after) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jboolean ret =
        JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_startAlarm, (jint)type, (jint)id, (jint)after).z;
    xinfo2(TSF "id= %0, after= %1, type= %2, ret= %3", id, after, type, (bool)ret);
    return (bool)ret;
}

DEFINE_FIND_METHOD(KC2Java_stopAlarm, KC2Java, "stopAlarm", "(I)Z")
bool AlarmManagerJniCallback::stopAlarm(int64_t id) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jboolean ret = JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_stopAlarm, (jint)id).z;
    xinfo2(TSF "id= %0, ret= %1", id, (bool)ret);
    return (bool)ret;
}
}  // namespace alarm
}  // namespace mars