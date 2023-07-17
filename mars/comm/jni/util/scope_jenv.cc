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
 * scop_jenv.cpp
 *
 *  Created on: 2012-8-21
 *      Author: yanguoyue
 */

#include "scope_jenv.h"

#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

#include <cstdio>

#include "assert/__assert.h"
#include "mars/comm/jni/util/var_cache.h"

ScopeJEnv::ScopeJEnv(JavaVM* jvm, jint _capacity) : env_(NULL), status_(0) {
    if (nullptr == jvm) {
        jvm = VarCache::Singleton()->GetJvm();
    }
    ASSERT(jvm);
    do {
        status_ = jvm->GetEnv((void**)&env_, JNI_VERSION_1_6);

        if (JNI_OK == status_) {
            ASSERT2(env_ != NULL, "env_ %p", env_);
            break;
        }

        char thread_name[32] = {0};
        snprintf(thread_name, sizeof(thread_name), "mars::%d", (int)gettid());
        JavaVMAttachArgs args;
        args.group = NULL;
        args.name = thread_name;
        args.version = JNI_VERSION_1_6;
        status_ = jvm->AttachCurrentThread(&env_, &args);

        if (JNI_OK == status_) {
            thread_local struct OnExit {
                ~OnExit() {
                    if (NULL != VarCache::Singleton()->GetJvm()) {
                        VarCache::Singleton()->GetJvm()->DetachCurrentThread();
                    }
                }
            } dummy;
        } else {
            ASSERT2(false, "vm:%p, env:%p, status:%d", jvm, env_, status_);
            env_ = NULL;
            return;
        }
    } while (false);

    jint ret = env_->PushLocalFrame(_capacity);
    ASSERT2(0 == ret, "ret:%d", ret);
}

ScopeJEnv::~ScopeJEnv() {
    if (NULL != env_) {
        env_->PopLocalFrame(NULL);
    }
}

JNIEnv* ScopeJEnv::GetEnv() {
    return env_;
}

int ScopeJEnv::Status() {
    return status_;
}
