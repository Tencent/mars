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
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include "assert/__assert.h"

extern pthread_key_t g_env_key;

ScopeJEnv::ScopeJEnv(JavaVM* jvm, jint _capacity)
    : vm_(jvm), env_(NULL), we_attach_(false), status_(0) {
    ASSERT(jvm);
    do {
        env_ = (JNIEnv*)pthread_getspecific(g_env_key);
        
        if (NULL != env_) {
            break;
        }
        
        status_ = vm_->GetEnv((void**) &env_, JNI_VERSION_1_6);

        if (JNI_OK == status_) {
            break;
        }

        JavaVMAttachArgs args;
        args.group = NULL;
        args.name = "default";
        args.version = JNI_VERSION_1_6;
        status_ = vm_->AttachCurrentThread(&env_, &args);

        if (JNI_OK == status_) {
            we_attach_ = true;
            pthread_setspecific(g_env_key, env_);
        } else {
            ASSERT2(false, "vm:%p, env:%p, status:%d", vm_, env_, status_);
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
