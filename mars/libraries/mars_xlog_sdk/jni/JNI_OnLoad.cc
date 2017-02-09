
#include "comm/jni/util/JNI_OnLoad.h"
#include <android/log.h>
#include <pthread.h>
#include "comm/jni/util/var_cache.h"
#include "comm/jni/util/scope_jenv.h"

pthread_key_t g_env_key;

static void __DetachCurrentThread(void* a) {
    if (NULL != VarCache::Singleton()->GetJvm()) {
        VarCache::Singleton()->GetJvm()->DetachCurrentThread();
    }
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

    std::vector<JniOnload_t>& ref = BOOT_REGISTER_CONTAINER<JniOnload_t>() ;
    for (std::vector<JniOnload_t>::const_iterator it= ref.begin(); it!=ref.end(); ++it)
    {
    	it->func(jvm, reserved);
    }
	
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
    VarCache::Release();
}

}
