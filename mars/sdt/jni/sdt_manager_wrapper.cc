//
// Created by Cpan on 2022/9/20.
//
#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/baseevent/baseevent.h"
#include "mars/sdt/sdt.h"
#include "mars/sdt/sdt_manager.h"
#include "sdt_manager_callback_wrapper.h"

using namespace mars::boot;

namespace mars {
namespace sdt {

class JniSdtManager {
 public:
    static void JniCreateSdtManagerFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto sdt_manager_cpp = (SdtManager*)j2c_cast(handle);
        auto sdtManagerWrapper = new jnicat::JniObjectWrapper<SdtManager>(sdt_manager_cpp);
        sdtManagerWrapper->instantiate(env, instance);
    }

    static void JniCreateSdtManagerFromContext(JNIEnv* env, jobject instance, jobject context) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, context);
        auto sdt_manager_cpp = new SdtManager(context_cpp);
        auto sdtManagerWrapper = new jnicat::JniObjectWrapper<SdtManager>(sdt_manager_cpp);
        sdtManagerWrapper->instantiate(env, instance);
    }

    static void JniOnDestroySdtManager(JNIEnv* env, jobject instance) {
        auto sdt_manager_cpp = jnicat::JniObjectWrapper<SdtManager>::object(env, instance);
        jnicat::JniObjectWrapper<Callback>::dispose(env, instance, "callbackHandle");
        sdt_manager_cpp->SetCallBack(nullptr);
        jnicat::JniObjectWrapper<SdtManager>::dispose(env, instance);
    }

    static void JniSetCallback(JNIEnv* env, jobject instance, jobject callback) {
        auto sdt_manager_cpp = jnicat::JniObjectWrapper<SdtManager>::object(env, instance);
        auto sdtManagerJniCallback = new SdtManagerJniCallback(env, callback);
        auto sdtManagerCallbackWrapper = new jnicat::JniObjectWrapper<SdtManagerJniCallback>(sdtManagerJniCallback);
        sdtManagerCallbackWrapper->instantiate(env, instance, "callbackHandle");
        sdt_manager_cpp->SetCallBack(sdtManagerJniCallback);
    }

    static jobject JniGetLoadLibraries(JNIEnv* env, jobject instance) {
        return mars::baseevent::getLoadLibraries(env);
    }

    static void JniSetHttpNetCheckCGI(JNIEnv* env, jobject instance, jstring cgi) {
        auto sdt_manager_cpp = jnicat::JniObjectWrapper<SdtManager>::object(env, instance);
        sdt_manager_cpp->SetHttpNetcheckCGI(j2c_cast(cgi));
    }
};

static const JNINativeMethod kSdtManagerJniMethods[] = {
    {"OnJniCreateSdtManagerFromHandle", "(J)V", (void*)&mars::sdt::JniSdtManager::JniCreateSdtManagerFromHandle},
    {"OnJniCreateSdtManagerFromContext",
     "(Ljava/lang/Object;)V",
     (void*)&mars::sdt::JniSdtManager::JniCreateSdtManagerFromContext},
    {"OnJniDestroySdtManager", "()V", (void*)&mars::sdt::JniSdtManager::JniOnDestroySdtManager},
    {"OnJniSetCallback", "(Ljava/lang/Object;)V", (void*)&mars::sdt::JniSdtManager::JniSetCallback},
    {"OnJniGetLoadLibraries", "()Ljava/util/ArrayList;", (void*)&mars::sdt::JniSdtManager::JniGetLoadLibraries},
    {"OnJniSetHttpNetCheckCGI", "(Ljava/lang/String;)V", (void*)&mars::sdt::JniSdtManager::JniSetHttpNetCheckCGI},
};

static const size_t kSdtManagerJniMethodsCount = sizeof(kSdtManagerJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kSdtManagerInterface,
                         "com/tencent/mars/sdt/SdtManager",
                         kSdtManagerJniMethods,
                         kSdtManagerJniMethodsCount)

}  // namespace sdt
}  // namespace mars

void ExportSdtManager() {
}
