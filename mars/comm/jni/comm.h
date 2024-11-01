//
// Created by Cpan on 2022/9/22.
//

#ifndef MMNET_COMM_H
#define MMNET_COMM_H
#include <jni.h>

#include <cstdlib>
#include <cstring>

class ByteArrayFromJava {
 private:
    unsigned char* ptr;
    unsigned int len;
    jbyteArray jData;
    jclass cls;
    JNIEnv* mEnv;

 public:
    ~ByteArrayFromJava() {
        if (ptr) {
            delete[] ptr;
            ptr = NULL;
        }
        if (mEnv) {
            if (cls) {
                mEnv->DeleteLocalRef(cls);
            }
            if (jData) {
                mEnv->DeleteLocalRef(jData);
            }
        }
        len = 0;
    }

    ByteArrayFromJava(JNIEnv* env, jobject obj, const char* name) {
        ptr = NULL;
        len = 0;
        jData = NULL;
        mEnv = env;
        cls = NULL;
        if (mEnv) {
            cls = env->GetObjectClass(obj);
            if (cls) {
                jfieldID fid = env->GetFieldID(cls, name, "[B");
                jData = (jbyteArray)(env->GetObjectField(obj, fid));
                if (jData) {
                    len = env->GetArrayLength(jData);
                    if (len > 0) {
                        jbyte* ba = env->GetByteArrayElements(jData, JNI_FALSE);
                        ptr = new unsigned char[len];
                        memcpy(ptr, ba, len);
                        env->ReleaseByteArrayElements(jData, ba, 0);
                    }
                }
            }
        }
    }

    unsigned char* GetPtr() {
        return ptr;
    }
    unsigned int GetLen() {
        return len;
    }
};

class IntArrayFromJava {
 private:
    int* ptr;
    unsigned int count;
    jintArray jData;
    jclass cls;
    JNIEnv* mEnv;

 public:
    ~IntArrayFromJava() {
        if (ptr) {
            delete[] ptr;
            ptr = NULL;
        }
        if (mEnv) {
            if (cls) {
                mEnv->DeleteLocalRef(cls);
            }
            if (jData) {
                mEnv->DeleteLocalRef(jData);
            }
        }
        count = 0;
    }

    IntArrayFromJava(JNIEnv* env, jobject obj, const char* name) {
        ptr = NULL;
        count = 0;
        jData = NULL;
        mEnv = env;
        cls = NULL;
        if (mEnv) {
            cls = env->GetObjectClass(obj);
            if (cls) {
                jfieldID fid = env->GetFieldID(cls, name, "[I");
                jData = (jintArray)(env->GetObjectField(obj, fid));
                if (jData) {
                    count = env->GetArrayLength(jData);
                    if (count > 0) {
                        jint* ba = env->GetIntArrayElements(jData, JNI_FALSE);
                        ptr = new int[count];
                        memcpy(ptr, ba, count * sizeof(int));
                        env->ReleaseIntArrayElements(jData, ba, 0);
                    }
                }
            }
        }
    }

    int* GetPtr() {
        return ptr;
    }
    unsigned int GetCount() {
        return count;
    }
};

#endif  // MMNET_COMM_H
