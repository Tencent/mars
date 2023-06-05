//
// Created by Changpeng Pan on 2022/9/13.
//

#include <jni.h>

#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "mars/comm/xlogger/xlogger.h"

using namespace mars::boot;

namespace mars {
namespace app {

class JniContext {
 private:
 public:
    static void JniCreateContext(JNIEnv* env, jobject instance, jstring context_id) {
        auto context_cpp = boot::Context::CreateContext(j2c_cast(context_id));
        auto contextWrapper = new jnicat::JniObjectWrapper<Context>(context_cpp);
        contextWrapper->instantiate(env, instance);
    }

    static void JniCreateContextFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto context_cpp = (Context*)j2c_cast(handle);
        auto contextWrapper = new jnicat::JniObjectWrapper<Context>(context_cpp);
        contextWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyContext(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        context_cpp->DestroyContext(context_cpp);
        jnicat::JniObjectWrapper<Context>::dispose(env, instance);
    }

    static jint JniInit(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        return c2j_cast(jint, context_cpp->Init());
    }

    static jint JniUnInit(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        return c2j_cast(jint, context_cpp->UnInit());
    }

    static void JniAddManager(JNIEnv* env, jobject instance, jstring name, jobject manager) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        auto manager_cpp = jnicat::JniObjectWrapper<BaseManager>::object(env, manager);
        context_cpp->AddManagerWithName(j2c_cast(name), manager_cpp);
    }

    static jlong JniGetManager(JNIEnv* env, jobject instance, jstring name) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        jlong handle = c2j_cast(jlong, (int64_t)context_cpp->GetManagerByName(j2c_cast(name)));
        return handle;
    }

    static void JniRemoveManager(JNIEnv* env, jobject instance, jstring name, jobject manager) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, instance);
        auto manager_cpp = jnicat::JniObjectWrapper<BaseManager>::object(env, manager);
        context_cpp->RemoveManagerWithName(j2c_cast(name), manager_cpp);
    }
};

static const JNINativeMethod kContextJniMethods[] = {
    {"OnJniCreateContext", "(Ljava/lang/String;)V", (void*)&JniContext::JniCreateContext},
    {"OnJniDestroyContext", "()V", (void*)&JniContext::JniOnDestroyContext},
    {"OnJniCreateContextFromHandle", "(J)V", (void*)&JniContext::JniCreateContextFromHandle},
    {"OnJniInit", "()I", (void*)&JniContext::JniInit},
    {"OnJniUnInit", "()I", (void*)&JniContext::JniUnInit},
    {"OnJniAddManager", "(Ljava/lang/String;Ljava/lang/Object;)V", (void*)&JniContext::JniAddManager},
    {"OnJniGetManager", "(Ljava/lang/String;)J", (void*)&JniContext::JniGetManager},
    {"OnJniRemoveManager", "(Ljava/lang/String;Ljava/lang/Object;)V", (void*)&JniContext::JniRemoveManager},
};

static const size_t kContextJniMethodsCount = sizeof(kContextJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kContextInterface, "com/tencent/mars/app/Context", kContextJniMethods, kContextJniMethodsCount)

}  // namespace app
}  // namespace mars

void ExportJniContext() {
}