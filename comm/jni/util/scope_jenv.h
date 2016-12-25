/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * scop_jenv.h
 *
 *  Created on: 2012-8-21
 *      Author: yanguoyue
 */

#ifndef SCOP_JENV_H_
#define SCOP_JENV_H_

#include <jni.h>

class ScopeJEnv {
  public:
    ScopeJEnv(JavaVM* jvm, jint _capacity = 16);
    ~ScopeJEnv();

    JNIEnv* GetEnv();
    int Status();

  private:
    JavaVM* vm_;
    JNIEnv* env_;
    bool we_attach_;
    int status_;
};


#endif /* SCOP_JENV_H_ */
