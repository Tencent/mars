/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#include <jni.h>
#include "comm/bootregister.h"

typedef jint JNICALL(*JniOnLoadFunc)(JavaVM*, void*);
struct JniOnload_t {
    explicit JniOnload_t(JniOnLoadFunc _func): func(_func) {}
    JniOnLoadFunc func;
};

#define JNI_ONLOAD_INIT(func) BOOT_REGISTER(JniOnload_t(func))
