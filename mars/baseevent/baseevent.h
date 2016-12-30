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
 * baseevent.h
 *
 *  Created on: 2016年3月24日
 *      Author: caoshaokun
 */

#ifndef BASEEVENT_INTERFACE_BASEEVENT_H_
#define BASEEVENT_INTERFACE_BASEEVENT_H_

#include <vector>
#include <string>

#ifdef ANDROID
#include <jni.h>

namespace mars {
namespace baseevent {

	extern void addLoadModule(std::string _module_name);

	extern jobject getLoadLibraries(JNIEnv *_env);

}
}

#endif

#endif /* BASEEVENT_INTERFACE_BASEEVENT_H_ */
