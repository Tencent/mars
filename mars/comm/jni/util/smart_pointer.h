//
// Created by Cpan on 2022/4/6.
// https://www.studiofuga.com/2017/03/10/a-c-smart-pointer-wrapper-for-use-with-jni/
//

#ifndef MMNET_SMART_POINTER_H
#define MMNET_SMART_POINTER_H

#include <memory>
template <typename T>

class SmartPointerWrapper {
    std::shared_ptr<T> mObject;
public:
    template <typename ...ARGS>
    explicit SmartPointerWrapper(ARGS... a) {
        mObject = std::make_shared<T>(a...);
    }

    explicit SmartPointerWrapper (std::shared_ptr<T> obj) {
        mObject = obj;
    }

    virtual ~SmartPointerWrapper() noexcept = default;

    void instantiate(JNIEnv *env, jobject instance) {
        setHandle<SmartPointerWrapper>(env, instance, this);
    }

    void instantiate(JNIEnv *env, jobject instance, const char* field) {
        setHandle<SmartPointerWrapper>(env, instance, this, field);
    }

    jlong instance() const {
        return reinterpret_cast<jlong>(this);
    }

    std::shared_ptr<T> get() const {
        return mObject;
    }

    static std::shared_ptr<T> object(JNIEnv *env, jobject instance) {
        return get(env, instance)->get();
    }

    static SmartPointerWrapper<T> *get(JNIEnv *env, jobject instance) {
        return getHandle<SmartPointerWrapper<T>>(env, instance);
    }

    static SmartPointerWrapper<T> *get(JNIEnv *env, jobject instance, const char* field) {
        return getHandle<SmartPointerWrapper<T>>(env, instance, field);
    }

    static void dispose(JNIEnv *env, jobject instance) {
        auto obj = get(env, instance);
        if (obj) delete obj;
        setHandle<SmartPointerWrapper>(env, instance, nullptr);
    }

    static void dispose(JNIEnv *env, jobject instance, const char* field) {
        auto obj = get(env, instance, field);
        if (obj) delete obj;
    }
};

#endif //MMNET_SMART_POINTER_H

