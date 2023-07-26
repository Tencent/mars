//
// Created by Changpeng Pan on 2023/7/25.
//
#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/boot/context.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/config/config_mananger.h"
#include "mars/stn/stn.h"

using namespace mars::boot;

namespace mars {
namespace cfg {

class JniConfigManager {
 public:

    static void JniCreateConfigManagerFromContext(JNIEnv* env, jobject instance, jobject context) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, context);
        auto config_manager_cpp = new ConfigManager(context_cpp);
        auto configManagerWrapper = new jnicat::JniObjectWrapper<ConfigManager>(config_manager_cpp);
        configManagerWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyConfigManager(JNIEnv* env, jobject instance) {
        auto config_manager_cpp = jnicat::JniObjectWrapper<ConfigManager>::object(env, instance);
        jnicat::JniObjectWrapper<ConfigManager>::dispose(env, instance);
    }
};

static const JNINativeMethod kConfigManagerJniMethods[] = {
    {"OnJniCreateConfigManagerFromContext",
     "(Ljava/lang/Object;)V",
     (void*)&mars::cfg::JniConfigManager::JniCreateConfigManagerFromContext},
    {"OnJniDestroyConfigManager", "()V", (void*)&mars::cfg::JniConfigManager::JniOnDestroyConfigManager},

};

static const size_t kConfigManagerJniMethodsCount = sizeof(kConfigManagerJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kConfigManagerInterface,
                         "com/tencent/mars/cfg/ConfigManager",
                         kConfigManagerJniMethods,
                         kConfigManagerJniMethodsCount)

}  // namespace cfg
}  // namespace mars

void ExportConfigManager() {
}