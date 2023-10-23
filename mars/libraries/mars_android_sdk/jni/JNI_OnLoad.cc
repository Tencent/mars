#include "comm/jni/util/JNI_OnLoad.h"

#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/util/scope_jenv.h"
#include "comm/jni/util/var_cache.h"

static void MyExceptionHandler(const std::string& stacktrace) {
    std::abort();
}

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved) {
    ScopeJEnv jenv(jvm);
    VarCache::Singleton()->SetJvm(jvm);

    LoadClass(jenv.GetEnv());
    LoadStaticMethod(jenv.GetEnv());
    LoadMethod(jenv.GetEnv());

    std::vector<JniOnload_t>& ref = BOOT_REGISTER_CONTAINER<JniOnload_t>();
    for (std::vector<JniOnload_t>::const_iterator it = ref.begin(); it != ref.end(); ++it) {
        it->func(jvm, reserved);
    }

    jcache::shared()->set_exception_handler(&MyExceptionHandler);
    jcache::shared()->init(jvm);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    VarCache::Release();
}
}
