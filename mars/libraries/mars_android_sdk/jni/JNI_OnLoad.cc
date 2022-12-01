
#include "comm/jni/util/JNI_OnLoad.h"
#include <android/log.h>
#include <pthread.h>
#include "comm/jni/util/var_cache.h"
#include "comm/jni/util/scope_jenv.h"
#include "comm/jni/jnicat/jnicat_core.h"

pthread_key_t g_env_key;

static void __DetachCurrentThread(void* a) {
    if (NULL != VarCache::Singleton()->GetJvm()) {
        VarCache::Singleton()->GetJvm()->DetachCurrentThread();
    }
}

static void MyExceptionHandler(const std::string& stacktrace) {
    std::abort();
}

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    
       if (0 != pthread_key_create(&g_env_key, __DetachCurrentThread)) {
       __android_log_print(ANDROID_LOG_ERROR, "MicroMsg", "create g_env_key fail");
       return(-1);
   }

    ScopeJEnv jenv(jvm);
    VarCache::Singleton()->SetJvm(jvm);


    LoadClass(jenv.GetEnv());
    LoadStaticMethod(jenv.GetEnv());
    LoadMethod(jenv.GetEnv());

    // owl::co_prepare_jvm(jvm);
    std::vector<JniOnload_t>& ref = BOOT_REGISTER_CONTAINER<JniOnload_t>() ;
    for (std::vector<JniOnload_t>::const_iterator it= ref.begin(); it!=ref.end(); ++it)
    {
    	it->func(jvm, reserved);
    }
	
    //jcache::shared()->set_exception_handler();
    jcache::shared()->set_exception_handler(&MyExceptionHandler);
    jcache::shared()->init(jvm);

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    VarCache::Release();
}

}
