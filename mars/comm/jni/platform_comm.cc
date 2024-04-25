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
 * created on : 2012-7-19
 * author : yanguoyue
 */

#include "../platform_comm.h"

#include <jni.h>
#ifdef ANDROID
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "../xlogger/xlogger.h"
#include "util/comm_function.h"
#include "util/scope_jenv.h"
#include "util/scoped_jstring.h"
#include "util/var_cache.h"

#include "mars/boost/bind.hpp"
#include "mars/boost/ref.hpp"

#include "mars/comm/thread/lock.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/coroutine/coroutine.h"
#include "mars/comm/coroutine/coro_async.h"

namespace mars {
namespace comm {

static std::function<bool(std::string&)> g_new_wifi_id_cb;
static mars::comm::Mutex wifi_id_mutex;

void SetWiFiIdCallBack(std::function<bool(std::string&)> _cb) {
    mars::comm::ScopedLock lock(wifi_id_mutex);
    g_new_wifi_id_cb = _cb;
}
void ResetWiFiIdCallBack() {
    mars::comm::ScopedLock lock(wifi_id_mutex);
    g_new_wifi_id_cb = NULL;
}


#ifdef ANDROID
    int g_NetInfo = 0;    // global cache netinfo for android
    uint64_t g_last_networkchange_tick = gettickcount();
    WifiInfo g_wifi_info;
    SIMInfo g_sim_info;
    APNInfo g_apn_info;
    Mutex g_net_mutex;
#endif


// 把 platform 编译到相应 so 中时这个函数一定要被调用。不然缓存信息清除不了
// 参看 stn_logic.cc
void OnPlatformNetworkChange() {
#ifdef ANDROID
    ScopedLock lock(g_net_mutex);
    g_NetInfo = 0;
    g_last_networkchange_tick = gettickcount();
    g_wifi_info.ssid.clear();
    g_wifi_info.bssid.clear();
    g_sim_info.isp_code.clear();
    g_sim_info.isp_name.clear();
    g_apn_info.nettype = kNoNet -1;
    g_apn_info.sub_nettype = 0;
    g_apn_info.extra_info.clear();
    lock.unlock();
#endif
}


#ifdef NATIVE_CALLBACK
    static std::weak_ptr<PlatformNativeCallback> platform_native_callback_instance;

    void SetPlatformNativeCallbackInstance(std::shared_ptr<PlatformNativeCallback> _cb) {
        platform_native_callback_instance = _cb;
    }

    #define CALL_NATIVE_CALLBACK_RETURN_FUN(fun, default_value) \
    {\
        auto cb = platform_native_callback_instance.lock();\
        if (cb) {\
            return (cb->fun);\
        }\
        xwarn2("platform native callback is null");\
        return (default_value);\
    }

    #define CALL_NATIVE_CALLBACK_VOID_FUN(fun)\
    {\
        auto cb = platform_native_callback_instance.lock();\
        if (cb) {\
            (cb->fun);\
            return;\
        }\
        xwarn2("platform native callback is null");\
        return;\
    }

#define DEFINE_FIND_EMPTY_STATIC_METHOD(methodid) \
        const static JniMethodInfo methodid = JniMethodInfo("", "", "");

#endif //NATIVE_CALLBACK


#ifndef NATIVE_CALLBACK
DEFINE_FIND_CLASS(KPlatformCommC2Java, "com/tencent/mars/comm/PlatformComm$C2Java")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java)
#endif

#ifdef ANDROID

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_startAlarm, KPlatformCommC2Java, "startAlarm", "(III)Z")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_startAlarm)
#endif
bool startAlarm(int type, int64_t id, int after) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(startAlarm(type, id, after), false);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&startAlarm, type, id, after));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_startAlarm, (jint)type, (jint)id, (jint)after).z;
    xdebug2(TSF"id= %0, after= %1, type= %2, ret= %3", id, after, type, (bool)ret);
    return (bool)ret;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_stopAlarm, KPlatformCommC2Java, "stopAlarm", "(I)Z")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_stopAlarm)
