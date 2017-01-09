#include <jni.h>
#include <string>

#include "xlogger/xloggerbase.h"
#include "xlogger/xlogger.h"

#include "xlogger/android_xlog.h"

extern "C"
jstring
Java_com_tencent_mars_xlogsample_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

	xinfo_function();


	xdebug2(TSF "test xlog, have level filter. line:%0, func:%1", __LINE__, __FUNCTION__);

	xdebug2("test xlog, have level filter. line:%d, func:%s", __LINE__, __FUNCTION__);

	xassert2(1<0, "assert false info");

	xassert2(false);
	xassert2(true);

	xverbose2(TSF "test xlog, have level filter");


	LOGD("testxlog", "-------user define:%d--------", __LINE__);
	
	__android_log_print(ANDROID_LOG_INFO, "test", "123");
	
	__android_log_print(ANDROID_LOG_INFO, "test", "123:%d", 4);
	__android_log_write(ANDROID_LOG_INFO, "test", "123");
	
	__android_log_assert(1>0, "test", "%d", 3455);
	
	__android_log_assert(1<0, "test", "%d", 3455);
	
	__android_log_assert(1<0, "test", "3455dfdddddddddd");
	

	LOGI("test", "111111111111");

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
