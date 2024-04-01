// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/**
 * created on : 2012-10-19
 * author : yanguoyue
 */

#ifndef USE_CPP_CALLBACK
#include <jni.h>

#include <sstream>

#include "mars/app/app.h"
#include "mars/comm/autobuffer.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"

#ifndef NATIVE_CALLBACK
DEFINE_FIND_CLASS(KC2Java, "com/tencent/mars/app/AppLogic")
#endif  // NATIVE_CALLBACK

namespace mars {
namespace app {

#ifdef NATIVE_CALLBACK
static std::weak_ptr<AppLogicNativeCallback> applogic_native_callback_instance;

void SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb) {
    applogic_native_callback_instance = _cb;
}

#define CALL_NATIVE_CALLBACK_RETURN_FUN(fun, default_value) \
    {                                                       \
        auto cb = applogic_native_callback_instance.lock(); \
        if (cb) {                                           \
            return (cb->fun);                               \
        }                                                   \
        xwarn2("applogic native callback is null");         \
        return (default_value);                             \
    }

#define DEFINE_FIND_EMPTY_STATIC_METHOD(methodid) const static JniMethodInfo methodid = JniMethodInfo("", "", "");

#endif  // NATIVE_CALLBACK

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KC2Java_getAppFilePath, KC2Java, "getAppFilePath", "()Ljava/lang/String;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KC2Java_getAppFilePath)
#endif
std::string GetAppFilePath() {
    xverbose_function();

#ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(GetAppFilePath(), std::string(""));
#endif

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jstring path = (jstring)JNU_CallStaticMethodByMethodInfo(env, KC2Java_getAppFilePath).l;
    if (NULL == path) {
        xerror2(TSF "getAppFilePath error return null");
        return "";
    }

    std::string app_path = ScopedJstring(env, path).GetChar();
    env->DeleteLocalRef(path);

    return app_path;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KC2Java_getAccountInfo,
                          KC2Java,
                          "getAccountInfo",
                          "()Lcom/tencent/mars/app/AppLogic$AccountInfo;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KC2Java_getAccountInfo)
#endif
AccountInfo GetAccountInfo() {
    xverbose_function();
#ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(GetAccountInfo(), AccountInfo());
#else

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    AccountInfo info;
    jobject ret_obj = JNU_CallStaticMethodByMethodInfo(env, KC2Java_getAccountInfo).l;
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
#endif
}

std::string GetAppUserName() {
    xverbose_function();

#ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(GetAppUserName(), std::string(""));
#endif
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jobject ret_obj = JNU_CallStaticMethodByMethodInfo(env, KC2Java_getAccountInfo).l;
    if (NULL == ret_obj) {
        return std::string();
    }

    jstring username_jstr = (jstring)JNU_GetField(env, ret_obj, "userName", "Ljava/lang/String;").l;

    env->DeleteLocalRef(ret_obj);

    if (username_jstr != NULL) {
        const char* name = env->GetStringUTFChars(username_jstr, NULL);
        std::string user_name(name);
        env->ReleaseStringUTFChars(username_jstr, name);
        env->DeleteLocalRef(username_jstr);
        return user_name;
    } else {
        return std::string();
    }
}

std::string GetRecentUserName() {
    return GetAppUserName();
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KC2Java_getClientVersion, KC2Java, "getClientVersion", "()I")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KC2Java_getClientVersion)
#endif
unsigned int GetClientVersion() {
#ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(GetClientVersion(), 0);
#endif
    static unsigned int s_version = 0;
    if (0 != s_version) {
        return s_version;
    }

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();
    s_version = (unsigned int)JNU_CallStaticMethodByMethodInfo(env, KC2Java_getClientVersion).i;

    return s_version;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KC2Java_getDeviceType,
                          KC2Java,
                          "getDeviceType",
                          "()Lcom/tencent/mars/app/AppLogic$DeviceInfo;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KC2Java_getDeviceType)
#endif
DeviceInfo GetDeviceInfo() {
    xverbose_function();
#ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(GetDeviceInfo(), DeviceInfo());
#endif

    static DeviceInfo s_info;
    if (!s_info.devicename.empty() || !s_info.devicetype.empty()) {
        return s_info;
    }

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jobject ret_obj = JNU_CallStaticMethodByMethodInfo(env, KC2Java_getDeviceType).l;
    if (NULL == ret_obj) {
        xerror2(TSF "GetDeviceInfo error return null");
        return s_info;
    }

    jstring devicename_jstr = (jstring)JNU_GetField(env, ret_obj, "devicename", "Ljava/lang/String;").l;

    static comm::Mutex mutex;
    comm::ScopedLock lock(mutex);

    if (NULL != devicename_jstr) {
        ScopedJstring scoped_jstr(env, devicename_jstr);

        jsize len = env->GetStringUTFLength(devicename_jstr);
        s_info.devicename = std::string(scoped_jstr.GetChar(), len);

        env->DeleteLocalRef(devicename_jstr);
    }

    jstring devicetype_jstr = (jstring)JNU_GetField(env, ret_obj, "devicetype", "Ljava/lang/String;").l;
    if (NULL != devicetype_jstr) {
        ScopedJstring scoped_jstr(env, devicetype_jstr);

        jsize len = env->GetStringUTFLength(devicetype_jstr);
        s_info.devicetype = std::string(scoped_jstr.GetChar(), len);

        env->DeleteLocalRef(devicetype_jstr);
    }

    return s_info;
}

mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) {
    return mars::comm::ProxyInfo();
}

}  // namespace app
}  // namespace mars
#endif
