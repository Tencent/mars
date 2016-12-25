/*
 * com_tencent_mars_sdt_SdtLogic_Java2C.cc
 *
 *  Created on: 2016年4月11日
 *      Author: caoshaokun
 */

#include <jni.h>

#include "mars/baseevent/baseevent.h"
#include "mars/comm/jni/util/scoped_jstring.h"

#include "mars/sdt/sdt_logic.h"

using namespace mars::sdt;

extern "C" {

/*
 * Class:     com_tencent_mars_sdt_SdtLogic
 * Method:    getLoadLibraries
 * Signature: ()jobject
 */
JNIEXPORT jobject JNICALL Java_com_tencent_mars_sdt_SdtLogic_getLoadLibraries
  (JNIEnv *_env, jclass clz) {

	return mars::baseevent::getLoadLibraries(_env);
}

JNIEXPORT void JNICALL Java_com_tencent_mars_sdt_SdtLogic_setHttpNetcheckCGI
  (JNIEnv *_env, jclass clz, jstring cgi) {

	SetHttpNetcheckCGI(ScopedJstring(_env, cgi).GetChar());
}

}

void ExportSDT() {

}
