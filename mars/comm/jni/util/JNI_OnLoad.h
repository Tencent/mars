// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#include <jni.h>
#include "comm/bootregister.h"

typedef jint JNICALL(*JniOnLoadFunc)(JavaVM*, void*);
struct JniOnload_t {
    explicit JniOnload_t(JniOnLoadFunc _func): func(_func) {}
    JniOnLoadFunc func;
};

#define JNI_ONLOAD_INIT(func) BOOT_REGISTER(JniOnload_t(func))
