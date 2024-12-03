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

#include "mars/comm/platform_comm.h"

#include <fcntl.h>
#include <jni.h>

#include "mars/boost/bind.hpp"
#include "mars/boost/ref.hpp"
#include "mars/comm/coroutine/coro_async.h"
#include "mars/comm/coroutine/coroutine.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/xlogger/xlogger.h"
#include "util/comm_function.h"
#include "util/scope_jenv.h"
#include "util/scoped_jstring.h"
#include "util/var_cache.h"

namespace mars {
namespace comm {
#ifndef NATIVE_CALLBACK
namespace {
NetType g_NetInfo = kNoNet;  // global cache netinfo for android
uint64_t g_last_networkchange_tick = gettickcount();
WifiInfo g_wifi_info;
SIMInfo g_sim_info;
APNInfo g_apn_info;
Mutex g_net_mutex;
}  // namespace
// 把 platform 编译到相应 so 中时这个函数一定要被调用。不然缓存信息清除不了
// 参看 stn_logic.cc
void OnPlatformNetworkChange() {
    xinfo_function();
    ScopedLock lock(g_net_mutex);
    g_NetInfo = kNoNet;
    g_last_networkchange_tick = gettickcount();
    g_wifi_info.ssid.clear();
    g_wifi_info.bssid.clear();
    g_sim_info.isp_code.clear();
    g_sim_info.isp_name.clear();
    g_apn_info.nettype = kNoNet - 1;
    g_apn_info.sub_nettype = 0;
    g_apn_info.extra_info.clear();
    lock.unlock();
}

DEFINE_FIND_CLASS(KPlatformCommC2Java, "com/tencent/mars/comm/PlatformComm$C2Java")

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, startAlarm, "(III)Z")
bool startAlarm(int type, int64_t id, int after) {
    xverbose_function();
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&startAlarm, type, id, after));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret =
        JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_startAlarm, (jint)type, (jint)id, (jint)after).z;
    xdebug2(TSF "id= %0, after= %1, type= %2, ret= %3", id, after, type, (bool)ret);
    return (bool)ret;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, stopAlarm, "(I)Z")
bool stopAlarm(int64_t id) {
    xverbose_function();
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&stopAlarm, id));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    jboolean ret = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_stopAlarm, (jint)id).z;
    xdebug2(TSF "id= %0, ret= %1", id, (bool)ret);
    return (bool)ret;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getProxyInfo, "(Ljava/lang/StringBuffer;)I")
bool getProxyInfo(int& port, std::string& strProxy, const std::string& _host) {
    xverbose_function();
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
        xinfo2(TSF "getProxyInfo port == 0, no proxy");
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

        if (strProxy == "null")
            strProxy.clear();

        env->DeleteLocalRef(retString);
    } else {
        strProxy = "";
    }

    // free the local reference
    env->DeleteLocalRef(stringbufferObj);
    xverbose2(TSF "strProxy= %0, port= %1", strProxy.c_str(), port);
    return !strProxy.empty();
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getNetInfo, "()I")
NetType getNetInfo(bool realtime /*=false*/) {
    xverbose_function();
    // 防止获取的信息不准确，切换网络后延迟 1min 再使用缓存信息，1min 这个值没什么讲究主要是做个延迟。
    if (!realtime && g_NetInfo != 0 && gettickcount() >= g_last_networkchange_tick + 60 * 1000) {
        return g_NetInfo;
    }

    // if (coroutine::isCoroutine())
    //     return coroutine::MessageInvoke(&getNetInfo);

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    jint netType = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getNetInfo).i;
    g_NetInfo = (NetType)netType;

    xdebug2(TSF "getNetInfo from JAVA %_", netType);
    return g_NetInfo;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getStatisticsNetType, "()I")
