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
 * created on : 2012-07-30
 * author    : yanguoyue
 */

#include "comm_function.h"

#include <jni.h>
#include <string>

#include "assert/__assert.h"
#include "var_cache.h"
#include "autobuffer.h"



using namespace std;


jvalue __JNU_CallMethodByName(
    JNIEnv* env,
    jobject obj,
    const char* name,
    const char* descriptor,
	va_list args) {
        
    ASSERT(env != NULL);
    ASSERT(obj != NULL);
    ASSERT(name != NULL);
    ASSERT(descriptor != NULL);

    VarCache* cacheInastance = VarCache::Singleton();

    jclass clazz;
    jmethodID mid;
    jvalue result;
    memset(&result, 0 , sizeof(result));

    if (env->EnsureLocalCapacity(2) == JNI_OK) {
        clazz = env->GetObjectClass(obj);

        ASSERT(clazz != NULL);

        mid = cacheInastance->GetMethodId(env, clazz, name, descriptor);

        ASSERT(mid != NULL);

        if (mid) {
            const char* p = descriptor;

            /* skip over argument types to find out the
             return type */
            while (*p != ')')
                p++;

            /* skip ')' */
            p++;

            switch (*p) {
            case 'V':
                env->CallVoidMethodV(obj, mid, args);
                break;

            case '[':
            case 'L':
                result.l = env->CallObjectMethodV(obj, mid, args);
                break;

            case 'Z':
                result.z = env->CallBooleanMethodV(obj, mid, args);
                break;

            case 'B':
                result.b = env->CallByteMethodV(obj, mid, args);
                break;

            case 'C':
                result.c = env->CallCharMethodV(obj, mid, args);
                break;

            case 'S':
                result.s = env->CallShortMethodV(obj, mid, args);
                break;

            case 'I':
                result.i = env->CallIntMethodV(obj, mid, args);
                break;

            case 'J':
                result.j = env->CallLongMethodV(obj, mid, args);
                break;

            case 'F':
                result.f = env->CallFloatMethodV(obj, mid, args);
                break;

            case 'D':
                result.d = env->CallDoubleMethodV(obj, mid, args);
                break;

            default:
                env->FatalError("illegal descriptor");
                break;
            }

        }

        env->DeleteLocalRef(clazz);
    }

    return result;
}

/**
 *
 * @param _env
 * @param obj    the object that invoke the method
 * @param _name    the method _name
 * @param _descriptor    the _descriptor of the method's parameters
 * @return
 */
jvalue JNU_CallMethodByName(
    JNIEnv* _env,
    jobject obj,
    const char* _name,
    const char* _descriptor, ...) {
        
    ASSERT(_env != NULL);
    ASSERT(obj != NULL);
    ASSERT(_name != NULL);
    ASSERT(_descriptor != NULL);

    va_list args;
	va_start(args, _descriptor);

	jvalue result =	__JNU_CallMethodByName( _env, obj, _name, _descriptor, args);

	va_end(args);
	return result;
}


jvalue __JNU_CallStaticMethodByName(
    JNIEnv* _env,
    jclass _clazz,
    const char* _name,
    const char* _descriptor,
    va_list args) {

    ASSERT(_env != NULL);
    ASSERT(_clazz != NULL);
    ASSERT(_name != NULL);
    ASSERT(_descriptor != NULL);


    VarCache* cacheInastance = VarCache::Singleton();

    jmethodID mid;
    jvalue result;
    memset(&result, 0 , sizeof(result));

    mid = cacheInastance->GetStaticMethodId(_env, _clazz, _name, _descriptor);

    ASSERT2(mid != NULL, "mid == NULL, _name= %s, des= %s", _name, _descriptor);

    if (mid) {
        const char* p = _descriptor;

        /* skip over argument types to find out the
         return type */
        while (*p != ')')
            p++;

        /* skip ')' */
        p++;

        switch (*p) {
        case 'V':
            _env->CallStaticVoidMethodV(_clazz, mid, args);
            break;

        case '[':
        case 'L':
            result.l = _env->CallStaticObjectMethodV(_clazz, mid, args);
            break;

        case 'Z':
            result.z = _env->CallStaticBooleanMethodV(_clazz, mid, args);
            break;

        case 'B':
            result.b = _env->CallStaticByteMethodV(_clazz, mid, args);
            break;

        case 'C':
            result.c = _env->CallStaticCharMethodV(_clazz, mid, args);
            break;

        case 'S':
            result.s = _env->CallStaticShortMethodV(_clazz, mid, args);
            break;

        case 'I':
            result.i = _env->CallStaticIntMethodV(_clazz, mid, args);
            break;

        case 'J':
            result.j = _env->CallStaticLongMethodV(_clazz, mid, args);
            break;

        case 'F':
            result.f = _env->CallStaticFloatMethodV(_clazz, mid, args);
            break;

        case 'D':
            result.d = _env->CallStaticDoubleMethodV(_clazz, mid, args);
            break;

        default:
            _env->FatalError("illegal _descriptor");
            break;
        }
    }

    return result;
}

