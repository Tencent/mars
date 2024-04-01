//
// Created by yong zhang on 2021/7/21.
//

#ifndef ILINKTEST_JNI_HANDLE_H
#define ILINKTEST_JNI_HANDLE_H
#include <jni.h>
namespace jnicat {
jfieldID inline getHandleField(JNIEnv* env, jobject obj) {
    jclass c = env->GetObjectClass(obj);
    // J is the type signature for long:
    return env->GetFieldID(c, "nativeHandle", "J");
}

jfieldID inline getHandleField(JNIEnv* env, jobject obj, const char* field) {
    jclass c = env->GetObjectClass(obj);
    return env->GetFieldID(c, field, "J");
}

template <typename T>
T* getHandle(JNIEnv* env, jobject obj) {
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T*>(handle);
}

template <typename T>
T* getHandle(JNIEnv* env, jobject obj, const char* field) {
    jlong handle = env->GetLongField(obj, getHandleField(env, obj, field));
    return reinterpret_cast<T*>(handle);
}

template <typename T>
void setHandle(JNIEnv* env, jobject obj, T* t) {
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}

template <typename T>
void setHandle(JNIEnv* env, jobject obj, T* t, const char* field) {
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj, field), handle);
}
}  // namespace jnicat
#endif  // ILINKTEST_JNI_HANDLE_H
