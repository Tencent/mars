//
// Created by Changpeng Pan on 2022/9/14.
//
#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/stn/stn.h"
#include "mars/comm/base_stn_manager.h"
#include "mars/stn/stn_manager.h"
#include "stn_manager_callback_wrapper.h"

namespace mars {
namespace stn {

class JniStnManager {

 public:
    static void JniCreateStnManager(JNIEnv* env, jobject instance, jstring contextId) {
        auto stn_manager_cpp = StnManager::CreateStnManager(j2c_cast(contextId));
        auto stnManagerWrapper = new jnicat::JniObjectWrapper<StnManager>(stn_manager_cpp);
        stnManagerWrapper->instantiate(env, instance);
    }

    static void JniCreateStnManagerFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto stn_manager_cpp = (StnManager*)j2c_cast(handle);
        auto stnManagerWrapper = new jnicat::JniObjectWrapper<StnManager>(stn_manager_cpp);
        stnManagerWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyStnManager(JNIEnv* env, jobject instance) {
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        jnicat::JniObjectWrapper<Callback>::dispose(env, instance, "callbackHandle");
        stn_manager_cpp->SetCallback(nullptr);
        jnicat::JniObjectWrapper<StnManager>::dispose(env, instance);
    }

    static void JniSetCallback(JNIEnv* env, jobject instance, jobject callback) {
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        auto stnManagerJniCallback = new StnManagerJniCallback(env, callback);
        auto stnManagerCallbackWrapper = new jnicat::JniObjectWrapper<StnManagerJniCallback>(stnManagerJniCallback);
        stnManagerCallbackWrapper->instantiate(env, instance, "callbackHandle");
        stn_manager_cpp->SetCallback(stnManagerJniCallback);
    }
};


static const JNINativeMethod kStnManagerJniMethods[] = {
    {"OnJniCreateStnManager", "(Ljava/lang/String;)V", (void*)&mars::stn::JniStnManager::JniCreateStnManager},
    {"OnJniCreateStnManagerFromHandle", "(J)V", (void*)&mars::stn::JniStnManager::JniCreateStnManagerFromHandle},
    {"OnJniDestroyStnManager", "()V", (void*)&mars::stn::JniStnManager::JniOnDestroyStnManager},
    {"OnJniSetCallback", "(Ljava/lang/Object;)V", (void*)&mars::stn::JniStnManager::JniSetCallback},
};

static const size_t kStnManagerJniMethodsCount = sizeof(kStnManagerJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kStnManagerInterface, "com/tencent/mars/stn/StnManager", kStnManagerJniMethods, kStnManagerJniMethodsCount)

}  // namespace stn
}  // namespace mars

void ExportStnManager() {
}