jvalue JNU_CallStaticMethodByName(
    JNIEnv* _env,
    jclass _clazz,
    const char* _name,
    const char* _descriptor, ...) {
    ASSERT(_env != NULL);
    ASSERT(_clazz != NULL);
    ASSERT(_name != NULL);
    ASSERT(_descriptor != NULL);

    va_list args;

    va_start(args, _descriptor);
    jvalue result = __JNU_CallStaticMethodByName(_env, _clazz, _name, _descriptor, args);
    va_end(args);

    return result;
}

jvalue JNU_CallStaticMethodByName(
    JNIEnv* _env,
    const char* className,
    const char* _name,
    const char* _descriptor, ...) {
    ASSERT(_env != NULL);
    ASSERT(className != NULL);
    ASSERT(_name != NULL);
    ASSERT(_descriptor != NULL);

    jclass _clazz = VarCache::Singleton()->GetClass(_env, className);
    ASSERT(_clazz != NULL);

    va_list args;
    va_start(args, _descriptor);
    jvalue result = __JNU_CallStaticMethodByName(_env, _clazz, _name, _descriptor, args);
    va_end(args);

    return result;
}

jvalue JNU_CallStaticMethodByMethodInfo(JNIEnv* _env, JniMethodInfo methodInfo, ...) {
    ASSERT(_env != NULL);

    jclass _clazz = VarCache::Singleton()->GetClass(_env, methodInfo.classname.c_str());

    va_list args;
    va_start(args, methodInfo);
    jvalue result = __JNU_CallStaticMethodByName(_env, _clazz, methodInfo.methodname.c_str(), methodInfo.methodsig.c_str(), args);
    va_end(args);

    return result;
}

jvalue JNU_GetStaticField(JNIEnv* _env, jclass _clazz, const char* _name, const char* sig) {
    ASSERT(NULL != _env);
    ASSERT(NULL != _clazz);
    ASSERT(NULL != _name);
    ASSERT(NULL != sig);


    jvalue result;
    memset(&result, 0 , sizeof(result));

    jfieldID fid = VarCache::Singleton()->GetStaticFieldId(_env, _clazz, _name, sig);

    if (NULL == fid) {
        return result;
    }

    switch (*sig) {
    case '[':
    case 'L':
        result.l = _env->GetStaticObjectField(_clazz, fid);
        break;

    case 'Z':
        result.z = _env->GetStaticBooleanField(_clazz, fid);
        break;

    case 'B':
        result.b = _env->GetStaticByteField(_clazz, fid);
        break;

    case 'C':
        result.c = _env->GetStaticCharField(_clazz, fid);
        break;

    case 'S':
        result.s = _env->GetStaticShortField(_clazz, fid);
        break;

    case 'I':
        result.i = _env->GetStaticIntField(_clazz, fid);
        break;

    case 'J':
        result.j = _env->GetStaticLongField(_clazz, fid);
        break;

    case 'F':
        result.f = _env->GetStaticFloatField(_clazz, fid);
        break;

    case 'D':
        result.d = _env->GetStaticDoubleField(_clazz, fid);
        break;

    default:
        _env->FatalError("illegal _descriptor");
        break;
    }

    return result;
}

jvalue JNU_GetField(JNIEnv* _env, jobject obj, const char* _name, const char* sig) {
    jvalue result;
    memset(&result, 0 , sizeof(result));

    if (_env->ExceptionOccurred()) {
        return result;
    }

    ASSERT(NULL != _env);
    ASSERT(NULL != obj);
    ASSERT(NULL != _name);
    ASSERT(NULL != sig);

    jclass _clazz = _env->GetObjectClass(obj);
    jfieldID fid = VarCache::Singleton()->GetFieldId(_env, _clazz, _name, sig);
    _env->DeleteLocalRef(_clazz);

    if (NULL == fid) {
        return result;
    }

    switch (*sig) {
    case '[':
    case 'L':
        result.l = _env->GetObjectField(obj, fid);
        break;

    case 'Z':
        result.z = _env->GetBooleanField(obj, fid);
        break;

    case 'B':
        result.b = _env->GetByteField(obj, fid);
        break;

    case 'C':
        result.c = _env->GetCharField(obj, fid);
        break;

    case 'S':
        result.s = _env->GetShortField(obj, fid);
        break;

    case 'I':
        result.i = _env->GetIntField(obj, fid);
        break;

    case 'J':
        result.j = _env->GetLongField(obj, fid);
        break;

    case 'F':
        result.f = _env->GetFloatField(obj, fid);
        break;

    case 'D':
        result.d = _env->GetDoubleField(obj, fid);
        break;

    default:
        _env->FatalError("illegal _descriptor");
        break;
    }

    return result;
}


