//
// Created by Changpeng Pan on 2022/9/13.
//

#include <android/log.h>
#include <jni.h>

#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/app/app_manager.h"
#include "mars/comm/base_app_manager.h"
#include "mars/comm/base_context.h"
#include "mars/boot/context.h"

namespace mars {
namespace app {

class JniContext {
 public:
    static void JniCreateContext(JNIEnv* env, jobject instance, jstring context_id) {
        auto context_cpp = CreateContext(j2c_cast(context_id));
        auto contextWrapper = new jnicat::JniObjectWrapper<BaseContext>(context_cpp);
        contextWrapper->instantiate(env, instance);
    }

    static void JniCreateContextFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto context_cpp = (BaseContext*)j2c_cast(handle);
        auto contextWrapper = new jnicat::JniObjectWrapper<BaseContext>(context_cpp);
        contextWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyContext(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<BaseContext>::object(env, instance);
        DestroyContext(context_cpp);
        jnicat::JniObjectWrapper<BaseContext>::dispose(env, instance);
    }

    static jint JniInit(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<BaseContext>::object(env, instance);
        return c2j_cast(jint, context_cpp->Init());
    }

    static jint JniUnInit(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<BaseContext>::object(env, instance);
        return c2j_cast(jint, context_cpp->UnInit());
    }

    static jlong JniGetAppManager(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<BaseContext>::object(env, instance);
        return c2j_cast(jlong, (int64_t)context_cpp->GetAppManager());
    }

    static jlong JniGetStnManager(JNIEnv* env, jobject instance) {
        auto context_cpp = jnicat::JniObjectWrapper<BaseContext>::object(env, instance);
        return c2j_cast(jlong, (int64_t)context_cpp->GetStnManager());
    }
};

static const JNINativeMethod kContextJniMethods[] = {
    {"OnJniCreateContext", "(Ljava/lang/String;)V", (void*)&JniContext::JniCreateContext},
    {"OnJniDestroyContext", "()V", (void*)&JniContext::JniOnDestroyContext},
    {"OnJniCreateContextFromHandle", "(J)V", (void*)&JniContext::JniCreateContextFromHandle},
    {"OnJniInit", "()I", (void*)&JniContext::JniInit},
    {"OnJniUnInit", "()I", (void*)&JniContext::JniUnInit},
    {"OnJniGetAppManager", "()J", (void*)&JniContext::JniGetAppManager},
    {"OnJniGetStnManager", "()J", (void*)&JniContext::JniGetStnManager},
};

static const size_t kContextJniMethodsCount = sizeof(kContextJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kContextInfterface, "com/tencent/mars/app/Context", kContextJniMethods, kContextJniMethodsCount)

}  // namespace app
}  // namespace mars

void ExportContext() {
}