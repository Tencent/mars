/*
 * com_tencent_mars_sdt_SdtLogic_C2Java.cc
 *
 *  Created on: 2016年8月9日
 *      Author: caoshaokun
 */

#include <jni.h>
#include <vector>
#include <sstream>

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/compiler_util.h"
#include "mars/sdt/sdt.h"
#include "mars/sdt/netchecker_profile.h"

DEFINE_FIND_CLASS(KC2Java, "com/tencent/mars/sdt/SdtLogic")

namespace mars {
namespace sdt {

DEFINE_FIND_STATIC_METHOD(KC2Java_reportSignalDetectResults, KC2Java, "reportSignalDetectResults", "(Ljava/lang/String;)V")
WEAK_FUNC void ReportNetCheckResult(std::vector<CheckResultProfile>& _check_results) {
	xverbose_function();

	VarCache* cache_instance = VarCache::Singleton();
	ScopeJEnv scope_jenv(cache_instance->GetJvm());
	JNIEnv *env = scope_jenv.GetEnv();

	std::stringstream check_results_str;
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
		}
		else {
			break;
		}
	}
	check_results_str << "]}";

	JNU_CallStaticMethodByMethodInfo(env, KC2Java_reportSignalDetectResults, ScopedJstring(env, check_results_str.str().c_str()).GetJstr());
}

}}
