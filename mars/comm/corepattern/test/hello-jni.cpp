/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <string.h>
#include <jni.h>
#include "TestCoreService.h"
#include "TestCoreService2.h"
#include "TestService.h"
#include "TestService2.h"
#include "TestService3.h"

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/com/example/hellojni/HelloJni.java
 */
extern "C" JNIEXPORT jstring
Java_com_example_hellojni_HelloJni_stringFromJNI( JNIEnv* env,
                                                  jobject thiz )
{

	design_patterns::TestCoreService t;
//	int i = t.GetPublicService<design_patterns::TestService>()->test(1, 2);
//	int i3 = t.GetPublicService<design_patterns::TestService3>()->test2(3, 4);
	int i3 = t.GetPublicService<design_patterns::TestCoreService2>()->GetPublicService<design_patterns::TestService3>()->test2(3, 4);
    return env->NewStringUTF("Hello from JNI !");
}
