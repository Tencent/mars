//
// Created by Cpan on 2022/9/7.
//

#include "app_manager_callback_wrapper.h"

#include "mars/comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace app {

DEFINE_FIND_CLASS(KC2Java, "com/tencent/mars/app/AppManager$CallBack")

AppManagerJniCallback::AppManagerJniCallback(JNIEnv* env, jobject callback) {
    callback_inst_ = env->NewGlobalRef(callback);
    jclass objClass = env->GetObjectClass(callback);
    if (objClass) {
        callbacks_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(objClass));
        env->DeleteLocalRef(objClass);
    }
}

AppManagerJniCallback::~AppManagerJniCallback() {
    jnienv_ptr env;
    env->DeleteGlobalRef(callback_inst_);
}

bool AppManagerJniCallback::GetProxyInfo(const std::string& _host, mars::comm::ProxyInfo& _proxy_info) {
    return false;
}

DEFINE_FIND_METHOD(KC2Java_getAppFilePath, KC2Java, "getAppFilePath", "()Ljava/lang/String;")
std::string AppManagerJniCallback::GetAppFilePath() {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jstring jstr = (jstring)JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_getAppFilePath).l;
    std::string filePath;
    if (NULL != jstr) {
        filePath.assign(ScopedJstring(env, jstr).GetChar());
        env->DeleteLocalRef(jstr);
    } else {
        xerror2(TSF "getAppFilePath error");
    }
    return filePath;
}

DEFINE_FIND_METHOD(KC2Java_getAccountInfo, KC2Java, "getAccountInfo", "()Lcom/tencent/mars/app/AppLogic$AccountInfo;")
AccountInfo AppManagerJniCallback::GetAccountInfo() {
    xverbose_function();
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    AccountInfo info;
    jobject ret_obj = JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_getAccountInfo).l;
    if (NULL == ret_obj) {
        xerror2(TSF "getAccountInfo error return null");
        return info;
    }

    jlong uin = JNU_GetField(env, ret_obj, "uin", "J").i;
    jstring username_jstr = (jstring)JNU_GetField(env, ret_obj, "userName", "Ljava/lang/String;").l;

    info.uin = (long)uin;

    if (username_jstr != NULL) {
        info.username = ScopedJstring(env, username_jstr).GetChar();
        env->DeleteLocalRef(username_jstr);
    }

    env->DeleteLocalRef(ret_obj);
    return info;
}

DEFINE_FIND_METHOD(KC2Java_getClientVersion, KC2Java, "getClientVersion", "()I")
unsigned int AppManagerJniCallback::GetClientVersion() {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    return JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_getClientVersion).i;
}

DEFINE_FIND_CLASS(KC2JavaDeviceInfo, "com/tencent/mars/app/AppLogic$DeviceInfo")
DEFINE_FIND_METHOD(KC2Java_getDeviceType, KC2Java, "getDeviceType", "()Lcom/tencent/mars/app/AppLogic$DeviceInfo;")
DeviceInfo AppManagerJniCallback::GetDeviceInfo() {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    DeviceInfo info;
    jobject ret_obj = JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_getDeviceType).l;

    if (NULL == ret_obj) {
        xerror2(TSF "getDeviceType error return null");
        return info;
    }

    jstring devicename_jstr = (jstring)JNU_GetField(env, ret_obj, "devicename", "Ljava/lang/String;").l;

    static comm::Mutex mutex;
    comm::ScopedLock lock(mutex);

    if (NULL != devicename_jstr) {
        ScopedJstring scoped_jstr(env, devicename_jstr);

        jsize len = env->GetStringUTFLength(devicename_jstr);
        info.devicename = std::string(scoped_jstr.GetChar(), len);

        env->DeleteLocalRef(devicename_jstr);
    }

    jstring devicetype_jstr = (jstring)JNU_GetField(env, ret_obj, "devicetype", "Ljava/lang/String;").l;
    if (NULL != devicetype_jstr) {
        ScopedJstring scoped_jstr(env, devicetype_jstr);

        jsize len = env->GetStringUTFLength(devicetype_jstr);
        info.devicetype = std::string(scoped_jstr.GetChar(), len);

        env->DeleteLocalRef(devicetype_jstr);
    }
    return info;
}

}  // namespace app
}  // namespace mars