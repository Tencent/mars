//
// Created by Cpan on 2022/9/7.
//

#include "app_manager_callback_wrapper.h"

#include "comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
//#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace app {

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

JNICAT_DEFINE_METHOD(kAppManagerJniCallback_GetAppFilePath, "com/tencent/mars/app/AppManager$Callback", "getAppFilePath", "()Ljava/lang/String;")

std::string AppManagerJniCallback::GetAppFilePath() {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jstring, callback_inst_, kAppManagerJniCallback_GetAppFilePath));
}

JNICAT_DEFINE_CLASS("com/tencent/mars/app/AppLogic$AccountInfo")
JNICAT_DEFINE_METHOD(kAppManagerJniCallback_GetAccountInfo, "com/tencent/mars/app/AppManager$Callback", "getAccountInfo", "()Lcom/tencent/mars/app/AppLogic$AccountInfo;")
AccountInfo AppManagerJniCallback::GetAccountInfo() {
    jnienv_ptr env;
    AccountInfo info;
    jobject ret_obj = c2j_call(jobject, callback_inst_, kAppManagerJniCallback_GetAccountInfo);
    if (NULL == ret_obj) {
        //xerror2(TSF "getAccountInfo error return null");
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

JNICAT_DEFINE_METHOD(kAppManagerJniCallback_GetClientVersion, "com/tencent/mars/app/AppManager$Callback", "getClientVersion", "()I")
unsigned int AppManagerJniCallback::GetClientVersion() {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jint, callback_inst_, kAppManagerJniCallback_GetClientVersion));
}

JNICAT_DEFINE_CLASS("com/tencent/mars/app/AppLogic$DeviceInfo")
JNICAT_DEFINE_METHOD(kAppManagerJniCallback_GetDeviceInfo, "com/tencent/mars/app/AppManager$Callback", "getDeviceInfo", "()Lcom/tencent/mars/app/AppLogic$DeviceInfo;")
DeviceInfo AppManagerJniCallback::GetDeviceInfo() {
    jnienv_ptr env;
    DeviceInfo info;
    jobject ret_obj = c2j_call(jobject, callback_inst_, kAppManagerJniCallback_GetClientVersion);

    if (NULL == ret_obj) {
        //xerror2(TSF "GetDeviceInfo error return null");
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