//
// Created by PeterFan on 2018/4/19.
//

#include "jnicat_core.h"

#include <android/log.h>

#define xdebug(args...) __android_log_print(ANDROID_LOG_DEBUG, "jnicat", args)
#define xerror(args...) __android_log_print(ANDROID_LOG_ERROR, "jnicat", args)

namespace jnicat {

const char* version() {
    return "1.2.3";
}

std::string jstring_to_string(JNIEnv* env, jstring jstr) {
    std::string str;
    if (jstr == nullptr) {
        return str;
    }
    auto cls = jcache::shared()->get_class(env, "java/lang/String");
    auto mid = jcache::shared()->get_methodid(env, cls, "getBytes", "(Ljava/lang/String;)[B");
    auto encoding = env->NewStringUTF("utf-8");
    auto bytes = (jbyteArray)env->CallObjectMethod(jstr, mid, encoding);
    auto len = env->GetArrayLength(bytes);
    if (len > 0) {
        jbyte* cstr = env->GetByteArrayElements(bytes, 0);
        str.assign((const char*)cstr, len);
        env->ReleaseByteArrayElements(bytes, cstr, JNI_ABORT);
    }
    env->DeleteLocalRef(encoding);
    env->DeleteLocalRef(bytes);
    return str;
}

jstring cstr_to_jstring(JNIEnv* env, const char* cstr) {
    auto cls = jcache::shared()->get_class(env, "java/lang/String");
    auto ctor = jcache::shared()->get_methodid(env, cls, "<init>", "([BLjava/lang/String;)V");
    jbyteArray bytes;
    if (cstr != nullptr) {
        auto len = strlen(cstr);
        bytes = env->NewByteArray(len);
        env->SetByteArrayRegion(bytes, 0, len, (const jbyte*)cstr);
    } else {
        char null_str[1] = {0};
        bytes = env->NewByteArray(1);
        env->SetByteArrayRegion(bytes, 0, 1, (const jbyte*)null_str);
    }

    auto encoding = env->NewStringUTF("utf-8");
    auto jstr = (jstring)env->NewObject(cls, ctor, bytes, encoding);
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(encoding);
    return jstr;
}

jbyteArray string_to_jbytearray(JNIEnv* env, const std::string& str) {
    return NewArray(env, (jsize)str.size(), (const jbyte*)str.data());
}

std::string jbytearray_to_string(JNIEnv* env, jbyteArray array) {
    return jarray_to_vector(env, array);
}

std::string jobject_to_pbstring(JNIEnv* env, const jobject obj, const std::string& sig) {
    jclass pbClass = jcache::shared()->get_class(env, sig.c_str());
    jmethodID toByteArray = env->GetMethodID(pbClass, "toByteArray", std::string("()[B").c_str());
    jbyteArray pb_byte_array = (jbyteArray)env->CallObjectMethod(obj, toByteArray);
    return jni_j2c_cast(env, pb_byte_array);
}

std::vector<std::string> jobjectarray_to_pbstringvector(JNIEnv* env, const jobjectArray array, const std::string& sig) {
    jclass pbClass = jcache::shared()->get_class(env, sig.c_str());
    jmethodID toByteArray = env->GetMethodID(pbClass, "toByteArray", std::string("()[B").c_str());
    auto len = env->GetArrayLength(array);
    std::vector<std::string> vec;
    vec.reserve(len);
    for (auto i = 0; i < len; ++i) {
        auto jobj = (jobject)env->GetObjectArrayElement(array, i);
        jbyteArray pb_byte_array = (jbyteArray)env->CallObjectMethod(jobj, toByteArray);
        vec.push_back(jni_j2c_cast(env, pb_byte_array));
        env->DeleteLocalRef(pb_byte_array);
    }
    return vec;
}

jobjectArray cpbarray_to_jobjectarray(JNIEnv* env, const std::vector<std::string>& vec, const std::string& sig) {
    jclass cls = jcache::shared()->get_class(env, sig.c_str());
    jmethodID mid = env->GetStaticMethodID(cls, "parseFrom", ("([B)L" + sig + ";").c_str());
    auto array = (jobjectArray)env->NewObjectArray(vec.size(), cls, nullptr);
    for (unsigned long i = 0; i < vec.size(); ++i) {
        jobject obj = (jobject)env->CallStaticObjectMethod(cls, mid, jni_c2j_cast<jbyteArray>(env, vec[i]));
        env->SetObjectArrayElement(array, i, obj);
    }
    return array;
}

std::vector<std::string> jobjectarray_to_stringvector(JNIEnv* env, jobjectArray array) {
    auto len = env->GetArrayLength(array);
    std::vector<std::string> vec;
    vec.reserve(len);
    for (auto i = 0; i < len; ++i) {
        auto jstr = (jstring)env->GetObjectArrayElement(array, i);
        vec.push_back(jni_j2c_cast(env, jstr));
        env->DeleteLocalRef(jstr);
    }
    return vec;
}

std::vector<std::vector<std::string>> jobjectarray_to_stringvector2(JNIEnv* env, jobjectArray array) {
    auto len = env->GetArrayLength(array);
    std::vector<std::vector<std::string>> vec;
    vec.reserve(len);
    for (auto i = 0; i < len; ++i) {
        auto arr = (jobjectArray)env->GetObjectArrayElement(array, i);
        vec.push_back(jobjectarray_to_stringvector(env, arr));
        env->DeleteLocalRef(arr);
    }
    return vec;
}

jobjectArray stringvector_to_jobjectarray(JNIEnv* env, const std::vector<std::string>& vec) {
    auto cls = jcache::shared()->get_class(env, "java/lang/String");
    auto array = env->NewObjectArray(vec.size(), cls, nullptr);
    for (unsigned long i = 0; i < vec.size(); ++i) {
        env->SetObjectArrayElement(array, i, env->NewStringUTF(vec[i].c_str()));
    }
    return array;
}

jobjectArray stringvector2_to_jobjectarray(JNIEnv* env, const std::vector<std::vector<std::string>>& vec) {
    auto cls = jcache::shared()->get_class(env, "[Ljava/lang/String;");
    auto array = env->NewObjectArray(vec.size(), cls, nullptr);
    for (unsigned long i = 0; i < vec.size(); ++i) {
        jobject value = stringvector_to_jobjectarray(env, vec[i]);
        env->SetObjectArrayElement(array, i, value);
    }
    return array;
}

jobject stringvector_to_arraylist(JNIEnv* env, const std::vector<std::string>& vec) {
    auto cls = jcache::shared()->get_class(env, "java/util/ArrayList");
    auto ctor_method = jcache::shared()->get_methodid(env, cls, "<init>", "()V");
    auto add_method = jcache::shared()->get_methodid(env, cls, "add", "(Ljava/lang/Object;)Z");
    auto obj = env->NewObject(cls, ctor_method);
    for (const auto& str : vec) {
        auto array = string_to_jbytearray(env, str);
        env->CallBooleanMethod(obj, add_method, array);
        env->DeleteLocalRef(array);
    }
    return obj;
}

std::vector<std::string> arraylist_to_stringvector(JNIEnv* env, jobject obj) {
    auto cls = jcache::shared()->get_class(env, "java/util/ArrayList");
    auto toarray_method = jcache::shared()->get_methodid(env, cls, "toArray", "()[Ljava/lang/Object;");
    jobjectArray array = (jobjectArray)env->CallObjectMethod(obj, toarray_method);
    auto len = env->GetArrayLength(array);
    std::vector<std::string> vec;
    vec.reserve(len);
    for (auto i = 0; i < len; ++i) {
        auto arr = (jbyteArray)env->GetObjectArrayElement(array, i);
        vec.push_back(jbytearray_to_string(env, arr));
        env->DeleteLocalRef(arr);
    }
    return vec;
}

jbooleanArray NewArray(JNIEnv* env, jsize len, const jboolean* buf) {
    auto array = env->NewBooleanArray(len);
    env->SetBooleanArrayRegion(array, 0, len, buf);
    return array;
}

jbyteArray NewArray(JNIEnv* env, jsize len, const jbyte* buf) {
    auto array = env->NewByteArray(len);
    env->SetByteArrayRegion(array, 0, len, buf);
    return array;
}

jcharArray NewArray(JNIEnv* env, jsize len, const jchar* buf) {
    auto array = env->NewCharArray(len);
    env->SetCharArrayRegion(array, 0, len, buf);
    return array;
}

jshortArray NewArray(JNIEnv* env, jsize len, const jshort* buf) {
    auto array = env->NewShortArray(len);
    env->SetShortArrayRegion(array, 0, len, buf);
    return array;
}

jintArray NewArray(JNIEnv* env, jsize len, const jint* buf) {
    auto array = env->NewIntArray(len);
    env->SetIntArrayRegion(array, 0, len, buf);
    return array;
}

jlongArray NewArray(JNIEnv* env, jsize len, const jlong* buf) {
    auto array = env->NewLongArray(len);
    env->SetLongArrayRegion(array, 0, len, buf);
    return array;
}

jfloatArray NewArray(JNIEnv* env, jsize len, const jfloat* buf) {
    auto array = env->NewFloatArray(len);
    env->SetFloatArrayRegion(array, 0, len, buf);
    return array;
}

jdoubleArray NewArray(JNIEnv* env, jsize len, const jdouble* buf) {
    auto array = env->NewDoubleArray(len);
    env->SetDoubleArrayRegion(array, 0, len, buf);
    return array;
}

static void __cache_all_cacheitem(JNIEnv* env);
static void __register_all_jnimethod(JNIEnv* env);

static void try_detach_current_thread() {
    auto jc = jcache::shared();
    if (jc->is_auto_detach() && jc->java_vm() != nullptr) {
        xdebug("auto detach enabled, call DetachCurrentThread()");
        jc->java_vm()->DetachCurrentThread();
    }
}

static void enable_detach_current_thread() {
    static thread_local struct OnExit {
        ~OnExit() {
            xdebug("try_detach_current_thread: thread_local");
            try_detach_current_thread();
        }
    } dummy;
};

jcache* jcache::instance_ = nullptr;

jcache::jcache() : vm_(nullptr), is_auto_detach_(true), exception_handler_(nullptr) {
}

jcache::~jcache() {
    __clear_cache();
}

jcache* jcache::shared() {
    if (instance_ == nullptr) {
        instance_ = new jcache();
    }
    return instance_;
}

void jcache::release() {
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}

int jcache::init(JavaVM* vm) {
    jnienv_ptr env(vm);
    if (env.get() == nullptr) {
        return JNI_ERR;
    }

    vm_ = vm;
    __cache_class(env);
    __cache_method(env);
    __register_all_jnimethod(env);
    return JNI_VERSION_1_6;
}

JavaVM* jcache::java_vm() const {
    return vm_;
}

void jcache::set_auto_detach(bool is_auto_detach) {
    is_auto_detach_ = is_auto_detach;
}

bool jcache::is_auto_detach() const {
    return is_auto_detach_;
}

void jcache::set_exception_handler(C2JavaExceptionHandler handler) {
    exception_handler_ = handler;
}

void jcache::__cache_class(JNIEnv* env) {
    for (auto classname : kJarrayClassnameMap) {
        get_class(env, classname);
    }

    for (auto classname : kJwrapperClassnameMap) {
        get_class(env, classname);
    }

    get_class(env, "java/lang/String");
    get_class(env, "[Ljava/lang/String;");
    get_class(env, "java/util/ArrayList");
}

void jcache::__cache_method(JNIEnv* env) {
    __cache_all_cacheitem(env);
}

void jcache::__clear_cache() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (vm_ == nullptr) {
        return;
    }