#endif
bool stopAlarm(int64_t  id) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(stopAlarm(id), false);
    #endif


    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&stopAlarm, id));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    jboolean ret = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_stopAlarm, (jint)id).z;
    xdebug2(TSF"id= %0, ret= %1", id, (bool)ret);
    return (bool)ret;
}
#endif

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getProxyInfo, KPlatformCommC2Java, "getProxyInfo", "(Ljava/lang/StringBuffer;)I")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getProxyInfo)
#endif
bool getProxyInfo(int& port, std::string& strProxy, const std::string& _host) {
    xverbose_function();

    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getProxyInfo(port, strProxy, _host), false);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getProxyInfo, boost::ref(port), boost::ref(strProxy), _host));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    // obtain the class StringBuffer
    jclass stringbufferClz = cacheInstance->GetClass(env, "java/lang/StringBuffer");

    // obtain the method id of construct method
    jmethodID constructMid = cacheInstance->GetMethodId(env, stringbufferClz, "<init>", "()V");

    // construct  the object of StringBuffer
    jobject stringbufferObj = env->NewObject(stringbufferClz, constructMid);

    jint ret = 0;
    ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getProxyInfo, stringbufferObj).i;

    if (ret <= 0) {
        xinfo2(TSF"getProxyInfo port == 0, no proxy");
        env->DeleteLocalRef(stringbufferObj);
        port = 0;
        strProxy = "";
        return false;
    }

    port = ret;

    jstring retString = NULL;
    retString = (jstring)JNU_CallMethodByName(env, stringbufferObj, "toString", "()Ljava/lang/String;").l;

    if (retString != NULL) {
        strProxy.assign(ScopedJstring(env, retString).GetChar());

        if (strProxy == "null") strProxy.clear();

        env->DeleteLocalRef(retString);
    } else {
        strProxy = "";
    }

    // free the local reference
    env->DeleteLocalRef(stringbufferObj);
    xverbose2(TSF"strProxy= %0, port= %1", strProxy.c_str(), port);
    return !strProxy.empty();
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getNetInfo, KPlatformCommC2Java, "getNetInfo", "()I")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getNetInfo)
#endif
int getNetInfo() {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getNetInfo(), -1);
    #endif

    // 防止获取的信息不准确，切换网络后延迟 1min 再使用缓存信息，1min 这个值没什么讲究主要是做个延迟。
    if (g_NetInfo != 0 && gettickcount() >= g_last_networkchange_tick + 60 * 1000) {
        return g_NetInfo;
    }
    
    // if (coroutine::isCoroutine())
    //     return coroutine::MessageInvoke(&getNetInfo);

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    jint netType = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getNetInfo).i;
    g_NetInfo = netType;

    xverbose2(TSF"netInfo= %0", netType);
    return (int)netType;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getStatisticsNetType, KPlatformCommC2Java, "getStatisticsNetType", "()I")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getStatisticsNetType)
