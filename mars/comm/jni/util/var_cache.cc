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
 *    author  : yanguoyue
 */

#include "var_cache.h"
#include "scope_jenv.h"
#include <stdlib.h>
#include <android/log.h>

#include "assert/__assert.h"
#include "thread/lock.h"

template <typename T> T& getListByClass(JNIEnv* _env, const jclass& _clz, std::map<jclass, T>& _map) {
    for (typename std::map<jclass, T>::iterator iter = _map.begin(); iter != _map.end(); ++iter) {
        if (_env->IsSameObject(_clz, (*iter).first))
            return (*iter).second;
    }

    jclass gloableClz = (jclass)_env->NewGlobalRef(_clz);
    std::pair<typename std::map<jclass, T>::iterator, bool> retPair = _map.insert(std::pair<jclass, T>(gloableClz, T()));
    ASSERT(retPair.second);

    return retPair.first->second;
}


struct method_struct {
    std::string methodName;
    std::string signature;
    jmethodID mid;
};

struct field_struct {
    std::string fieldName;
    std::string signature;
    jfieldID fid;
};

VarCache* VarCache::instance_ = NULL;

VarCache::VarCache() :
    vm_(NULL) {
}

VarCache::~VarCache() {
    ScopeJEnv scopeJEnv(vm_);
    JNIEnv* env = scopeJEnv.GetEnv();


    ScopedSpinLock lock(class_map_lock_);

    for (std::map<std::string, jclass>::iterator iter = class_map_.begin();
            iter != class_map_.end(); ++iter) {
        env->DeleteGlobalRef(iter->second);
    }
}

VarCache* VarCache::Singleton() {
    if (instance_ == NULL) {
        instance_ = new VarCache();
    }

    return instance_;
}

void VarCache::Release() {
    if (instance_ != NULL) {
        delete instance_;
        instance_ = NULL;
    }
}

JavaVM* VarCache::GetJvm() {
    return vm_;
}

void VarCache::SetJvm(JavaVM* _vm) {
    ASSERT(_vm);
    vm_ = _vm;
}

/**
 * get the class, if the class does not exist in the cache,find and cache it
 * @param _class_path        find the class
 */
jclass VarCache::GetClass(JNIEnv* _env, const char* const _class_path) {
    if (_env->ExceptionOccurred()) {
        return NULL;
    }

    ASSERT(_env != NULL);
    ASSERT(_class_path != NULL);

    ScopedSpinLock lock(class_map_lock_);
    std::map<std::string, jclass>::iterator iter = class_map_.find(_class_path);

    if (iter != class_map_.end()) {
        return iter->second;
    }

    jclass clz = /*newClass(env, classPath);  // */_env->FindClass(_class_path);

    ASSERT2(clz != NULL, "classpath:%s", _class_path);

    if (clz == NULL) {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "classPath:%s", _class_path);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }

        return NULL;
    }

    jclass gloable_clz = (jclass) _env->NewGlobalRef(clz);
    _env->DeleteLocalRef(clz);

    class_map_.insert(std::pair<std::string, jclass>(_class_path, gloable_clz));

    return gloable_clz;
}

void VarCache::CacheClass(const char* const _class_path, jclass _clz) {
    ASSERT(_class_path != NULL);
    ASSERT(_clz != NULL);

    if (_class_path == NULL || _clz == NULL) {
        return;
    }

    ScopedSpinLock lock(class_map_lock_);
    std::map<std::string, jclass>::iterator iter = class_map_.find(_class_path);

    if (iter == class_map_.end()) {
        class_map_.insert(std::pair<std::string, jclass>(_class_path, _clz));
    }
}

/**
 * description: get the jmethodID, if it does not exist in the cache,find and cache it.
 * @param _clz    find the jmethodID in the class
 * @param _method_name     the name of the method
 * @param _signature        the signature of the method's parameter
 */


