// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/**
 * created on : 2012-07-19
 * author : yanguoyue
 */

#include <jni.h>
#include <android/log.h>

#include "comm/jni/util/scoped_jstring.h"

#include "mars/baseevent/base_logic.h"
#include "mars/baseevent/baseevent.h"

//DEFINE_FIND_CLASS(KBaseEventJava2C, "com/tencent/mars/BaseEvent");

namespace mars {
namespace baseevent {

	std::vector<std::string>* getLoadModuleVec(){
		static std::vector<std::string> sg_modules;
		return &sg_modules;
	}

	void addLoadModule(std::string _module_name) {
		getLoadModuleVec()->push_back(_module_name);
	}

	jobject getLoadLibraries(JNIEnv *_env) {
		jclass list_cls = _env->FindClass("java/util/ArrayList");
		jmethodID list_costruct = _env->GetMethodID(list_cls, "<init>", "()V");
		jobject list_obj = _env->NewObject(list_cls , list_costruct);

		jmethodID list_add  = _env->GetMethodID(list_cls, "add", "(Ljava/lang/Object;)Z");

		for(std::vector<std::string>::iterator iter = getLoadModuleVec()->begin(); iter != getLoadModuleVec()->end(); ++iter){
			_env->CallBooleanMethod(list_obj , list_add , ScopedJstring(_env, (*iter).c_str()).GetJstr());
		}

		return list_obj;
	}
}
}

extern "C" {

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onCreate(JNIEnv* env, jclass)
{
    mars::baseevent::OnCreate();
}

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onDestroy(JNIEnv* env, jclass)
{
    mars::baseevent::OnDestroy();
}

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onForeground (JNIEnv *, jclass, jboolean _isforeground)
{
    mars::baseevent::OnForeground(_isforeground);
}

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onNetworkChange (JNIEnv *, jclass)
{
	mars::baseevent::OnNetworkChange();
}

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onSingalCrash(JNIEnv *, jclass, jint _sig){
    mars::baseevent::OnSingalCrash((int)_sig);
}

JNIEXPORT void JNICALL Java_com_tencent_mars_BaseEvent_onExceptionCrash(JNIEnv*, jclass){
    mars::baseevent::OnExceptionCrash();
}

}

void ExportBaseEvent() {
}