#endif
int getNetTypeForStatistics(){
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getNetTypeForStatistics(), -1);
    #endif

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    return (int)JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getStatisticsNetType).i;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurRadioAccessNetworkInfo, KPlatformCommC2Java, "getCurRadioAccessNetworkInfo", "()I")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getCurRadioAccessNetworkInfo)
#endif
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _raninfo) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getCurRadioAccessNetworkInfo(_raninfo), false);
    #endif
    int netType = getNetTypeForStatistics(); // change interface calling to "getNetTypeForStatistics", because of Android's network info method calling restrictions

    /**
        NETTYPE_NOT_WIFI = 0;
        NETTYPE_WIFI = 1;
        NETTYPE_WAP = 2;
        NETTYPE_2G = 3;
        NETTYPE_3G = 4;
        NETTYPE_4G = 5;
        NETTYPE_UNKNOWN = 6;
        NETTYPE_5G = 7;
        NETTYPE_NON = -1;
    **/

    switch (netType) {
    case -1:
    case 0:
    case 6:
        break;
    case 1:
        _raninfo.radio_access_network = WIFI;
        break;

    case 2:
    case 3:
        _raninfo.radio_access_network = GPRS;
        break;

    case 4:
        _raninfo.radio_access_network = WCDMA;
        break;

    case 5:
        _raninfo.radio_access_network = LTE;
        break;

    case 7:
        // _raninfo.radio_access_network = G5;  // consider it to "4G" though it may be real "5G".
        _raninfo.radio_access_network = LTE;
        break;

    default:
        break;
    }
    xverbose2(TSF"netInfo= %0, %1", netType, _raninfo.radio_access_network);

    return !_raninfo.radio_access_network.empty();
}



#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurWifiInfo, KPlatformCommC2Java,
                          "getCurWifiInfo", "()Lcom/tencent/mars/comm/PlatformComm$WifiInfo;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getCurWifiInfo)
#endif
bool getCurWifiInfo(WifiInfo& wifiInfo, bool _force_refresh) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getCurWifiInfo(wifiInfo, _force_refresh), false);
    #endif

    if (!_force_refresh && !g_wifi_info.ssid.empty()) {
        wifiInfo = g_wifi_info;
        return true;
    }

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getCurWifiInfo, boost::ref(wifiInfo), _force_refresh));
                                        
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    ScopedLock lock(g_net_mutex);

    jobject retObj = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getCurWifiInfo).l;

    if (NULL == retObj) {
        xwarn2(TSF"getCurWifiInfo error return null");
        return false;
    }


    jstring ssidJstr = (jstring)JNU_GetField(env, retObj, "ssid", "Ljava/lang/String;").l;
    jstring bssidJstr = (jstring)JNU_GetField(env, retObj,  "bssid", "Ljava/lang/String;").l;

    if (NULL == ssidJstr || NULL == bssidJstr) {
        return false;
    }

    g_wifi_info.ssid = ScopedJstring(env, ssidJstr).GetChar();
    g_wifi_info.bssid = ScopedJstring(env, bssidJstr).GetChar();
    wifiInfo = g_wifi_info;
    env->DeleteLocalRef(ssidJstr);
    env->DeleteLocalRef(bssidJstr);
    env->DeleteLocalRef(retObj);

    return true;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurSIMInfo, KPlatformCommC2Java, "getCurSIMInfo",
                          "()Lcom/tencent/mars/comm/PlatformComm$SIMInfo;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getCurSIMInfo)
#endif
bool getCurSIMInfo(SIMInfo& simInfo) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getCurSIMInfo(simInfo), false);
    #endif

    if (!g_sim_info.isp_code.empty()) {
        simInfo = g_sim_info;
        return true;
    }
    
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getCurSIMInfo, boost::ref(simInfo)));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    ScopedLock lock(g_net_mutex);
    jobject retObj = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getCurSIMInfo).l;

    if (NULL == retObj) {
        xwarn2(TSF"getCurSIMInfo error return null");
        return false;
    }


    jstring ispCodeJstr = (jstring)JNU_GetField(env, retObj, "ispCode", "Ljava/lang/String;").l;
    jstring ispNameJstr = (jstring)JNU_GetField(env, retObj, "ispName", "Ljava/lang/String;").l;

    env->DeleteLocalRef(retObj);

    if (NULL == ispCodeJstr) {
        return false;
    }

    xgroup2_define(group);
    ScopedJstring icJstr(env, ispCodeJstr);
    xdebug2(TSF"ispCode:%0, ", icJstr.GetChar()) >> group;

    g_sim_info.isp_code = icJstr.GetChar();
    env->DeleteLocalRef(ispCodeJstr);

    if (NULL == ispNameJstr) { return true;    }  // isp name may NULL or empty

    ScopedJstring inJstr(env, ispNameJstr);
    xdebug2(TSF"ispName:%0", inJstr.GetChar()) >> group;

    g_sim_info.isp_name = inJstr.GetChar();
    env->DeleteLocalRef(ispNameJstr);

    simInfo = g_sim_info;

    return true;
}

