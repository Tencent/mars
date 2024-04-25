//
// Created by Cpan on 2022/9/7.
//

#include <jni.h>

#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/app/app.h"
#include "mars/app/app_manager.h"
#include "mars/app/jni/app_manager_callback_wrapper.h"
#include "mars/boot/context.h"
#include "mars/comm/xlogger/xlogger.h"

using namespace mars::boot;

namespace mars {
namespace app {

class JniAppManager {
 public:
    static void JniCreateAppManagerFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto app_manager_cpp = (AppManager*)j2c_cast(handle);
        auto appManagerWrapper = new jnicat::JniObjectWrapper<AppManager>(app_manager_cpp);
        appManagerWrapper->instantiate(env, instance);
    }

    static void JniCreateAppManagerFromContext(JNIEnv* env, jobject instance, jobject context) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, context);
        auto app_manager_cpp = new AppManager(context_cpp);
        auto appManagerWrapper = new jnicat::JniObjectWrapper<AppManager>(app_manager_cpp);
        appManagerWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyAppManager(JNIEnv* env, jobject instance) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        jnicat::JniObjectWrapper<Callback>::dispose(env, instance, "callbackHandle");
        app_manager_cpp->SetCallback(nullptr);
        jnicat::JniObjectWrapper<AppManager>::dispose(env, instance);
    }

    static void JniSetCallback(JNIEnv* env, jobject instance, jobject callback) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        auto appManagerJniCallback = new AppManagerJniCallback(env, callback);
        auto appManagerCallbackWrapper = new jnicat::JniObjectWrapper<AppManagerJniCallback>(appManagerJniCallback);
        appManagerCallbackWrapper->instantiate(env, instance, "callbackHandle");
        app_manager_cpp->SetCallback(appManagerJniCallback);
    }

    static void JniUpdateAppConfig(JNIEnv* env, jobject instance, jobject jconfig) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        jclass configClass = env->GetObjectClass(jconfig);
        if (configClass != NULL) {
            jfieldID enableField = env->GetFieldID(configClass, "cellularNetworkEnable", "Z");
            jfieldID connIndexField = env->GetFieldID(configClass, "cellularNetworkConnIndex", "I");
            jfieldID trafficLimitField = env->GetFieldID(configClass, "cellularNetworkTrafficLimitMB", "I");

            Config config;
            config.cellular_network_enable = env->GetBooleanField(jconfig, enableField);
            config.cellular_network_conn_index = env->GetIntField(jconfig, connIndexField);
            config.cellular_network_traffic_limit_mb = env->GetIntField(jconfig, trafficLimitField);
            app_manager_cpp->UpdateAppConfig(config);
        } else {
            xerror2(TSF "jconfig is null.");
        }
    }
};

static const JNINativeMethod kAppManagerJniMethods[] = {
    {"OnJniCreateAppManagerFromHandle", "(J)V", (void*)&mars::app::JniAppManager::JniCreateAppManagerFromHandle},
    {"OnJniCreateAppManagerFromContext",
     "(Ljava/lang/Object;)V",
     (void*)&mars::app::JniAppManager::JniCreateAppManagerFromContext},
    {"OnJniDestroyAppManager", "()V", (void*)&mars::app::JniAppManager::JniOnDestroyAppManager},
    {"OnJniSetCallback", "(Ljava/lang/Object;)V", (void*)&mars::app::JniAppManager::JniSetCallback},
    {"OnJniUpdateAppConfig", "(Ljava/lang/Object;)V", (void*)&mars::app::JniAppManager::JniUpdateAppConfig}
};

static const size_t kAppManagerJniMethodsCount = sizeof(kAppManagerJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kAppManagerInterface,
                         "com/tencent/mars/app/AppManager",
                         kAppManagerJniMethods,
                         kAppManagerJniMethodsCount)

}  // namespace app
}  // namespace mars

void ExportAppManager() {
}
