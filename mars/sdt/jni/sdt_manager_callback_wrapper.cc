//
// Created by Cpan on 2022/9/20.
//
#include <vector>
#include "sdt_manager_callback_wrapper.h"

#include "comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"

namespace mars{
namespace sdt {

SdtManagerJniCallback::SdtManagerJniCallback(JNIEnv* env, jobject callback) {
    callback_inst_ = env->NewGlobalRef(callback);
    jclass objClass = env->GetObjectClass(callback);
    if (objClass) {
        callbacks_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(objClass));
        env->DeleteLocalRef(objClass);
    }
}

SdtManagerJniCallback::~SdtManagerJniCallback() {
    jnienv_ptr env;
    env->DeleteGlobalRef(callback_inst_);
}

JNICAT_DEFINE_METHOD(kSdtManagerJniCallback_ReportNetCheckResult, "com/tencent/mars/sdt/SdtManager$CallBack", "reportSignalDetectResults", "(Ljava/lang/String;)V")
void ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results){
    jnienv_ptr env;
    jstring result ;
    c2j_call(void, callback_inst_,kSdtManagerJniCallback_ReportNetCheckResult, result);
}

}
}