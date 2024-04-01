//
// Created by yong zhang on 2021/7/21.
//

#ifndef ILINKTEST_SMART_POINTER_H
#define ILINKTEST_SMART_POINTER_H

#include <memory>

#include "jnicat_jni_handle.h"
//#include "owl/zlog.h"

/** @brief a Wrapper for smart pointers to be used in JNI code
 *
 * **Usage**
 * Instantiation:
 * JniObjectWrapper<Object> obj = new JniObjectWrapper<Object>(arguments);
 * obj->instantiate(env,instance);
 *
 * Recovery:
 * std::shared_ptr<Object> obj = JniObjectWrapper<Object>::object(env,instance);
 *
 * or
 *
 * JniObjectWrapper<Object> wrapper = JniObjectWrapper<Object>::get(env,instance);
 * std::shared_ptr<Object> obj = wrapper->get();
 *
 * Dispose:
 * JniObjectWrapper<Object> wrapper = JniObjectWrapper<Object>::get(env,instance);
 * delete wrapper;
 *
 * or simpler
 *
 * JniObjectWrapper<Object>::dispose(env,instance);
 */
namespace jnicat {
//     template<typename T>
//     class JniObjectWrapper {
//         std::shared_ptr<T> mObject;
//     public:
//         template<typename ...ARGS>
//         explicit JniObjectWrapper(ARGS... a) {
//             mObject = std::make_shared<T>(a...);
//         }
//
//         explicit JniObjectWrapper(std::shared_ptr<T> obj) {
//             mObject = obj;
//         }
//
//         virtual ~JniObjectWrapper() {
//
//             if (mObject.get()) {
//                 zdebug("mObject not nulls %_", mObject.use_count());
//                 mObject.reset();
////                 delete mObject.get();
//             } else {
//                 mObject.reset();
//             }
//         }
//
//         void instantiate(JNIEnv *env, jobject instance) {
//             setHandle<JniObjectWrapper>(env, instance, this);
//         }
//
//         void instantiate(JNIEnv *env, jobject instance, const char *field) {
//             setHandle<JniObjectWrapper>(env, instance, this, field);
//         }
//
//         jlong instance() const {
//             return reinterpret_cast<jlong>(this);
//         }
//
//         std::shared_ptr<T> get() const {
//             return mObject;
//         }
//
//         static std::shared_ptr<T> object(JNIEnv *env, jobject instance) {
//             return get(env, instance)->get();
//         }
//
//         static JniObjectWrapper<T> *get(JNIEnv *env, jobject instance) {
//             return getHandle<JniObjectWrapper<T>>(env, instance);
//         }
//
//         static JniObjectWrapper<T> *get(JNIEnv *env, jobject instance, const char *field) {
//             return getHandle<JniObjectWrapper<T>>(env, instance, field);
//         }
//
//         static void dispose(JNIEnv *env, jobject instance) {
//             auto obj = get(env, instance);
//             if (obj) delete obj;
//             setHandle<JniObjectWrapper>(env, instance, nullptr);
//         }
//
//         static void dispose(JNIEnv *env, jobject instance, const char *field) {
//             auto obj = get(env, instance, field);
//             if (obj) delete obj;
//         }
//     };
template <typename T>
class JniObjectWrapper {
    T* mObject;

 public:
    template <typename... ARGS>
    explicit JniObjectWrapper(ARGS... a) {
        mObject = new T(a...);
    }

    explicit JniObjectWrapper(T* obj) {
        mObject = obj;
    }

    virtual ~JniObjectWrapper() {
    }

    void instantiate(JNIEnv* env, jobject instance) {
        setHandle<JniObjectWrapper>(env, instance, this);
    }

    void instantiate(JNIEnv* env, jobject instance, const char* field) {
        setHandle<JniObjectWrapper>(env, instance, this, field);
    }

    jlong instance() const {
        return reinterpret_cast<jlong>(this);
    }

    T* get() const {
        return mObject;
    }

    static T* object(JNIEnv* env, jobject instance) {
        return get(env, instance)->get();
    }

    static JniObjectWrapper<T>* get(JNIEnv* env, jobject instance) {
        return getHandle<JniObjectWrapper<T>>(env, instance);
    }

    static JniObjectWrapper<T>* get(JNIEnv* env, jobject instance, const char* field) {
        return getHandle<JniObjectWrapper<T>>(env, instance, field);
    }

    static void dispose(JNIEnv* env, jobject instance) {
        auto obj = get(env, instance);
        if (obj)
            delete obj;
        setHandle<JniObjectWrapper>(env, instance, nullptr);
    }

    static void dispose(JNIEnv* env, jobject instance, const char* field, bool disposeRealObject = false) {
        auto obj = get(env, instance, field);
        if (disposeRealObject) {
            auto real = obj->get();
            if (real)
                delete real;
        }
        if (obj)
            delete obj;
    }
};
}  // namespace jnicat
#endif  // ILINKTEST_SMART_POINTER_H