jmethodID VarCache::GetStaticMethodId(JNIEnv* _env, jclass _clz,
                                      const char* const _method_name, const char* const _signature) {
    if (_env->ExceptionOccurred()) {
        return NULL;
    }

    ASSERT(_env != NULL);
    ASSERT(_clz != NULL);
    ASSERT(_method_name != NULL);
    ASSERT(_signature != NULL);

    if (NULL == _clz) {
        return NULL;
    }

#ifdef USE_JNI_METHOD_CACHE
    ScopedSpinLock lock(staticMethodMapLock);
    //    std::list<method_struct>& methodStructList = staticMethodMap[clz];
    std::list<method_struct>& methodStructList = getListByClass(_env, _clz, staticMethodMap);

    for (list<method_struct>::iterator iter = methodStructList.begin();
            iter != methodStructList.end(); ++iter) {
        method_struct& mStruct = *iter;

        if (strcmp(mStruct._method_name.c_str(), _method_name) == 0
                && strcmp(mStruct._signature.c_str(), _signature) == 0) {
            return mStruct.mid;
        }
    }

#endif

    jmethodID mid = _env->GetStaticMethodID(_clz, _method_name,
                                           _signature);

    ASSERT2(mid != NULL, "NULL sig:%s, mid:%s", _signature, _method_name);

#ifdef USE_JNI_METHOD_CACHE

    if (mid != NULL) {
        method_struct methodStruct;
        methodStruct._method_name.assign(_method_name);

        methodStruct._signature.assign(_signature);
        methodStruct.mid = mid;
        methodStructList.push_back(methodStruct);
    } else
#endif
    {
        // the lib may be out of date.
        // If we GetStaticMethodID fail, throw "UnsatisfiedLinkError".
        // Then Java code catch the error, copy  the correct lib to dest path
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "method:%s, sig:%s", _method_name, _signature);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }

    return mid;
}

jmethodID VarCache::GetStaticMethodId(JNIEnv* _env, const char* const _class_path,
                                      const char* const _method_name, const char* const _signature) {
    ASSERT(_env != NULL);
    ASSERT(_class_path != NULL);
    ASSERT(_method_name != NULL);
    ASSERT(_signature != NULL);

    jclass clz = GetClass(_env, _class_path);
    return GetStaticMethodId(_env, clz, _method_name, _signature);
}

jmethodID VarCache::GetMethodId(JNIEnv* _env, jclass _clz,
                                const char* const _method_name, const char* const _signature) {
    if (_env->ExceptionOccurred()) {
        return NULL;
    }

    ASSERT(_env != NULL);
    ASSERT(_clz != NULL);
    ASSERT(_method_name != NULL);
    ASSERT(_signature != NULL);

    if (NULL == _clz) {
        return NULL;
    }

#ifdef USE_JNI_METHOD_CACHE
    ScopedSpinLock lock(methodMapLock);
    //    std::list<method_struct>& methodStructList = methodMap[clz];
    std::list<method_struct>& methodStructList = getListByClass(_env, _clz, methodMap);

    for (list<method_struct>::iterator iter = methodStructList.begin();
            iter != methodStructList.end(); ++iter) {
        method_struct& mStruct = *iter;

        if (strcmp(mStruct._method_name.c_str(), _method_name) == 0
                && strcmp(mStruct._signature.c_str(), _signature) == 0) {
            return mStruct.mid;
        }
    }

#endif

    jmethodID mid = _env->GetMethodID(_clz, _method_name,
                                     _signature);

    ASSERT2(mid != NULL, "method:%s, sig:%s", _method_name, _signature);

#ifdef USE_JNI_METHOD_CACHE

    if (mid != NULL) {
        method_struct methodStruct;
        methodStruct._method_name.assign(_method_name);
        methodStruct._signature.assign(_signature);
        methodStruct.mid = mid;
        methodStructList.push_back(methodStruct);
    } else
#endif
    {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "method:%s, sig:%s", _method_name, _signature);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }


    return mid;
}

jmethodID VarCache::GetMethodId(JNIEnv* _env, const char* const _class_path,
                                const char* const _method_name, const char* const _signature) {
    ASSERT(_env != NULL);
    ASSERT(_class_path != NULL);
    ASSERT(_method_name != NULL);
    ASSERT(_signature != NULL);

    jclass clz = GetClass(_env, _class_path);

    return GetMethodId(_env, clz, _method_name, _signature);
}


