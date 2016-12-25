/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * ScopedJstring.cpp
 *
 *  Created on: 2014-8-27
 *      Author: yanguoyue
 */

#include "scoped_jstring.h"

#include <stddef.h>
#include <string.h>

#include "assert/__assert.h"

ScopedJstring::ScopedJstring(JNIEnv* _env, jstring _jstr)
: env_(_env)
, jstr_((jstring)_env->NewLocalRef(_jstr))
, char_(NULL)
, jstr2char_(true)
{
    ASSERT(env_);
    if (NULL == env_ || NULL == jstr_)
    {
        return;
    }

    if (env_->ExceptionOccurred())
    {
        return;
    }

    char_ =  env_->GetStringUTFChars(jstr_, NULL);
}

ScopedJstring::ScopedJstring(JNIEnv* _env, const char* _char)
: env_(_env)
, jstr_(NULL)
, char_(_char)
, jstr2char_(false)
{

    ASSERT(env_);
    if (NULL == env_ || NULL == _char)
    {
        return;
    }

    if (env_->ExceptionOccurred())
    {
        return;
    }

    jclass strClass = env_->FindClass("java/lang/String");
    jmethodID ctorID = env_->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");

    jbyteArray bytes = env_->NewByteArray((jsize)strlen(char_));
    env_->SetByteArrayRegion(bytes, 0, (jsize)strlen(char_), (jbyte*) char_);
    jstring encoding = env_->NewStringUTF("utf-8");

    jstr_ = (jstring) env_->NewObject(strClass, ctorID, bytes, encoding);

    env_->DeleteLocalRef(bytes);
    env_->DeleteLocalRef(encoding);
    env_->DeleteLocalRef(strClass);
}

ScopedJstring::~ScopedJstring()
{
    if (NULL == env_ || NULL == jstr_ || NULL == char_)
    {
        return;
    }

    if (env_->ExceptionOccurred())
    {
        return;
    }

    if (jstr2char_)
    {
        env_->ReleaseStringUTFChars(jstr_, char_);
    }

    env_->DeleteLocalRef(jstr_);
}

const char* ScopedJstring::GetChar() const
{
    if (env_->ExceptionOccurred())
    {
        return NULL;
    }

    return char_;
}

jstring ScopedJstring::GetJstr() const
{
    if (env_->ExceptionOccurred())
    {
        return NULL;
    }

    return jstr_;
}