NetTypeForStatistics getNetTypeForStatistics() {
    xverbose_function();
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    return (NetTypeForStatistics)JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getStatisticsNetType).i;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getCurRadioAccessNetworkInfo, "()I")
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _raninfo) {
    xverbose_function();
    // change interface calling to "getNetTypeForStatistics"
    // because of Android's network info method calling restrictions
    NetTypeForStatistics netType = getNetTypeForStatistics();

    switch (netType) {
        case NETTYPE_NON:
        case NETTYPE_NOT_WIFI:
        case NETTYPE_UNKNOWN:
            break;
        case NETTYPE_WIFI:
            _raninfo.radio_access_network = WIFI;
            break;

        case NETTYPE_WAP:
        case NETTYPE_2G:
            _raninfo.radio_access_network = GPRS;
            break;

        case NETTYPE_3G:
            _raninfo.radio_access_network = WCDMA;
            break;

        case NETTYPE_4G:
        case NETTYPE_5G:
            _raninfo.radio_access_network = LTE;
            break;

        default:
            break;
    }
    xverbose2(TSF "netInfo= %0, %1", netType, _raninfo.radio_access_network);

    return !_raninfo.radio_access_network.empty();
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getCurWifiInfo, "()Lcom/tencent/mars/comm/PlatformComm$WifiInfo;")
bool getCurWifiInfo(WifiInfo& wifiInfo, bool _force_refresh) {
    xverbose_function();

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
        xwarn2(TSF "getCurWifiInfo error return null");
        return false;
    }

    jstring ssidJstr = (jstring)JNU_GetField(env, retObj, "ssid", "Ljava/lang/String;").l;
    jstring bssidJstr = (jstring)JNU_GetField(env, retObj, "bssid", "Ljava/lang/String;").l;

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
DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getCurSIMInfo, "()Lcom/tencent/mars/comm/PlatformComm$SIMInfo;")
bool getCurSIMInfo(SIMInfo& simInfo, bool realtime /*=false*/) {
    xverbose_function();

    if (!realtime && !g_sim_info.isp_code.empty()) {
        simInfo = g_sim_info;
        return true;
    }

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getCurSIMInfo, boost::ref(simInfo), realtime));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    ScopedLock lock(g_net_mutex);
    jobject retObj = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getCurSIMInfo).l;

    if (NULL == retObj) {
        xwarn2(TSF "getCurSIMInfo error return null");
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
    xdebug2(TSF "ispCode:%0, ", icJstr.GetChar()) >> group;

    g_sim_info.isp_code = icJstr.GetChar();
    env->DeleteLocalRef(ispCodeJstr);

    if (NULL == ispNameJstr) {
        return true;
    }  // isp name may NULL or empty

    ScopedJstring inJstr(env, ispNameJstr);
    xdebug2(TSF "ispName:%0", inJstr.GetChar()) >> group;

    g_sim_info.isp_name = inJstr.GetChar();
    env->DeleteLocalRef(ispNameJstr);

    simInfo = g_sim_info;

    return true;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getAPNInfo, "()Lcom/tencent/mars/comm/PlatformComm$APNInfo;")
bool getAPNInfo(APNInfo& info) {
    xverbose_function();

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

    jobject retObj = JNU_CallStaticMethodByName(env,
                                                cacheInstance->GetClass(env, KPlatformCommC2Java),
                                                "getAPNInfo",
                                                "()Lcom/tencent/mars/comm/PlatformComm$APNInfo;")
                         .l;

    if (NULL == retObj) {
        xinfo2(TSF "getAPNInfo error return null");
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

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, getSignal, "(Z)J")
uint32_t getSignal(bool isWifi) {
    xverbose_function();
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getSignal, isWifi));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());

    jlong signal = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_getSignal, isWifi).j;

    xverbose2(TSF "Signal Strength= %0, wifi:%1", signal, isWifi);
    return (unsigned int)signal;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, isNetworkConnected, "()Z")
bool isNetworkConnected() {
    xverbose_function();
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(&isNetworkConnected);

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());

    jboolean ret = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_isNetworkConnected).z;
    xverbose2(TSF "ret= %0", (bool)ret);
    return (bool)ret;
}

bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ip) {
    return false;
}

DEFINE_FIND_STATIC_METHOD2(KPlatformCommC2Java, wakeupLock_new, "()Lcom/tencent/mars/comm/WakerLock;")
void* wakeupLock_new() {
    xverbose_function();

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(&wakeupLock_new);

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jobject ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_wakeupLock_new).l;

    if (ret) {
        jobject newref = env->NewGlobalRef(ret);
        env->DeleteLocalRef(ret);
        xdebug2(TSF "newref= %0", newref);
        return newref;
    } else {
        xerror2(TSF "wakeupLock_new return null");
        return NULL;
    }
}

void wakeupLock_delete(void* _object) {
    xverbose_function();
    xdebug2(TSF "_object= %0", _object);
    if (NULL == _object)
        return;

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_delete, _object));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    env->DeleteGlobalRef((jobject)_object);
}

void wakeupLock_Lock(void* _object) {
    xverbose_function();
    xassert2(_object);
    xdebug2(TSF "_object= %0", _object);

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Lock, _object));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "lock", "()V");
}

void wakeupLock_Lock_Timeout(void* _object, int64_t _timeout) {
    xverbose_function();
    xassert2(_object);
    xassert2(0 < _timeout);
    xverbose2(TSF "_object= %0, _timeout= %1", _object, _timeout);

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Lock_Timeout, _object, _timeout));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "lock", "(J)V", (jlong)_timeout);
}

void wakeupLock_Unlock(void* _object) {
    xverbose_function();
    xassert2(_object);
    xdebug2(TSF "_object= %0", _object);

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_Unlock, _object));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    JNU_CallMethodByName(env, (jobject)_object, "unLock", "()V");
}

bool wakeupLock_IsLocking(void* _object) {
    xverbose_function();
    xassert2(_object);

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_IsLocking, _object));

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallMethodByName(env, (jobject)_object, "isLocking", "()Z").z;
    xdebug2(TSF "_object= %0, ret= %1", _object, (bool)ret);
    return (bool)ret;
}