jfieldID VarCache::GetStaticFieldId(JNIEnv* _env, const char* const _class_path, const char* const _name, const char* const _sig) {
    ASSERT(NULL != _env);
    ASSERT(NULL != _class_path);
    ASSERT(NULL != _name);
    ASSERT(NULL != _sig);

    jclass clazz = GetClass(_env, _class_path);

    if (NULL == clazz) {
        return NULL;
    }

    return GetStaticFieldId(_env, clazz, _name, _sig);
}

jfieldID VarCache::GetStaticFieldId(JNIEnv* _env, jclass _clazz, const char* const _name, const char* const _sig) {
    if (_env->ExceptionOccurred()) {
        return NULL;
    }

    ASSERT(NULL != _env);
    ASSERT(NULL != _clazz);
    ASSERT(NULL != _name);
    ASSERT(NULL != _sig);

    if (NULL == _clazz) {
        return NULL;
    }

#ifdef USE_JNI_FIELD_CACHE
    ScopedSpinLock lock(fieldMapLock);
    std::list<field_struct>& fieldStructList = getListByClass(_env, _clazz, fieldMap);

    for (list<field_struct>::iterator iter = fieldStructList.begin();
            iter != fieldStructList.end(); ++iter) {
        field_struct& fStruct = *iter;

        if (strcmp(fStruct.fieldName.c_str(), _name) == 0
                && strcmp(fStruct.signature.c_str(), _sig) == 0) {
            return fStruct.fid;
        }
    }

#endif

    jfieldID fid = _env->GetStaticFieldID(_clazz, _name, _sig);
    ASSERT2(fid != NULL, "field:%s, sig:%s", _name, _sig);

#ifdef USE_JNI_FIELD_CACHE

    if (NULL != fid) {
        field_struct fieldStruct;
        fieldStruct.fieldName = _name;
        fieldStruct.signature = _sig;
        fieldStruct.fid = fid;
        fieldStructList.push_back(fieldStruct);
    } else
#endif
    {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "static field:%s, sig:%s", _name, _sig);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }

    return fid;
}


/**
 * description: get the jfieldID, if it does not exist in the cache,find and cache it.
 * @param _clz    find the jfieldID in the class
 * @param _field_name     the name of the field
 * @param _signature        the signature of the filed's type
 */
jfieldID VarCache::GetFieldId(JNIEnv* _env, jclass _clz, const char* const _field_name,
                              const char* const _signature) {
    if (_env->ExceptionOccurred()) {
        return NULL;
    }

    ASSERT(_env != NULL);
    ASSERT(_clz != NULL);
    ASSERT(_field_name != NULL);
    ASSERT(_signature != NULL);

    if (NULL == _clz) {
        return NULL;
    }

#ifdef USE_JNI_FIELD_CACHE
    ScopedSpinLock lock(fieldMapLock);
    //    std::list<field_struct>& fieldStructList = fieldMap[clz];
    std::list<field_struct>& fieldStructList = getListByClass(_env, _clz, fieldMap);

    for (list<field_struct>::iterator iter = fieldStructList.begin();
            iter != fieldStructList.end(); ++iter) {
        field_struct& fStruct = *iter;

        if (strcmp(fStruct._field_name.c_str(), _field_name) == 0
                && strcmp(fStruct._signature.c_str(), _signature) == 0) {
            return fStruct.fid;
        }
    }

#endif

    jfieldID fid = _env->GetFieldID(_clz, _field_name, _signature);

    ASSERT2(fid != NULL, "field:%s, sig:%s", _field_name, _signature);

#ifdef USE_JNI_FIELD_CACHE

    if (fid != NULL) {
        field_struct fieldStruct;
        fieldStruct._field_name.assign(_field_name);
        fieldStruct._signature.assign(_signature);
        fieldStruct.fid = fid;
        fieldStructList.push_back(fieldStruct);
    } else
#endif
    {
        if (_env->ExceptionOccurred()) {
            _env->ExceptionClear();
            char err_msg[512] = {0};
            snprintf(err_msg, sizeof(err_msg), "field:%s, sig:%s", _field_name, _signature);
            _env->ThrowNew(_env->FindClass("java/lang/UnsatisfiedLinkError"), err_msg);
        }
    }

    return fid;
}