#ifndef NATIVE_CALLBACK
        DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getAPNInfo, KPlatformCommC2Java, "getAPNInfo", "()Lcom/tencent/mars/comm/PlatformComm$APNInfo;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getAPNInfo)
#endif
bool getAPNInfo(APNInfo& info) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getAPNInfo(info), false);
    #endif

    if (g_apn_info.nettype >= kNoNet) {
        info = g_apn_info;
        return true;
    }

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getAPNInfo, boost::ref(info)));
                                        
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    ScopedLock lock(g_net_mutex);

    jobject retObj = JNU_CallStaticMethodByName(env, cacheInstance->GetClass(env, KPlatformCommC2Java), "getAPNInfo", "()Lcom/tencent/mars/comm/PlatformComm$APNInfo;").l;

    if (NULL == retObj) {
        xinfo2(TSF"getAPNInfo error return null");
        return false;
    }


    g_apn_info.nettype = (int)JNU_GetField(env, retObj, "netType", "I").i;
    g_apn_info.sub_nettype = (int)JNU_GetField(env, retObj, "subNetType", "I").i;

    jstring extraStr = (jstring)JNU_GetField(env, retObj, "extraInfo", "Ljava/lang/String;").l;

    env->DeleteLocalRef(retObj);

    if (extraStr != NULL) {
        ScopedJstring extraJstr(env, extraStr);

        if (extraJstr.GetChar() != NULL) {
            g_apn_info.extra_info = extraJstr.GetChar();
        }

        env->DeleteLocalRef(extraStr);
    }

    info = g_apn_info;
    return true;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getSignal, KPlatformCommC2Java, "getSignal", "(Z)J")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_getSignal)
#endif

unsigned int getSignal(bool isWifi) {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(getSignal(isWifi), 0);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getSignal, isWifi));
                                        
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());

    jlong signal = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_getSignal, isWifi).j;

    xverbose2(TSF"Signal Strength= %0, wifi:%1", signal, isWifi);
    return (unsigned int)signal;
}

#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_isNetworkConnected, KPlatformCommC2Java, "isNetworkConnected", "()Z")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_isNetworkConnected)
#endif
bool isNetworkConnected() {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(isNetworkConnected(), false);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(&isNetworkConnected);
                                        
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());

    jboolean ret = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_isNetworkConnected).z;
    xverbose2(TSF"ret= %0", (bool)ret);
    return (bool)ret;
}


bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ip) {
    return false;
}

#ifdef ANDROID
#ifndef NATIVE_CALLBACK
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_wakeupLock_new, KPlatformCommC2Java, "wakeupLock_new",
                          "()Lcom/tencent/mars/comm/WakerLock;")
#else
DEFINE_FIND_EMPTY_STATIC_METHOD(KPlatformCommC2Java_wakeupLock_new)
#endif
void* wakeupLock_new() {
    xverbose_function();
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(wakeupLock_new(), nullptr);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(&wakeupLock_new);
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jobject ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_wakeupLock_new).l;

    if (ret) {
        jobject newref = env->NewGlobalRef(ret);
        env->DeleteLocalRef(ret);
        xdebug2(TSF"newref= %0", newref);
        return newref;
    } else {
        xerror2(TSF"wakeupLock_new return null");
        return NULL;
    }
}

void  wakeupLock_delete(void* _object) {
    xverbose_function();
    xdebug2(TSF"_object= %0", _object);
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_VOID_FUN(wakeupLock_delete(_object));
    #endif

    if (NULL == _object) return;

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_delete, _object));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    env->DeleteGlobalRef((jobject)_object);
}

