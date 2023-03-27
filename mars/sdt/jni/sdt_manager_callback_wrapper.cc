//
// Created by Cpan on 2022/9/20.
//
#include "sdt_manager_callback_wrapper.h"

#include <vector>

#include "comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/sdt/netchecker_profile.h"

// C2Java

namespace mars {
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

JNICAT_DEFINE_METHOD(kSdtManagerJniCallback_ReportNetCheckResult,
                     "com/tencent/mars/sdt/SdtManager$CallBack",
                     "reportSignalDetectResults",
                     "(Ljava/lang/String;)V")
void SdtManagerJniCallback::ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results) {
    jnienv_ptr env;

    XMessage check_results_str;
    check_results_str << "{";
    check_results_str << "\"details\":[";
    std::vector<CheckResultProfile>::const_iterator iter = _check_results.begin();
    for (; iter != _check_results.end();) {
        check_results_str << "{";
        check_results_str << "\"detectType\":" << iter->netcheck_type;
        check_results_str << ",\"errorCode\":" << iter->error_code;
        check_results_str << ",\"networkType\":" << iter->network_type;
        check_results_str << ",\"detectIP\":\"" << iter->ip << "\"";
        check_results_str << ",\"port\":" << iter->port;
        check_results_str << ",\"conntime\":" << iter->conntime;
        check_results_str << ",\"rtt\":" << iter->rtt;
        check_results_str << ",\"rttStr\":\"" << iter->rtt_str << "\"";
        check_results_str << ",\"httpStatusCode\":" << iter->status_code;
        check_results_str << ",\"pingCheckCount\":" << iter->checkcount;
        check_results_str << ",\"pingLossRate\":\"" << iter->loss_rate << "\"";
        check_results_str << ",\"dnsDomain\":\"" << iter->domain_name << "\"";
        check_results_str << ",\"localDns\":\"" << iter->local_dns << "\"";
        check_results_str << ",\"dnsIP1\":\"" << iter->ip1 << "\"";
        check_results_str << ",\"dnsIP2\":\"" << iter->ip2 << "\"";
        check_results_str << "}";
        if (++iter != _check_results.end()) {
            check_results_str << ",";
        } else {
            break;
        }
    }
    check_results_str << "]}";

    c2j_call(void,
             callback_inst_,
             kSdtManagerJniCallback_ReportNetCheckResult,
             c2j_cast(jstring, check_results_str.String().c_str()));
}

}  // namespace sdt
}  // namespace mars