jfieldID VarCache::GetFieldId(JNIEnv* _env, const char* const _class_path,
                              const char* const _field_name, const char* const _signature) {
    ASSERT(_env != NULL);
    ASSERT(_class_path != NULL);
    ASSERT(_field_name != NULL);
    ASSERT(_signature != NULL);

    jclass clz = GetClass(_env, _class_path);

    return GetFieldId(_env, clz, _field_name, _signature);
}


/***************************************************************************************/
#include <set>

static std::set<std::string>& __GetClassNameSet() {
    static std::set<std::string> class_name_set;
    return class_name_set;
}

bool LoadClass(JNIEnv* env) {
    ASSERT(NULL != env);
    std::set<std::string>&  class_name_set = __GetClassNameSet();

    for (std::set<std::string>::iterator it = class_name_set.begin(); it != class_name_set.end(); ++it) {
        jclass clz = VarCache::Singleton()->GetClass(env, (*it).c_str());

        if (NULL == clz) {
        	class_name_set.clear();
            return false;
        }
    }

    class_name_set.clear();
    return true;
}

bool AddClass(const char* const _class_path) {
    ASSERT(_class_path != NULL);
    std::set<std::string>&  classNameSet = __GetClassNameSet();
    return classNameSet.insert(_class_path).second;
}

/*********************************************************************************************/

static std::set<JniMethodInfo>& __GetStaticMethodInfoSet() {
    static std::set<JniMethodInfo> method_info_set;
    return method_info_set;
}

static std::set<JniMethodInfo>& __GetMethodInfoSet() {
    static std::set<JniMethodInfo> methodInfoSet;
    return methodInfoSet;
}
bool LoadStaticMethod(JNIEnv* _env) {
    ASSERT(NULL != _env);

    std::set<JniMethodInfo> methoNameSet = __GetStaticMethodInfoSet();

    for (std::set<JniMethodInfo>::iterator iter = methoNameSet.begin(); iter != methoNameSet.end(); ++iter) {
        jmethodID mid = VarCache::Singleton()->GetStaticMethodId(_env, (*iter).classname.c_str(), (*iter).methodname.c_str(), (*iter).methodsig.c_str());

        if (NULL == mid) {
            return false;
        }
    }

    return true;
}

bool LoadMethod(JNIEnv* env) {
    ASSERT(NULL != env);

    std::set<JniMethodInfo> methoNameSet = __GetMethodInfoSet();

    for (std::set<JniMethodInfo>::iterator iter = methoNameSet.begin(); iter != methoNameSet.end(); ++iter) {
        jmethodID mid = VarCache::Singleton()->GetMethodId(env, (*iter).classname.c_str(), (*iter).methodname.c_str(), (*iter).methodsig.c_str());

        if (NULL == mid) {
            return false;
        }
    }

    return true;
}


bool AddStaticMethod(const char* const _classname, const char* const _methodname, const char* const _methodsig) {
    ASSERT(_classname != NULL);
    ASSERT(_methodname != NULL);
    ASSERT(_methodsig != NULL);

    std::set<JniMethodInfo>&  method_name_set = __GetStaticMethodInfoSet();

    return method_name_set.insert(JniMethodInfo(_classname, _methodname, _methodsig)).second;
}

bool AddMethod(const char* const _classname, const char* const _methodname, const char* const _methodsig) {
    ASSERT(_classname != NULL);
    ASSERT(_methodname != NULL);
    ASSERT(_methodsig != NULL);

    std::set<JniMethodInfo>&  methodNameSet = __GetMethodInfoSet();

    return methodNameSet.insert(JniMethodInfo(_classname, _methodname, _methodsig)).second;
}
bool AddNativeMethod(JNINativeMethod* methods, int num) {
    return true;
}