void  wakeupLock_Lock(void* _object) {
    xverbose_function();
    xassert2(_object);
    xdebug2(TSF"_object= %0", _object);
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_VOID_FUN(wakeupLock_Lock(_object));
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Lock, _object));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "lock", "()V");
}

void  wakeupLock_Lock_Timeout(void* _object, int64_t _timeout) {
    xverbose_function();
    xassert2(_object);
    xassert2(0 < _timeout);
    xverbose2(TSF"_object= %0, _timeout= %1", _object, _timeout);
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_VOID_FUN(wakeupLock_Lock_Timeout(_object, _timeout));
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Lock_Timeout, _object, _timeout));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "lock", "(J)V", (jlong)_timeout);
}

void  wakeupLock_Unlock(void* _object) {
    xverbose_function();
    xassert2(_object);
    xdebug2(TSF"_object= %0", _object);
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_VOID_FUN(wakeupLock_Unlock(_object));
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Unlock, _object));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "unLock", "()V");
}

bool  wakeupLock_IsLocking(void* _object) {
    xverbose_function();
    xassert2(_object);
    #ifdef NATIVE_CALLBACK
    CALL_NATIVE_CALLBACK_RETURN_FUN(wakeupLock_IsLocking(_object), false);
    #endif

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_IsLocking, _object));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallMethodByName(env, (jobject)_object, "isLocking", "()Z").z;
    xdebug2(TSF"_object= %0, ret= %1", _object, (bool)ret);
    return (bool)ret;
}

#ifdef ANDROID
std::string GetCurrentProcessName(){
    static std::string cmdline;
    if (!cmdline.empty())
        return cmdline;

    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0)
        return cmdline;

    char szcmdline[128] = {0};
    if (read(fd, &szcmdline[0], sizeof(szcmdline) - 1) > 0){
        size_t bytes = strlen(szcmdline);
        cmdline.assign(szcmdline, bytes);
    }
    close(fd);
    return cmdline;
}


//[dual-channel]
//public static boolean isCellularNetworkActive();
//descriptor: ()Z
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_isCellularNetworkActive, KPlatformCommC2Java, "isCellularNetworkActive", "()Z")
bool IsCellularNetworkActive() {
    xverbose_function();
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_isCellularNetworkActive).z;
    xdebug2(TSF"IsCellularNetworkActive ret= %0", (bool)ret);
    return (bool)ret;
}

//public static int resolveHostByCellularNetwork(java.lang.String, java.util.ArrayList<java.lang.String>);
//descriptor:  (Ljava/lang/String;)Ljava/lang/String;
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_resolveHostByCellularNetwork, KPlatformCommC2Java, "resolveHostByCellularNetwork", "(Ljava/lang/String;)Ljava/lang/String;")
bool ResolveHostByCellularNetwork(const std::string& host, std::vector<std::string>& ips) {
    xverbose_function();
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    ScopedJstring host_jstr(env, host.c_str());

    jstring ip_jstr = (jstring)JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_resolveHostByCellularNetwork, host_jstr.GetJstr()).l;

    if (nullptr == ip_jstr) {
        xwarn2("nullptr == ip_jstr");
        return false;
    }
    std::string ip(ScopedJstring(env, ip_jstr).GetChar());
    if (ip.empty()) {
        return false;
    }
    ips.emplace_back(ip);
    env->DeleteLocalRef(ip_jstr);
    return true;
}

//public static int bindSocketToCellularNetwork(int);
//descriptor: (I)Z
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_bindSocketToCellularNetwork, KPlatformCommC2Java, "bindSocketToCellularNetwork", "(I)Z")
bool BindSocketToCellularNetwork(int socket_fd) {
    xverbose_function();
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_bindSocketToCellularNetwork, (jint)socket_fd).z;
    xverbose2(TSF"ret: %0", ret);
    return ret;
}

#endif

}
}
#endif



