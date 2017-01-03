// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

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