jvalue JNU_CallMethodByMethodInfo(JNIEnv* env, jobject obj, JniMethodInfo methodInfo, ...) {
    ASSERT(env != NULL);


    va_list args;
    va_start(args, methodInfo);

    jvalue result = __JNU_CallMethodByName(env, obj, methodInfo.methodname.c_str(), methodInfo.methodsig.c_str(), args);
    va_end(args);

    return result;
}

/**
 *remember to free bytes using DeleteLocalRef
 * @param _env
 * @param ab
 * @param bytes
 * @return
 */
jbyteArray JNU_Buffer2JbyteArray(JNIEnv* _env, const AutoBuffer& ab) {
    unsigned int len = ab.Length();

    if (len == 0) {
        return NULL;
    }

    jbyteArray bytes = _env->NewByteArray((jsize)len);
    _env->SetByteArrayRegion(bytes, 0, (jsize)len, (jbyte*) ab.Ptr());
    return bytes;
}

jbyteArray JNU_Buffer2JbyteArray(JNIEnv* _env, const void* _buffer, size_t _length) {
    unsigned int len = _length;

    if (len == 0) {
        return NULL;
    }

    jbyteArray bytes = _env->NewByteArray((jsize)len);
    _env->SetByteArrayRegion(bytes, 0, (jsize)len, (jbyte*) _buffer);
    return bytes;
}

void JNU_FreeJbyteArray(JNIEnv* _env, jbyteArray bytes) {
    ASSERT(_env != NULL);
    ASSERT(bytes != NULL);
    _env->DeleteLocalRef(bytes);
}

bool JNU_JbyteArray2Buffer(
    JNIEnv* _env,
    const jbyteArray bytes,
    AutoBuffer& ab) {
    ASSERT(_env != NULL);

    if (_env == NULL) {
        return false;
    }

    if (bytes == NULL) {
        return true;
    }

    jsize alen = _env->GetArrayLength(bytes);
    jbyte* ba = _env->GetByteArrayElements(bytes, NULL);

    ASSERT(alen > 0);
    ab.Write(ba, (size_t)alen);

    _env->ReleaseByteArrayElements(bytes, ba, 0);
    return true;
}


/**
 * @param _env
 * @param jstr
 * @return
 */
wchar_t* JNU_Jstring2Wchar(
    JNIEnv* _env,
    const jstring jstr) {
    ASSERT(_env != NULL);

    if (jstr == NULL)
        return NULL;

    wchar_t* wchar = (wchar_t*)_env->GetStringChars(jstr, NULL);

    return wchar;
}

void JNU_FreeWchar(JNIEnv* _env, jstring str, wchar_t* wchar) {
    ASSERT(_env != NULL);
    ASSERT(wchar != NULL);
    ASSERT(str != NULL);

    _env->ReleaseStringChars(str, (jchar*) wchar);
}

/**
 * remember to free the jstr using DeleteLocalRef
 * @param _env
 * @param wstr
 * @param jstr
 * @return
 */
jstring JNU_Wstring2Jstring(
    JNIEnv* _env,
    const std::wstring& wstr) {
    ASSERT(_env != NULL);
    ASSERT(wstr.size() != 0);

    jstring jstr = _env->NewString((jchar*)wstr.c_str(), (jsize)wstr.length());
    return jstr;
}

/**
 *  remember to free the jstr using DeleteLocalRef
 * @param _env
 * @param wchar
 * @param jstr
 * @return
 */
jstring JNU_Wchar2JString(JNIEnv* _env, wchar_t* wchar) {
    ASSERT(_env != NULL);
    ASSERT(wcslen(wchar) != 0);

    jstring jstr = _env->NewString((jchar*)wchar, (jsize)wcslen(wchar));

    return jstr;
}

// char* to jstring
jstring JNU_Chars2Jstring(JNIEnv* _env, const char* pat) {
    VarCache* cacheInastance = VarCache::Singleton();
    jclass str_class = cacheInastance->GetClass(_env, "java/lang/String");
    jmethodID ctorID = cacheInastance->GetMethodId(_env, str_class, "<init>", "([BLjava/lang/String;)V");

    jbyteArray bytes;

    if (pat != NULL) {
        bytes = _env->NewByteArray((jsize)strlen(pat));
        _env->SetByteArrayRegion(bytes, 0, (jsize)strlen(pat), (jbyte*) pat);
    } else {
        bytes = _env->NewByteArray(1);
        char ch[1] =
        { 0 };
        _env->SetByteArrayRegion(bytes, 0, 1, (jbyte*) ch);
    }

    jstring encoding = _env->NewStringUTF("utf-8");

    jstring jstr = (jstring) _env->NewObject(str_class, ctorID, bytes, encoding);
    _env->DeleteLocalRef(bytes);
    _env->DeleteLocalRef(encoding);

    return jstr;
}

void JNU_FreeJstring(JNIEnv* _env, jstring str) {
    ASSERT(_env != NULL);

    _env->DeleteLocalRef(str);
}
