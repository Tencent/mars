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

#ifndef _VAR_CACHE_H_
#define _VAR_CACHE_H_

#include <jni.h>
#include <map>
#include <list>
#include <string>

#include "comm/thread/spinlock.h"

struct method_struct;
struct field_struct;

struct JniMethodInfo {
    JniMethodInfo(const std::string& _classname, const std::string& _methodname, const std::string& _methodsig)
        : classname(_classname), methodname(_methodname), methodsig(_methodsig) {}

    bool operator <(const JniMethodInfo& _info) const {
        if (classname < _info.classname) {
            return true;
        }

        if (classname == _info.classname && methodname < _info.methodname) {
            return true;
        }

        if (classname == _info.classname
                && methodname == _info.methodname
                && methodsig < _info.methodsig) {
            return true;
        }

        return false;
    }

    std::string classname;
    std::string methodname;
    std::string methodsig;
};

class VarCache {
  public:
    static VarCache* Singleton();
    static void Release();
    ~VarCache();

    JavaVM* GetJvm();
    void SetJvm(JavaVM* vm);

    jclass GetClass(JNIEnv*, const char* const);
    void CacheClass(const char* const, jclass);

    jmethodID GetStaticMethodId(JNIEnv*, const char* const, const char* const, const char* const);
    jmethodID GetStaticMethodId(JNIEnv*, jclass, const char* const, const char* const);

    jmethodID GetMethodId(JNIEnv*, const char* const, const char* const, const char* const);
    jmethodID GetMethodId(JNIEnv*, jclass, const char* const, const char* const);

    jfieldID GetStaticFieldId(JNIEnv*, const char* const, const char* const, const char* const);
    jfieldID GetStaticFieldId(JNIEnv*, jclass, const char* const, const char* const);

    jfieldID GetFieldId(JNIEnv*, const char* const, const char* const, const char* const);
    jfieldID GetFieldId(JNIEnv*, jclass, const char* const, const char* const);

  private:
    VarCache();

  private:
    static VarCache* instance_;

    JavaVM* vm_;

    std::map<std::string, jclass> class_map_;
    std::map<jclass, std::list<method_struct> > static_method_map_;
    std::map<jclass, std::list<method_struct> > method_map_;
    std::map<jclass, std::list<field_struct> > field_map_;

    SpinLock class_map_lock_;
    SpinLock static_method_map_lock_;
    SpinLock method_map_lock_;
    SpinLock field_map_lock_;
};

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

bool LoadClass(JNIEnv* env);
bool AddClass(const char* const classPath);

#define DEFINE_FIND_CLASS(classname, classpath) \
    VARIABLE_IS_NOT_USED static bool b_##classname = AddClass(classpath);\
    VARIABLE_IS_NOT_USED static const char* classname = classpath;


bool LoadStaticMethod(JNIEnv* env);
bool AddStaticMethod(const char* const _classname, const char* const _methodname, const char* const _methodsig);

bool LoadMethod(JNIEnv* env);
bool AddMethod(const char* const _classname, const char* const _methodname, const char* const _methodsig);

#define DEFINE_FIND_STATIC_METHOD(methodid, classname, methodname, methodsig) \
    VARIABLE_IS_NOT_USED static bool b_static_##methodid = AddStaticMethod(classname, methodname, methodsig);\
    VARIABLE_IS_NOT_USED const static JniMethodInfo methodid = JniMethodInfo(classname, methodname, methodsig);

#define DEFINE_FIND_METHOD(methodid, classname, methodname, methodsig) \
    VARIABLE_IS_NOT_USED static bool b_##methodid = AddMethod(classname, methodname, methodsig);\
    VARIABLE_IS_NOT_USED const static JniMethodInfo methodid = JniMethodInfo(classname, methodname, methodsig);

#endif