#else   // #ifndef NATIVE_CALLBACK

namespace {
std::shared_ptr<NetworkInfoCallback> s_default_network_cb = std::make_shared<NetworkInfoCallback>();
std::shared_ptr<AlarmCallback> s_default_alarm_cb = std::make_shared<AlarmCallback>();
std::shared_ptr<WakeUpLockCallback> s_default_wakeup_lock_cb = std::make_shared<WakeUpLockCallback>();

std::weak_ptr<NetworkInfoCallback> s_network_cb;
std::weak_ptr<AlarmCallback> s_alarm_cb;
std::weak_ptr<WakeUpLockCallback> s_wakeup_lock_cb;

std::shared_ptr<NetworkInfoCallback> getNetworkInfoCallback() {
    auto cb = s_network_cb.lock();
    if (cb) {
        return cb;
    }
    xerror2("no network info callback");
    return s_default_network_cb;
}
std::shared_ptr<AlarmCallback> getAlarmCallback() {
    auto cb = s_alarm_cb.lock();
    if (cb) {
        return cb;
    }
    xerror2("no alarm callback");
    return s_default_alarm_cb;
}
std::shared_ptr<WakeUpLockCallback> getWakeUpLockCallback() {
    auto cb = s_wakeup_lock_cb.lock();
    if (cb) {
        return cb;
    }
    xerror2("no wakeup lock callback");
    return s_default_wakeup_lock_cb;
}
}  // namespace

void SetNetworkInfoCallback(const std::shared_ptr<NetworkInfoCallback>& _cb) {
    s_network_cb = _cb;
}
void SetAlarmCallback(const std::shared_ptr<AlarmCallback>& _cb) {
    s_alarm_cb = _cb;
}
void SetWakeUpLockCallback(const std::shared_ptr<WakeUpLockCallback>& _cb) {
    s_wakeup_lock_cb = _cb;
}

bool getProxyInfo(int& port, std::string& strProxy, const std::string& _host) {
    return getNetworkInfoCallback()->getProxyInfo(port, strProxy, _host);
}
bool getAPNInfo(APNInfo& info) {
    return getNetworkInfoCallback()->getAPNInfo(info);
}
NetType getNetInfo(bool realtime) {
    return getNetworkInfoCallback()->getNetInfo(realtime);
}
NetTypeForStatistics getNetTypeForStatistics() {
    return getNetworkInfoCallback()->getNetTypeForStatistics();
}
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _info) {
    return getNetworkInfoCallback()->getCurRadioAccessNetworkInfo(_info);
}
bool getCurWifiInfo(WifiInfo& _wifi_info, bool _force_refresh) {
    return getNetworkInfoCallback()->getCurWifiInfo(_wifi_info, _force_refresh);
}
bool getCurSIMInfo(SIMInfo& _sim_info, bool realtime) {
    return getNetworkInfoCallback()->getCurSIMInfo(_sim_info, realtime);
}
uint32_t getSignal(bool isWifi) {
    return getNetworkInfoCallback()->getSignal(isWifi);
}
bool isNetworkConnected() {
    return getNetworkInfoCallback()->isNetworkConnected();
}
bool getifaddrs_ipv4_hotspot(std::string& _ifname, std::string& _ifip) {
    return getNetworkInfoCallback()->getIfAddrsIpv4HotSpot(_ifname, _ifip);
}

bool startAlarm(int type, int64_t id, int after) {
    return getAlarmCallback()->startAlarm(type, id, after);
}
bool stopAlarm(int64_t id) {
    return getAlarmCallback()->stopAlarm(id);
}

void* wakeupLock_new() {
    return getWakeUpLockCallback()->wakeupLock_new();
}
void wakeupLock_delete(void* _object) {
    getWakeUpLockCallback()->wakeupLock_delete(_object);
}
void wakeupLock_Lock(void* _object) {
    getWakeUpLockCallback()->wakeupLock_Lock(_object);
}
void wakeupLock_Lock_Timeout(void* _object, int64_t _timeout) {
    getWakeUpLockCallback()->wakeupLock_Lock_Timeout(_object, _timeout);
}
void wakeupLock_Unlock(void* _object) {
    getWakeUpLockCallback()->wakeupLock_Unlock(_object);
}
bool wakeupLock_IsLocking(void* _object) {
    return getWakeUpLockCallback()->wakeupLock_IsLocking(_object);
}
#endif  // #ifndef NATIVE_CALLBACK

std::string GetCurrentProcessName() {
    static std::string cmdline;
    if (!cmdline.empty())
        return cmdline;

    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0)
        return cmdline;

    char szcmdline[128] = {0};
    if (read(fd, &szcmdline[0], sizeof(szcmdline) - 1) > 0) {
        size_t bytes = strlen(szcmdline);
        cmdline.assign(szcmdline, bytes);
    }
    close(fd);
    return cmdline;
}

}  // namespace comm
}  // namespace mars
