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

#include "../xlogger/xlogger.h"
#include "util/comm_function.h"
#include "util/scope_jenv.h"
#include "util/scoped_jstring.h"
#include "util/var_cache.h"

#include "mars/boost/bind.hpp"
#include "mars/boost/ref.hpp"

#include "mars/comm/thread/lock.h"
#include "mars/comm/coroutine/coroutine.h"
#include "mars/comm/coroutine/coro_async.h"

#ifdef ANDROID
	int g_NetInfo = 0;    // global cache netinfo for android
	WifiInfo g_wifi_info;
	SIMInfo g_sim_info;
	APNInfo g_apn_info;
	Mutex g_net_mutex;
#endif

DEFINE_FIND_CLASS(KPlatformCommC2Java, "com/tencent/mars/comm/PlatformComm$C2Java")

#ifdef ANDROID
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_startAlarm, KPlatformCommC2Java, "startAlarm", "(II)Z")
bool startAlarm(int64_t id, int after) {
    xverbose_function();
    
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&startAlarm, id, after));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_startAlarm, (jint)id, (jint)after).z;
    xdebug2(TSF"id= %0, after= %1, ret= %2", id, after, (bool)ret);
    return (bool)ret;
}

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_stopAlarm, KPlatformCommC2Java, "stopAlarm", "(I)Z")
bool stopAlarm(int64_t  id) {
    xverbose_function();
    
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&stopAlarm, id));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    jboolean ret = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_stopAlarm, (jint)id).z;
    xdebug2(TSF"id= %0, ret= %1", id, (bool)ret);
    return (bool)ret;
}
#endif

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getProxyInfo, KPlatformCommC2Java, "getProxyInfo", "(Ljava/lang/StringBuffer;)I")
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

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getNetInfo, KPlatformCommC2Java, "getNetInfo", "()I")
int getNetInfo() {
	xverbose_function();

    if (g_NetInfo != 0)
        return g_NetInfo;
    
    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(&getNetInfo);

    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    jint netType = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getNetInfo).i;
    g_NetInfo = netType;

    xverbose2(TSF"netInfo= %0", netType);
    return (int)netType;
}

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurRadioAccessNetworkInfo, KPlatformCommC2Java, "getCurRadioAccessNetworkInfo", "()I")
bool getCurRadioAccessNetworkInfo(RadioAccessNetworkInfo& _raninfo) {
    xverbose_function();

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getCurRadioAccessNetworkInfo, boost::ref(_raninfo)));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();

    jint netType = JNU_CallStaticMethodByMethodInfo(env, KPlatformCommC2Java_getCurRadioAccessNetworkInfo).i;

    xverbose2(TSF"netInfo= %0", netType);

    switch ((int)netType) {
    case 0:
        break;

    case 1:
        _raninfo.radio_access_network = GPRS;
        break;

    case 2:
        _raninfo.radio_access_network = Edge;
        break;

    case 3:
        _raninfo.radio_access_network = UMTS;
        break;

    case 4:
        _raninfo.radio_access_network = CDMA;
        break;

    case 5:
        _raninfo.radio_access_network = CDMAEVDORev0;
        break;

    case 6:
        _raninfo.radio_access_network = CDMAEVDORevA;
        break;

    case 7:
        _raninfo.radio_access_network = CDMA1x;
        break;

    case 8:
        _raninfo.radio_access_network = HSDPA;
        break;

    case 9:
        _raninfo.radio_access_network = HSUPA;
        break;

    case 10:
        _raninfo.radio_access_network = HSPA;
        break;

    case 11:
        _raninfo.radio_access_network = IDEN;
        break;

    case 12:
        _raninfo.radio_access_network = CDMAEVDORevB;
        break;

    case 13:
        _raninfo.radio_access_network = LTE;
        break;

    case 14:
        _raninfo.radio_access_network = eHRPD;
        break;

    case 15:
        _raninfo.radio_access_network = HSPAP;
        break;

    default:
        break;
    }

    return !_raninfo.radio_access_network.empty();
}



DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurWifiInfo, KPlatformCommC2Java,
                          "getCurWifiInfo", "()Lcom/tencent/mars/comm/PlatformComm$WifiInfo;")
bool getCurWifiInfo(WifiInfo& wifiInfo) {
    xverbose_function();

    if (!g_wifi_info.ssid.empty()) {
    	wifiInfo = g_wifi_info;
    	return true;
    }

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getCurWifiInfo, boost::ref(wifiInfo)));
                                        
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

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getCurSIMInfo, KPlatformCommC2Java, "getCurSIMInfo",
                          "()Lcom/tencent/mars/comm/PlatformComm$SIMInfo;")
bool getCurSIMInfo(SIMInfo& simInfo) {
    xverbose_function();

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

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getAPNInfo, KPlatformCommC2Java, "getAPNInfo", "()Lcom/tencent/mars/comm/PlatformComm$APNInfo;")
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

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_getSignal, KPlatformCommC2Java, "getSignal", "(Z)J")

unsigned int getSignal(bool isWifi) {
    xverbose_function();

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&getSignal, isWifi));
                                        
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());

    jlong signal = JNU_CallStaticMethodByMethodInfo(scopeJEnv.GetEnv(), KPlatformCommC2Java_getSignal, isWifi).j;

    xverbose2(TSF"Signal Strength= %0, wifi:%1", signal, isWifi);
    return (unsigned int)signal;
}

DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_isNetworkConnected, KPlatformCommC2Java, "isNetworkConnected", "()Z")
bool isNetworkConnected() {
    xverbose_function();

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
DEFINE_FIND_STATIC_METHOD(KPlatformCommC2Java_wakeupLock_new, KPlatformCommC2Java, "wakeupLock_new",
                          "()Lcom/tencent/mars/comm/WakerLock;")
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

    if (coroutine::isCoroutine())
        return coroutine::MessageInvoke(boost::bind(&wakeupLock_IsLocking, _object));
    
    VarCache* cacheInstance = VarCache::Singleton();
    ScopeJEnv scopeJEnv(cacheInstance->GetJvm());
    JNIEnv* env = scopeJEnv.GetEnv();
    jboolean ret = JNU_CallMethodByName(env, (jobject)_object, "isLocking", "()Z").z;
    xdebug2(TSF"_object= %0, ret= %1", _object, (bool)ret);
    return (bool)ret;
}

#endif