    jnienv_ptr env(vm_);
    for (const auto& item : classmap_) {
        env->DeleteGlobalRef(item.second);
    }
    for (const auto& item : fieldmap_) {
        env->DeleteGlobalRef(item.first);
        delete item.second;
    }
    for (const auto& item : methodmap_) {
        env->DeleteGlobalRef(item.first);
        delete item.second;
    }
    env.detach();
}

jclass jcache::get_class(JNIEnv* env, const char* name) {
    assert(env != nullptr);
    assert(name != nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    class_map_t::iterator it = classmap_.find(name);
    if (it != classmap_.end()) {
        return it->second;
    }

    auto cls = env->FindClass(name);
    if (cls == nullptr) {
        return nullptr;
    }

    auto gcls = (jclass)env->NewGlobalRef(cls);
    env->DeleteLocalRef(cls);
    classmap_.insert(std::make_pair(name, gcls));
    return gcls;
}

static std::string __name_sig_hash(const char* name, const char* sig) {
    std::string hash(name);
    hash.append(":");
    hash.append(sig);
    return hash;
}

jfieldID jcache::get_fieldid(JNIEnv* env, jobject obj, const char* fieldname, const char* signature) {
    auto cls = env->GetObjectClass(obj);
    if (cls == nullptr) {
        return nullptr;
    }
    auto fid = get_fieldid(env, cls, fieldname, signature);
    env->DeleteLocalRef(cls);
    return fid;
}

jfieldID jcache::get_static_fieldid(JNIEnv* env, jobject obj, const char* fieldname, const char* signature) {
    auto cls = env->GetObjectClass(obj);
    if (cls == nullptr) {
        return nullptr;
    }
    auto fid = get_static_fieldid(env, cls, fieldname, signature);
    env->DeleteLocalRef(cls);
    return fid;
}

jmethodID jcache::get_methodid(JNIEnv* env, jobject obj, const char* methodname, const char* signature) {
    auto cls = env->GetObjectClass(obj);
    if (cls == nullptr) {
        return nullptr;
    }
    auto mid = get_methodid(env, cls, methodname, signature);
    env->DeleteLocalRef(cls);
    return mid;
}

jmethodID jcache::get_static_methodid(JNIEnv* env, jobject obj, const char* methodname, const char* signature) {
    auto cls = env->GetObjectClass(obj);
    if (cls == nullptr) {
        return nullptr;
    }
    auto mid = get_static_methodid(env, cls, methodname, signature);
    env->DeleteLocalRef(cls);
    return mid;
}

jfieldID jcache::get_fieldid(JNIEnv* env, jclass cls, const char* fieldname, const char* signature) {
    assert(env != nullptr);
    assert(cls != nullptr);
    assert(fieldname != nullptr);
    assert(signature != nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    auto fmap = __get_field_map(env, cls);
    assert(fmap != nullptr);
    auto key = __name_sig_hash(fieldname, signature);
    auto it = fmap->find(key);
    if (it != fmap->end()) {
        return it->second;
    }

    auto fid = env->GetFieldID(cls, fieldname, signature);
    if (fid == nullptr) {
        return nullptr;
    }

    fmap->insert(std::make_pair(key, fid));
    return fid;
}

jfieldID jcache::get_static_fieldid(JNIEnv* env, jclass cls, const char* fieldname, const char* signature) {
    assert(env != nullptr);
    assert(cls != nullptr);
    assert(fieldname != nullptr);
    assert(signature != nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    auto fmap = __get_field_map(env, cls);
    assert(fmap != nullptr);
    auto key = __name_sig_hash(fieldname, signature);
    auto it = fmap->find(key);
    if (it != fmap->end()) {
        return it->second;
    }

    auto fid = env->GetStaticFieldID(cls, fieldname, signature);
    if (fid == nullptr) {
        return nullptr;
    }

    fmap->insert(std::make_pair(key, fid));
    return fid;
}

jmethodID jcache::get_methodid(JNIEnv* env, jclass cls, const char* methodname, const char* signature) {
    assert(env != nullptr);
    assert(cls != nullptr);
    assert(methodname != nullptr);
    assert(signature != nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    auto mmap = __get_method_map(env, cls);
    assert(mmap != nullptr);
    auto key = __name_sig_hash(methodname, signature);
    auto it = mmap->find(key);
    if (it != mmap->end()) {
        return it->second;
    }

    auto mid = env->GetMethodID(cls, methodname, signature);
    if (mid == nullptr) {
        return nullptr;
    }

    mmap->insert(std::make_pair(key, mid));
    return mid;
}

jmethodID jcache::get_static_methodid(JNIEnv* env, jclass cls, const char* methodname, const char* signature) {
    assert(env != nullptr);
    assert(cls != nullptr);
    assert(methodname != nullptr);
    assert(signature != nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    auto mmap = __get_method_map(env, cls);
    assert(mmap != nullptr);
    auto key = __name_sig_hash(methodname, signature);
    auto it = mmap->find(key);
    if (it != mmap->end()) {
        return it->second;
    }

    auto mid = env->GetStaticMethodID(cls, methodname, signature);
    if (mid == nullptr) {
        return nullptr;
    }

    mmap->insert(std::make_pair(key, mid));
    return mid;
}

jfieldID jcache::get_fieldid(JNIEnv* env, const char* classname, const char* fieldname, const char* signature) {
    auto cls = get_class(env, classname);
    if (cls == nullptr) {
        return nullptr;
    }
    return get_fieldid(env, cls, fieldname, signature);
}

jfieldID jcache::get_static_fieldid(JNIEnv* env, const char* classname, const char* fieldname, const char* signature) {
    auto cls = get_class(env, classname);
    if (cls == nullptr) {
        return nullptr;
    }
    return get_static_fieldid(env, cls, fieldname, signature);
}

jmethodID jcache::get_methodid(JNIEnv* env, const char* classname, const char* methodname, const char* signature) {
    auto cls = get_class(env, classname);
    if (cls == nullptr) {
        return nullptr;
    }
    return get_methodid(env, cls, methodname, signature);
}

jmethodID jcache::get_static_methodid(JNIEnv* env,
                                      const char* classname,
                                      const char* methodname,
                                      const char* signature) {
    auto cls = get_class(env, classname);
    if (cls == nullptr) {
        return nullptr;
    }
    return get_static_methodid(env, cls, methodname, signature);
}

struct jobject_compare_t {
 public:
    jobject_compare_t(JNIEnv* env, jobject obj) : env_(env), obj_(obj) {
    }
    bool operator()(const jcache::class_field_map_t::value_type& rhs) const {
        return env_->IsSameObject(obj_, rhs.first);
    }
    bool operator()(const jcache::class_method_map_t::value_type& rhs) const {
        return env_->IsSameObject(obj_, rhs.first);
    }

 private:
    JNIEnv* env_;
    jobject obj_;
};

jcache::field_map_t* jcache::__get_field_map(JNIEnv* env, jclass cls) {
    auto it = std::find_if(fieldmap_.begin(), fieldmap_.end(), jobject_compare_t(env, cls));
    if (it != fieldmap_.end()) {
        return it->second;
    }
    auto fmap = new field_map_t();
    auto gcls = (jclass)env->NewGlobalRef(cls);
    fieldmap_.insert(std::make_pair(gcls, fmap));
    return fmap;
}

jcache::method_map_t* jcache::__get_method_map(JNIEnv* env, jclass cls) {
    auto it = std::find_if(methodmap_.begin(), methodmap_.end(), jobject_compare_t(env, cls));
    if (it != methodmap_.end()) {
        return it->second;
    }
    auto mmap = new method_map_t();
    auto gcls = (jclass)env->NewGlobalRef(cls);
    methodmap_.insert(std::make_pair(gcls, mmap));
    return mmap;
}

static void __dump_fieldmap(jcache::field_map_t* fmap) {
    for (const auto& item : *fmap) {
        xdebug("    %s -> %p", item.first.c_str(), item.second);
    }
}

static void __dump_methodmap(jcache::method_map_t* mmap) {
    for (const auto& item : *mmap) {
        xdebug("    %s -> %p", item.first.c_str(), item.second);
    }
}

void jcache::dump() {
    std::lock_guard<std::mutex> lock(mutex_);
    xdebug("**********  class cache ********** count %d", (int)classmap_.size());
    for (const auto& item : classmap_) {
        xdebug("%s -> %p", item.first.c_str(), item.second);
    }
    xdebug("**********  field cache ********** count %d", (int)fieldmap_.size());
    for (const auto& item : fieldmap_) {
        xdebug("%p:", item.first);
        __dump_fieldmap(item.second);
    }
    xdebug("********** method cache ********** count %d", (int)methodmap_.size());
    for (const auto& item : methodmap_) {
        xdebug("%p:", item.first);
        __dump_methodmap(item.second);
    }
}

JNICAT_DEFINE_METHOD(kThrowable_toString, "java/lang/Throwable", "toString", "()Ljava/lang/String;")
std::string GetExceptionString(jthrowable e) {
    jnienv_ptr env;
    return j2c_cast(c2j_call(jstring, e, kThrowable_toString));
}

JNICAT_DEFINE_STATIC_METHOD(kLog_getStackTraceString,
                            "android/util/Log",
                            "getStackTraceString",
                            "(Ljava/lang/Throwable;)Ljava/lang/String;")
std::string GetStackTraceString(jthrowable e) {
    jnienv_ptr env;
    return j2c_cast(c2j_call_static(jstring, kLog_getStackTraceString, e));
}

void jcache::check_exception(JNIEnv* env) {
    if (env == nullptr) {
        return;
    }
    if (jthrowable e = env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        std::string stacktrace = GetStackTraceString(e);
        // no need to call DeleteLocalRef
        // env->DeleteLocalRef(e);
        if (exception_handler_ != nullptr) {
            exception_handler_(stacktrace);
        } else {
            env->FatalError(stacktrace.c_str());
        }
    }
}

jobject NewObjectV(JNIEnv* env, const char* classname, const char* methodname, const char* signature, va_list args) {
    // xdebug("NewObjectV(\"%s\", \"%s\", \"%s\")", classname, name, signature);
    auto cls = jcache::shared()->get_class(env, classname);
    auto mid = jcache::shared()->get_methodid(env, cls, methodname, signature);
    return env->NewObjectV(cls, mid, args);
}

jobject NewObject(JNIEnv* env, const char* classname, const char* methodname, const char* signature, ...) {
    va_list args;
    va_start(args, signature);
    jobject result = NewObjectV(env, classname, methodname, signature, args);
    va_end(args);
    return result;
}

jobject NewObject(JNIEnv* env, const jcacheitem* ci, ...) {
    va_list args;
    va_start(args, ci);
    jobject result = NewObjectV(env, ci->classname, ci->name, ci->signature, args);
    va_end(args);
    return result;
}

jvalue CallStaticMethodV(JNIEnv* env,
                         const char* classname,
                         const char* methodname,
                         const char* signature,
                         va_list args) {
    // xdebug("CallStaticMethodV(\"%s\", \"%s\", \"%s\")", classname, name, signature);
    jvalue result;
    memset(&result, 0, sizeof(result));
    auto cls = jcache::shared()->get_class(env, classname);
    auto mid = jcache::shared()->get_static_methodid(env, cls, methodname, signature);

    auto p = strchr(signature, ')');
    if (p == nullptr) {
        env->FatalError("illegal signature");
        return result;
    }

    ++p;
    switch (*p) {
        case 'V':
            env->CallStaticVoidMethodV(cls, mid, args);
            break;

        case '[':
        case 'L':
            result.l = env->CallStaticObjectMethodV(cls, mid, args);
            break;

        case 'Z':
            result.z = env->CallStaticBooleanMethodV(cls, mid, args);
            break;

        case 'B':
            result.b = env->CallStaticByteMethodV(cls, mid, args);
            break;

        case 'C':
            result.c = env->CallStaticCharMethodV(cls, mid, args);
            break;

        case 'S':
            result.s = env->CallStaticShortMethodV(cls, mid, args);
            break;

        case 'I':
            result.i = env->CallStaticIntMethodV(cls, mid, args);
            break;

        case 'J':
            result.j = env->CallStaticLongMethodV(cls, mid, args);
            break;

        case 'F':
            result.f = env->CallStaticFloatMethodV(cls, mid, args);
            break;

        case 'D':
            result.d = env->CallStaticDoubleMethodV(cls, mid, args);
            break;

        default:
            env->FatalError("illegal descriptor");
            break;
    }
    jcache::shared()->check_exception(env);
    return result;
}

jvalue CallStaticMethod(JNIEnv* env, const char* classname, const char* methodname, const char* signature, ...) {
    va_list args;
    va_start(args, signature);
    jvalue result = CallStaticMethodV(env, classname, methodname, signature, args);
    va_end(args);
    return result;
}

jvalue CallStaticMethod(JNIEnv* env, const jcacheitem* ci, ...) {
    va_list args;
    va_start(args, ci);
    jvalue result = CallStaticMethodV(env, ci->classname, ci->name, ci->signature, args);
    va_end(args);
    return result;
}

jvalue CallMethodV(JNIEnv* env, jobject obj, const char* methodname, const char* signature, va_list args) {
    // xdebug("CallMethodV(\"%p\", \"%s\", \"%s\")", obj, name, signature);
    jvalue result;
    memset(&result, 0, sizeof(result));
    auto mid = jcache::shared()->get_methodid(env, obj, methodname, signature);

    auto p = strchr(signature, ')');
    if (p == nullptr) {
        env->FatalError("illegal signature");
        return result;
    }

    ++p;
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
    jcache::shared()->check_exception(env);
    return result;
}

jvalue CallMethod(JNIEnv* env, jobject obj, const char* methodname, const char* signature, ...) {
    va_list args;
    va_start(args, signature);
    jvalue result = CallMethodV(env, obj, methodname, signature, args);
    va_end(args);
    return result;
}

jvalue CallMethod(JNIEnv* env, jobject obj, const jcacheitem* ci, ...) {
    va_list args;
    va_start(args, ci);
    jvalue result = CallMethodV(env, obj, ci->name, ci->signature, args);
    va_end(args);
    return result;
}

static std::vector<const jcacheitem*>& __get_all_cacheitem() {
    static std::vector<const jcacheitem*> cache;
    return cache;
}

static std::vector<const jnativeitem*>& __get_all_jnimethod() {
    static std::vector<const jnativeitem*> cache;
    return cache;
}

static void __cache_all_cacheitem(JNIEnv* env) {
    // xdebug("__cache_all_cacheitem(), size %d", __get_all_cacheitem().size());
    auto cache = __get_all_cacheitem();
    for (auto ci : cache) {
        xdebug("cacheitem(\"%s\", \"%s\", \"%s\", %d)", ci->classname, ci->name, ci->signature, ci->type);
        switch (ci->type) {
            case kCacheClass: {
                jcache::shared()->get_class(env, ci->classname);
                break;
            }
            case kCacheMethod: {
                jcache::shared()->get_methodid(env, ci->classname, ci->name, ci->signature);
                break;
            }
            case kCacheStaticMethod: {
                jcache::shared()->get_static_methodid(env, ci->classname, ci->name, ci->signature);
                break;
            }
            case kCacheField: {
                jcache::shared()->get_fieldid(env, ci->classname, ci->name, ci->signature);
                break;
            }
            case kCacheStaticField: {
                jcache::shared()->get_static_fieldid(env, ci->classname, ci->name, ci->signature);
                break;
            }
            default:
                break;
        }
    }
}

static void __register_all_jnimethod(JNIEnv* env) {
    auto cache = __get_all_jnimethod();
    for (auto ni : cache) {
        auto clz = jcache::shared()->get_class(env, ni->classname);
        jint error = env->RegisterNatives(clz, ni->methods, ni->count);
        if (error != JNI_OK) {
            xerror("RegisterNatives() failed %d: classname %s", error, ni->classname);
        }
    }
}

const jcacheitem* add_cacheitem(const jcacheitem* ci) {
    __get_all_cacheitem().push_back(ci);
    return ci;
}

const jnativeitem* add_nativeitem(const jnativeitem* ni) {
    __get_all_jnimethod().push_back(ni);
    return ni;
}

jnienv_ptr::jnienv_ptr(JavaVM* vm) : vm_(vm), env_(nullptr), attached_(false) {
    if (vm_ == nullptr) {
        vm_ = jcache::shared()->java_vm();
    }

    assert(vm_ != nullptr);

    JNIEnv* env = nullptr;
    jint error = vm_->GetEnv((void**)&env, JNI_VERSION_1_6);
    // xdebug("jnienv_ptr GetEnv %d", error);
    if (error == JNI_OK) {
        env_ = env;
        return;
    }

    error = vm_->AttachCurrentThread(&env, nullptr);
    xdebug("jnienv_ptr AttachCurrentThread() %d", error);
    if (error == JNI_OK) {
        env_ = env;
        attached_ = true;
        // NOTE:
        // using thread_local is more portable than pthread_key_xxx
        enable_detach_current_thread();
    }
}

jnienv_ptr::~jnienv_ptr() {
}

void jnienv_ptr::detach() {
    if (vm_ != nullptr && attached_) {
        vm_->DetachCurrentThread();
        attached_ = false;
    }
}

jobject jnienv_ptr::get_object_field(jobject obj, const char* fieldname, const char* signature, jobject default_value) {
    jobject value = default_value;
    jfieldID fid = jcache::shared()->get_fieldid(env_, obj, fieldname, signature);
    if (fid != nullptr) {
        GetField(env_, obj, fid, value);
    }
    return value;
}

void jnienv_ptr::set_object_field(jobject obj, const char* fieldname, const char* signature, jobject value) {
    jfieldID fid = jcache::shared()->get_fieldid(env_, obj, fieldname, signature);
    if (fid != nullptr) {
        SetField(env_, obj, fid, value);
    }
}

}  // namespace jnicat

// extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
//    return jcache::shared()->init(vm);
//}
//
// extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
//    jcache::release();
//}
