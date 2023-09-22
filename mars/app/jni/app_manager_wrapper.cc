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
#include "mars/comm/jni/util/scoped_jstring.h"

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

    static void JniSetConfigBooleanValue(JNIEnv* env, jobject instance, jstring jkey, jboolean jvalue) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        std::string key = (NULL == jkey ? "" : ScopedJstring(env, jkey).GetChar());
        bool value = (jvalue == JNI_TRUE);
        app_manager_cpp->SetConfig(key, value);
    }

    static void JniSetConfigIntValue(JNIEnv* env, jobject instance, jstring jkey, jint jvalue) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        std::string key = (NULL == jkey ? "" : ScopedJstring(env, jkey).GetChar());
        int value = static_cast<int>(jvalue);
        app_manager_cpp->SetConfig(key, value);
    }

    static void JniSetConfigStringValue(JNIEnv* env, jobject instance, jstring jkey, jstring jvalue) {
        auto app_manager_cpp = jnicat::JniObjectWrapper<AppManager>::object(env, instance);
        std::string key = (NULL == jkey ? "" : ScopedJstring(env, jkey).GetChar());
        std::string value = (NULL == jvalue ? "" : ScopedJstring(env, jvalue).GetChar());
        app_manager_cpp->SetConfig(key, value);
    }
};

static const JNINativeMethod kAppManagerJniMethods[] = {
    {"OnJniCreateAppManagerFromHandle", "(J)V", (void*)&mars::app::JniAppManager::JniCreateAppManagerFromHandle},
    {"OnJniCreateAppManagerFromContext",
     "(Ljava/lang/Object;)V",
     (void*)&mars::app::JniAppManager::JniCreateAppManagerFromContext},
    {"OnJniDestroyAppManager", "()V", (void*)&mars::app::JniAppManager::JniOnDestroyAppManager},
    {"OnJniSetCallback", "(Ljava/lang/Object;)V", (void*)&mars::app::JniAppManager::JniSetCallback},
    {"OnJniSetConfigBooleanValue",
     "(Ljava/lang/String;Z)V",
     (void*)&mars::app::JniAppManager::JniSetConfigBooleanValue},
    {"OnJniSetConfigIntValue", "(Ljava/lang/String;I)V", (void*)&mars::app::JniAppManager::JniSetConfigIntValue},
    {"OnJniSetConfigStringValue",
     "(Ljava/lang/String;Ljava/lang/String;)V",
     (void*)&mars::app::JniAppManager::JniSetConfigStringValue},
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
