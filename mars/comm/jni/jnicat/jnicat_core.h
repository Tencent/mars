//
// Created by PeterFan on 2018/4/19.
//

#ifndef JNIRPC_JNICAT_CORE_H
#define JNIRPC_JNICAT_CORE_H

#ifdef JNICAT_NO_OWL
#include "mars/comm/owl/mpl/typelist.h"
#else
#include "mars/comm/owl/mpl/typelist.h"
#endif

#include <jni.h>
#include <pthread.h>

#include <cassert>
#include <cstdarg>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

namespace jnicat {

const char* version();

using C2JavaExceptionHandler = void (*)(const std::string& stacktrace);

class jcache {
 public:
    typedef std::map<std::string, jclass> class_map_t;
    typedef std::map<std::string, jfieldID> field_map_t;
    typedef std::map<std::string, jmethodID> method_map_t;
    typedef std::map<jclass, field_map_t*> class_field_map_t;
    typedef std::map<jclass, method_map_t*> class_method_map_t;

 public:
    static jcache* shared();
    static void release();

    int init(JavaVM* vm);
    JavaVM* java_vm() const;
    void set_auto_detach(bool is_auto_detach);
    bool is_auto_detach() const;
    void set_exception_handler(C2JavaExceptionHandler handler);
    void check_exception(JNIEnv* env);

    jclass get_class(JNIEnv* env, const char* name);

    jfieldID get_fieldid(JNIEnv* env, jobject obj, const char* fieldname, const char* signature);
    jfieldID get_static_fieldid(JNIEnv* env, jobject obj, const char* fieldname, const char* signature);
    jmethodID get_methodid(JNIEnv* env, jobject obj, const char* methodname, const char* signature);
    jmethodID get_static_methodid(JNIEnv* env, jobject obj, const char* methodname, const char* signature);

    jfieldID get_fieldid(JNIEnv* env, jclass cls, const char* fieldname, const char* signature);
    jfieldID get_static_fieldid(JNIEnv* env, jclass cls, const char* fieldname, const char* signature);
    jmethodID get_methodid(JNIEnv* env, jclass cls, const char* methodname, const char* signature);
    jmethodID get_static_methodid(JNIEnv* env, jclass cls, const char* methodname, const char* signature);

    jfieldID get_fieldid(JNIEnv* env, const char* classname, const char* fieldname, const char* signature);
    jfieldID get_static_fieldid(JNIEnv* env, const char* classname, const char* fieldname, const char* signature);
    jmethodID get_methodid(JNIEnv* env, const char* classname, const char* methodname, const char* signature);
    jmethodID get_static_methodid(JNIEnv* env, const char* classname, const char* methodname, const char* signature);

    void dump();

    jcache(const jcache&) = delete;
    jcache& operator=(const jcache&) = delete;

 private:
    jcache();
    ~jcache();

    field_map_t* __get_field_map(JNIEnv* env, jclass cls);
    method_map_t* __get_method_map(JNIEnv* env, jclass cls);

    void __cache_class(JNIEnv* env);
    void __cache_method(JNIEnv* env);
    void __clear_cache();

 private:
    static jcache* instance_;

    JavaVM* vm_;
    class_map_t classmap_;
    class_field_map_t fieldmap_;
    class_method_map_t methodmap_;
    std::mutex mutex_;
    bool is_auto_detach_;
    C2JavaExceptionHandler exception_handler_;
};

using typemap_j2c_t = owl::make_typemap<owl::pair<jboolean, bool>,
                                        owl::pair<jbyte, int8_t>,
                                        owl::pair<jchar, uint16_t>,
                                        owl::pair<jshort, short>,
                                        owl::pair<jint, int>,
                                        owl::pair<jlong, int64_t>,
                                        owl::pair<jfloat, float>,
                                        owl::pair<jdouble, double>,
                                        // owl::pair<jobject, jobject>,
                                        // owl::pair<jobjectArray, jobjectArray>,
                                        owl::pair<jstring, std::string>,
                                        owl::pair<jbooleanArray, std::vector<uint8_t>>,
                                        owl::pair<jbyteArray, std::string>,
                                        owl::pair<jcharArray, std::vector<uint16_t>>,
                                        owl::pair<jshortArray, std::vector<short>>,
                                        owl::pair<jintArray, std::vector<int>>,
                                        owl::pair<jlongArray, std::vector<int64_t>>,
                                        owl::pair<jfloatArray, std::vector<float>>,
                                        owl::pair<jdoubleArray, std::vector<double>>>;

using typemap_j2jarray_t = owl::make_typemap<owl::pair<jboolean, jbooleanArray>,
                                             owl::pair<jbyte, jbyteArray>,
                                             owl::pair<jchar, jcharArray>,
                                             owl::pair<jshort, jshortArray>,
                                             owl::pair<jint, jintArray>,
                                             owl::pair<jlong, jlongArray>,
                                             owl::pair<jfloat, jfloatArray>,
                                             owl::pair<jdouble, jdoubleArray>>;

template <typename T>
using typemap_j2c = owl::typemap_get<typemap_j2c_t, T>;

template <typename T>
using typemap_c2j = owl::typemap_flip_get<typemap_j2c_t, T>;

template <typename T>
using typemap_jarray2j = owl::typemap_flip_get<typemap_j2jarray_t, T>;

template <typename T>
using typemap_j2jarray = owl::typemap_get<typemap_j2jarray_t, T>;

template <typename T>
struct is_j_arithmetic {
    enum { begin_index_ = owl::typemap_find<typemap_j2c_t, jboolean>::value };
    enum { end_index_ = owl::typemap_find<typemap_j2c_t, jdouble>::value };
    enum { index_ = owl::typemap_find<typemap_j2c_t, T>::value };

    enum { value = begin_index_ <= index_ <= end_index_ };
};

template <typename T>
struct is_c_arithmetic {
    enum { begin_index_ = owl::typemap_flip_find<typemap_j2c_t, bool>::value };
    enum { end_index_ = owl::typemap_flip_find<typemap_j2c_t, double>::value };
    enum { index_ = owl::typemap_flip_find<typemap_j2c_t, T>::value };

    enum { value = begin_index_ <= index_ <= end_index_ };
};

static const char* const kJshortClassnameMap[] = {"Z", "B", "C", "S", "I", "J", "F", "D"};

static const char* const kJarrayClassnameMap[] = {"[Z", "[B", "[C", "[S", "[I", "[J", "[F", "[D"};

static const char* const kJwrapperClassnameMap[] = {"java/lang/Boolean",
                                                    "java/lang/Byte",
                                                    "java/lang/Character",
                                                    "java/lang/Short",
                                                    "java/lang/Integer",
                                                    "java/lang/Long",
                                                    "java/lang/Float",
                                                    "java/lang/Double"};

static const char* const kJwrapperInitSigMap[] = {"(Z)V", "(B)V", "(C)V", "(S)V", "(I)V", "(J)V", "(F)V", "(D)V"};

static const char* const kJtypeClassnameMap[] =
    {"Z", "B", "C", "S", "I", "J", "F", "D", "Ljava/lang/String;", "[Z", "[B", "[C", "[S", "[I", "[J", "[F", "[D"};

template <typename T>
constexpr const char* jarray_classname() {
    return kJarrayClassnameMap[owl::typemap_find<typemap_j2jarray_t, T>::value];
}

template <typename T>
constexpr const char* jshort_classname() {
    return kJshortClassnameMap[owl::typemap_flip_find<typemap_j2c_t, T>::value];
}

template <typename T>
constexpr const char* jfull_classname() {
    return kJwrapperClassnameMap[owl::typemap_flip_find<typemap_j2c_t, T>::value];
}

template <typename T>
constexpr const char* jwrapper_init_sig() {
    return kJwrapperInitSigMap[owl::typemap_flip_find<typemap_j2c_t, T>::value];
}

template <typename T>
constexpr const char* jtype_classname() {
    return kJtypeClassnameMap[owl::typemap_find<typemap_j2c_t, T>::value];
}

std::string jstring_to_string(JNIEnv* env, jstring jstr);
jstring cstr_to_jstring(JNIEnv* env, const char* cstr);
jbyteArray string_to_jbytearray(JNIEnv* env, const std::string& str);
std::string jbytearray_to_string(JNIEnv* env, jbyteArray array);

jbooleanArray NewArray(JNIEnv* env, jsize len, const jboolean* buf);
jbyteArray NewArray(JNIEnv* env, jsize len, const jbyte* buf);
jcharArray NewArray(JNIEnv* env, jsize len, const jchar* buf);
jshortArray NewArray(JNIEnv* env, jsize len, const jshort* buf);
jintArray NewArray(JNIEnv* env, jsize len, const jint* buf);
jlongArray NewArray(JNIEnv* env, jsize len, const jlong* buf);
jfloatArray NewArray(JNIEnv* env, jsize len, const jfloat* buf);
jdoubleArray NewArray(JNIEnv* env, jsize len, const jdouble* buf);

inline jboolean* GetArrayElements(JNIEnv* env, jbooleanArray array) {
    return env->GetBooleanArrayElements(array, 0);
}
inline jbyte* GetArrayElements(JNIEnv* env, jbyteArray array) {
    return env->GetByteArrayElements(array, 0);
}
inline jchar* GetArrayElements(JNIEnv* env, jcharArray array) {
    return env->GetCharArrayElements(array, 0);
}
inline jshort* GetArrayElements(JNIEnv* env, jshortArray array) {
    return env->GetShortArrayElements(array, 0);
}
inline jint* GetArrayElements(JNIEnv* env, jintArray array) {
    return env->GetIntArrayElements(array, 0);
}
inline jlong* GetArrayElements(JNIEnv* env, jlongArray array) {
    return env->GetLongArrayElements(array, 0);
}
inline jfloat* GetArrayElements(JNIEnv* env, jfloatArray array) {
    return env->GetFloatArrayElements(array, 0);
}
inline jdouble* GetArrayElements(JNIEnv* env, jdoubleArray array) {
    return env->GetDoubleArrayElements(array, 0);
}

inline void ReleaseArrayElements(JNIEnv* env, jbooleanArray array, jboolean* elems) {
    env->ReleaseBooleanArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jbyteArray array, jbyte* elems) {
    env->ReleaseByteArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jcharArray array, jchar* elems) {
    env->ReleaseCharArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jshortArray array, jshort* elems) {
    env->ReleaseShortArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jintArray array, jint* elems) {
    env->ReleaseIntArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jlongArray array, jlong* elems) {
    env->ReleaseLongArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jfloatArray array, jfloat* elems) {
    env->ReleaseFloatArrayElements(array, elems, 0);
}
inline void ReleaseArrayElements(JNIEnv* env, jdoubleArray array, jdouble* elems) {
    env->ReleaseDoubleArrayElements(array, elems, 0);
}

inline std::string jarray_to_vector(JNIEnv* env, jstring jstr) {
    return jstring_to_string(env, jstr);
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
auto jarray_to_vector(JNIEnv* env, T array) -> typemap_j2c<T> {
    typemap_j2c<T> vec;
    if (nullptr != array && env->GetArrayLength(array) > 0) {
        auto data = GetArrayElements(env, array);
        auto size = env->GetArrayLength(array);
        vec.assign(data, data + size);
        ReleaseArrayElements(env, array, data);
    }
    return vec;
}

template <typename F, typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
std::vector<F> jarray_to_vector2(JNIEnv* env, T array) {
    std::vector<F> vec;
    if (nullptr != array && env->GetArrayLength(array) > 0) {
        auto data = GetArrayElements(env, array);
        auto size = env->GetArrayLength(array);
        vec.assign(data, data + size);
        ReleaseArrayElements(env, array, data);
    }
    return vec;
}

template <typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
auto jni_j2c_cast(JNIEnv* env, T t) -> typemap_j2c<T> {
    return jarray_to_vector(env, t);
}

template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
auto jni_j2c_cast(JNIEnv*, T t) -> typemap_j2c<T> {
    return t;
}

template <typename T, typename std::enable_if<std::is_same<T, jstring>::value>::type* = nullptr>
T vector_to_jarray(JNIEnv* env, const std::string& str) {
    return cstr_to_jstring(env, str.c_str());
}

template <typename T, typename std::enable_if<std::is_same<T, jstring>::value>::type* = nullptr>
T vector_to_jarray(JNIEnv* env, const std::string* str) {
    return str == nullptr ? nullptr : vector_to_jarray<T>(env, *str);
}

template <typename T, typename std::enable_if<std::is_same<T, jstring>::value>::type* = nullptr>
T vector_to_jarray(JNIEnv* env, const char* str) {
    return cstr_to_jstring(env, str);
}

template <typename T, typename F, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
T vector_to_jarray(JNIEnv* env, const F& f) {
    typedef typemap_jarray2j<T> value_type;
    return NewArray(env, (jsize)f.size(), (const value_type*)f.data());
}

template <typename T, typename F, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
T vector_to_jarray(JNIEnv* env, const F* f) {
    return f == nullptr ? nullptr : vector_to_jarray<T>(env, *f);
}

template <typename T, typename F, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
T jni_c2j_cast(JNIEnv* env, const F& f) {
    return vector_to_jarray<T>(env, f);
}

template <typename T, typename F, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
T jni_c2j_cast(JNIEnv*, F f) {
    return f;
}

// String[] to std::vector<std::string>
std::vector<std::string> jobjectarray_to_stringvector(JNIEnv* env, jobjectArray array);

// std::vector<std::string> to String[]
jobjectArray stringvector_to_jobjectarray(JNIEnv* env, const std::vector<std::string>& vec);

// String[][] to std::vector<std::vector<std::string>>
std::vector<std::vector<std::string>> jobjectarray_to_stringvector2(JNIEnv* env, jobjectArray array);

// std::vector<std::vector<std::string>> to String[][]
jobjectArray stringvector2_to_jobjectarray(JNIEnv* env, const std::vector<std::vector<std::string>>& vec);

template <typename T>
struct jni_type_traits {
    typedef typemap_j2jarray<T> array_type;
    typedef typemap_j2c<array_type> vector_type;
};

template <>
struct jni_type_traits<jstring> {
    typedef jobjectArray array_type;
    typedef std::vector<std::vector<std::string>> vector_type;
};

// T[][] to std::vector<std::vector<U>>
// T -> (jboolean, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble, jstring)
// U -> (T, jbyte, jstring -> std::string)
template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
auto jobjectarray_to_vector2(JNIEnv* env, jobjectArray array) -> std::vector<typename jni_type_traits<T>::vector_type> {
    typedef typename jni_type_traits<T>::vector_type vector_type;
    typedef typename jni_type_traits<T>::array_type array_type;
    auto len = env->GetArrayLength(array);
    std::vector<vector_type> vecvec;
    vecvec.reserve(len);
    for (auto i = 0; i < len; ++i) {
        auto arr = (array_type)env->GetObjectArrayElement(array, i);
        vecvec.push_back(jarray_to_vector(env, arr));
        env->DeleteLocalRef(arr);
    }
    return vecvec;
}

template <typename T, typename std::enable_if<std::is_same<T, jstring>::value>::type* = nullptr>
auto jobjectarray_to_vector2(JNIEnv* env, jobjectArray array) -> std::vector<std::vector<std::string>> {
    return jobjectarray_to_stringvector2(env, array);
}

// std::vector<std::vector<U>> to T[][]
// T -> (jboolean, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble, jstring)
// U -> (T, jbyte, jstring -> std::string)
template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
jobjectArray vector2_to_jobjectarray(JNIEnv* env, const std::vector<typename jni_type_traits<T>::vector_type>& vec) {
    typedef typename jni_type_traits<T>::array_type array_type;
    auto cls = jcache::shared()->get_class(env, jarray_classname<T>());
    auto array = (jobjectArray)env->NewObjectArray(vec.size(), cls, nullptr);
    for (auto i = 0; i < vec.size(); ++i) {
        jobject value = vector_to_jarray<array_type>(env, vec[i]);
        env->SetObjectArrayElement(array, i, value);
    }
    return array;
};

template <typename T, typename std::enable_if<std::is_same<T, jstring>::value>::type* = nullptr>
jobjectArray vector2_to_jobjectarray(JNIEnv* env, const std::vector<std::vector<std::string>>& vec) {
    return stringvector2_to_jobjectarray(env, vec);
}

template <typename F, typename std::enable_if<std::is_enum<F>::value>::type* = nullptr>
jobject cenum_to_jobject(JNIEnv* env, F data, const std::string& sig) {
    jclass cls = jcache::shared()->get_class(env, sig.c_str());
    jmethodID mid = env->GetStaticMethodID(cls, "forNumber", ("(I)L" + sig + ";").c_str());
    jobject obj = (jobject)env->CallStaticObjectMethod(cls, mid, (jint)data);
    return obj;
}

template <typename F>
jobject cpb_to_jobject(JNIEnv* env, const F& data, const std::string& sig) {
    jclass cls = jcache::shared()->get_class(env, sig.c_str());
    jmethodID mid = env->GetStaticMethodID(cls, "parseFrom", ("([B)L" + sig + ";").c_str());
    jobject obj = (jobject)env->CallStaticObjectMethod(cls, mid, jni_c2j_cast<jbyteArray>(env, data));
    return obj;
}

std::string jobject_to_pbstring(JNIEnv* env, const jobject obj, const std::string& sig);

std::vector<std::string> jobjectarray_to_pbstringvector(JNIEnv* env, const jobjectArray array, const std::string& sig);

jobjectArray cpbarray_to_jobjectarray(JNIEnv* env, const std::vector<std::string>& vec, const std::string& sig);
// std::vector<std::string> to ArrayList<byte[]>
jobject stringvector_to_arraylist(JNIEnv* env, const std::vector<std::string>& vec);

// ArrayList<byte[]> to std::vector<std::string>
std::vector<std::string> arraylist_to_stringvector(JNIEnv* env, jobject obj);

// const char*/std::string -> jstring
inline jstring __jni_c2j_cast_s(JNIEnv* env, const char* x) {
    return cstr_to_jstring(env, x);
}

inline jstring __jni_c2j_cast_s(JNIEnv* env, const std::string& x) {
    return cstr_to_jstring(env, x.c_str());
}

// T -> c++ type
// if T is arithmetic -> T
// if T is const char*/std::string -> jbyteArray
// std::vector<T> -> typemap_j2jarray<T>
template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
T __jni_c2j_cast(JNIEnv* env, T x) {
    return x;
}

inline jbyteArray __jni_c2j_cast(JNIEnv* env, const char* x) {
    return NewArray(env, (jsize)strlen(x), (const jbyte*)x);
}

inline jbyteArray __jni_c2j_cast(JNIEnv* env, const std::string& x) {
    return NewArray(env, (jsize)x.size(), (const jbyte*)x.data());
}

template <typename T>
auto __jni_c2j_cast(JNIEnv* env, const std::vector<T>& x) -> typemap_j2jarray<T> {
    return NewArray(env, (jsize)x.size(), (const T*)x.data());
}

#define __c2j_cast(arg) jnicat::__jni_c2j_cast(env, arg)
#define __c2j_cast_s(arg) jnicat::__jni_c2j_cast_s(env, arg)

}  // namespace jnicat

#define j2c_cast(arg) jnicat::jni_j2c_cast(env, arg)
#define c2j_cast(T, arg) jnicat::jni_c2j_cast<T>(env, arg)

#define j2c_cast_sv(arg) jnicat::jobjectarray_to_stringvector(env, arg)
#define j2c_cast_vv(T, arg) jnicat::jobjectarray_to_vector2<T>(env, arg)

#define c2j_cast_sv(arg) jnicat::stringvector_to_jobjectarray(env, arg)
#define c2j_cast_vv(T, arg) jnicat::vector2_to_jobjectarray<T>(env, arg)

#define c2j_cast_sv_alba(arg) jnicat::stringvector_to_arraylist(env, arg)
#define j2c_cast_sv_alba(arg) jnicat::arraylist_to_stringvector(env, arg)

#define j2c_cast_v(T, arg) jnicat::jarray_to_vector2<T>(env, arg)

#define c2j_enum_cast(arg1, arg2) jnicat::cenum_to_jobject(env, arg1, arg2)
#define c2j_pb_cast(arg1, arg2) jnicat::cpb_to_jobject(env, arg1, arg2)

#define c2j_pb_cast_v(arg1, arg2) jnicat::cpbarray_to_jobjectarray(env, arg1, arg2)

#define j2c_pb_cast(arg1, arg2) jnicat::jobject_to_pbstring(env, arg1, arg2)

#define j2c_pb_cast_v(arg1, arg2) jnicat::jobjectarray_to_pbstringvector(env, arg1, arg2)

// support for Java wrapper class Boolean,Int,etc...
namespace jnicat {

inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jboolean value) {
    env->SetBooleanField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jchar value) {
    env->SetCharField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jbyte value) {
    env->SetByteField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jshort value) {
    env->SetShortField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jint value) {
    env->SetIntField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong value) {
    env->SetLongField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jfloat value) {
    env->SetFloatField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jdouble value) {
    env->SetDoubleField(obj, fieldID, value);
}
inline void SetField(JNIEnv* env, jobject obj, jfieldID fieldID, jobject value) {
    env->SetObjectField(obj, fieldID, value);
}

inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jboolean& value) {
    value = env->GetBooleanField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jchar& value) {
    value = env->GetCharField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jbyte& value) {
    value = env->GetByteField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jshort& value) {
    value = env->GetShortField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jint& value) {
    value = env->GetIntField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jlong& value) {
    value = env->GetLongField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jfloat& value) {
    value = env->GetFloatField(obj, fieldID);
}
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, jdouble& value) {
    value = env->GetDoubleField(obj, fieldID);
}

template <typename T, typename std::enable_if<std::is_convertible<T, jobject>::value>::type* = nullptr>
inline void GetField(JNIEnv* env, jobject obj, jfieldID fieldID, T& value) {
    value = (T)env->GetObjectField(obj, fieldID);
}

template <typename T>
class jref {
 public:
    jref(JNIEnv* env, jobject obj) : env_(env), obj_(obj), value_() {
        cls_ = jcache::shared()->get_class(env, jfull_classname<T>());
        fid_ = jcache::shared()->get_fieldid(env, cls_, "value", jshort_classname<T>());
        if (cls_ != nullptr && fid_ != nullptr) {
            GetField(env_, obj_, fid_, value_);
        }
    }
    ~jref() {
        if (cls_ != nullptr && fid_ != nullptr) {
            SetField(env_, obj_, fid_, value_);
        }
    }

    T value() const {
        return value_;
    }
    void set_value(T value) {
        value_ = value;
    }

    operator T() const {
        return value_;
    }
    jref& operator=(T value) {
        value_ = value;
        return *this;
    }

    jref(jref&& rhs) {
        env_ = rhs.env_;
        obj_ = rhs.obj_;
        cls_ = rhs.cls_;
        fid_ = rhs.fid_;
        value_ = rhs.value_;

        rhs.cls_ = nullptr;
    }

    jref& operator=(const jref& rhs) {
        value_ = rhs.value_;
        return *this;
    }

    jref(const jref&) = delete;

 private:
    JNIEnv* env_;
    jobject obj_;
    jclass cls_;
    jfieldID fid_;
    T value_;
};

// Java fundamental type int,byte,etc to Java wrapper class Integer,Byte,etc
template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
jobject jni_c2jobject(JNIEnv* env, T t) {
    auto cls = jcache::shared()->get_class(env, jfull_classname<T>());
    auto ctor = jcache::shared()->get_methodid(env, cls, "<init>", jwrapper_init_sig<T>());
    if (cls != nullptr && ctor != nullptr) {
        return env->NewObject(cls, ctor, t);
    }
    return nullptr;
}

template <typename T>
T jni_jobject2c(JNIEnv* env, jobject obj) {
    T value;
    auto cls = jcache::shared()->get_class(env, jfull_classname<T>());
    auto fid = jcache::shared()->get_fieldid(env, cls, "value", jshort_classname<T>());
    if (cls != nullptr && fid != nullptr) {
        GetField(env, obj, fid, value);
    }
    return value;
}

// jintArray    -> jarray<int>
// T            -> bool, int, etc
// j_type       -> jboolean, jint, etc
// jarray_type  -> jbooleanArray, jintArray, etc
template <typename T>
class jarray {
    static_assert(is_c_arithmetic<T>::value, "");
    typedef typemap_c2j<T> j_type;
    typedef typemap_j2jarray<j_type> jarray_type;

 public:
    jarray(JNIEnv* env, jarray_type array) : env_(env), array_(array), ptr_(nullptr), size_(0) {
        size_ = env_->GetArrayLength(array_);
    }

    ~jarray() {
        __release_ptr();
    }

    jarray(jarray&& rhs) {
        env_ = rhs.env_;
        array_ = rhs.array_;
        ptr_ = rhs.ptr_;
        size_ = rhs.size_;

        rhs.ptr_ = nullptr;
        rhs.size_ = 0;
    }

    jarray(jarray&) = delete;
    jarray& operator=(jarray&) = delete;

    T* data() {
        return __get_ptr();
    }
    size_t size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }

    T& operator[](size_t pos) {
        assert(pos < size_);
        return data()[pos];
    }

 private:
    T* __get_ptr() const {
        if (ptr_ == nullptr) {
            ptr_ = GetArrayElements(env_, array_);
        }
        return ptr_;
    }

    void __release_ptr() {
        if (ptr_ != nullptr) {
            ReleaseArrayElements(env_, array_, ptr_);
            ptr_ = nullptr;
        }
    }

 private:
    JNIEnv* env_;
    jarray_type array_;
    mutable j_type* ptr_;
    mutable size_t size_;
};

class jbuffer {
 public:
    jbuffer() : data_(nullptr), size_(0) {
    }

    jbuffer(void* data, int64_t size) : data_((uint8_t*)data), size_(size) {
    }

    jbuffer(JNIEnv* env, jobject obj) {
        data_ = (uint8_t*)env->GetDirectBufferAddress(obj);
        size_ = env->GetDirectBufferCapacity(obj);
    }

    ~jbuffer() {
    }

    uint8_t* data() {
        return data_;
    }
    int64_t size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }
    uint8_t& operator[](size_t pos) {
        assert(pos < size_);
        return data_[pos];
    }

    jobject to_bytebuffer(JNIEnv* env) {
        if (data_ == nullptr) {
            return nullptr;
        }
        return env->NewDirectByteBuffer(data_, size_);
    }

 private:
    uint8_t* data_;
    int64_t size_;
};

enum jcachetype { kCacheClass, kCacheMethod, kCacheStaticMethod, kCacheField, kCacheStaticField };

struct jcacheitem {
    const char* classname;
    const char* name;
    const char* signature;
    jcachetype type;
};

struct jnativeitem {
    const char* classname;
    const JNINativeMethod* methods;
    size_t count;
};

inline jboolean __jvalue_cast(jvalue val, jboolean*) {
    return val.z;
}
inline jbyte __jvalue_cast(jvalue val, jbyte*) {
    return val.b;
}
inline jchar __jvalue_cast(jvalue val, jchar*) {
    return val.c;
}
inline jshort __jvalue_cast(jvalue val, jshort*) {
    return val.s;
}
inline jint __jvalue_cast(jvalue val, jint*) {
    return val.i;
}
inline jlong __jvalue_cast(jvalue val, jlong*) {
    return val.j;
}
inline jfloat __jvalue_cast(jvalue val, jfloat*) {
    return val.f;
}
inline jdouble __jvalue_cast(jvalue val, jdouble*) {
    return val.d;
}
inline jobject __jvalue_cast(jvalue val, void*) {
    return val.l;
}

// T -> jni primitive types & jobject types
template <typename T, typename std::enable_if<!std::is_void<T>::value>::type* = nullptr>
T jvalue_cast(jvalue val) {
    return (T)__jvalue_cast(val, (T*)nullptr);
}

template <typename T, typename std::enable_if<std::is_void<T>::value>::type* = nullptr>
void jvalue_cast(jvalue val) {
}

std::string GetStackTraceString(jthrowable e);

jobject NewObjectV(JNIEnv* env, const char* classname, const char* methodname, const char* signature, va_list args);
jobject NewObject(JNIEnv* env, const char* classname, const char* methodname, const char* signature, ...);
jobject NewObject(JNIEnv* env, const jcacheitem* ci, ...);

jvalue CallStaticMethodV(JNIEnv* env,
                         const char* classname,
                         const char* methodname,
                         const char* signature,
                         va_list args);
jvalue CallStaticMethod(JNIEnv* env, const char* classname, const char* methodname, const char* signature, ...);
jvalue CallStaticMethod(JNIEnv* env, const jcacheitem* ci, ...);

jvalue CallMethodV(JNIEnv* env, jobject obj, const char* methodname, const char* signature, va_list args);
jvalue CallMethod(JNIEnv* env, jobject obj, const char* methodname, const char* signature, ...);
jvalue CallMethod(JNIEnv* env, jobject obj, const jcacheitem* ci, ...);

const jcacheitem* add_cacheitem(const jcacheitem* ci);
const jnativeitem* add_nativeitem(const jnativeitem* ni);

// jobject 类型参数会返回自己，其它基本类型参数返回 nullptr
inline jobject __filter_object_arg(jobject arg) {
    return arg;
}

template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
jobject __filter_object_arg(T) {
    return nullptr;
}
inline jobject __filter_object_arg(void* arg) {
    return nullptr;
}

template <size_t N>
void __delete_localref_array(JNIEnv* env, jobject (&object_args)[N]) {
    for (auto obj : object_args) {
        if (obj != nullptr) {
            env->DeleteLocalRef(obj);
        }
    }
}

class jnienv_ptr {
 public:
    explicit jnienv_ptr(JavaVM* vm = nullptr);
    ~jnienv_ptr();

    operator JNIEnv*() const {
        return env_;
    }
    JNIEnv* operator->() const {
        return env_;
    }
    JNIEnv* get() const {
        return env_;
    }
    void detach();

    jnienv_ptr(const jnienv_ptr&) = delete;
    jnienv_ptr& operator=(const jnienv_ptr&) = delete;

    template <typename... Args>
    jobject new_object(const char* classname, const char* methodname, const char* signature, Args... args) {
        return NewObject(env_, classname, methodname, signature, args...);
    }

    template <typename... Args>
    jobject new_object(const jcacheitem* ci, Args... args) {
        return NewObject(env_, ci, args...);
    }

    // NOTE:
    // C++ 调用 Java 的情况下，C++ 层构造出的 jobject 类型参数在调用结束后，
    // 必须调用 DeleteLocalRef() 删除局部引用，
    // 否则会出现泄漏，在 Android 8.0 以下局部引用数超出限制会导致 Crash
    //
    // PS：
    // 返回给 Java 层的 jobject 对象，由 Java 层负责管理，无需调用 DeleteLocalRef()

    // 解决办法：
    // 使用元编程构造一个 jobject 数组：
    // 1）对于 jobject 参数，在数组中的值是本身；
    // 2）对于非 jobject 参数，在数组中的值是 nullptr；
    // 3）CallStaticMethod()/CallMethod() 调用结束后，循环释放局部引用

    template <typename T, typename... Args>
    T call_static_method(const char* classname, const char* methodname, const char* signature, Args... args) {
        jobject object_args[sizeof...(args) + 1]{__filter_object_arg(args)...};
        auto result = CallStaticMethod(env_, classname, methodname, signature, args...);
        __delete_localref_array(env_, object_args);
        return jvalue_cast<T>(result);
    }

    template <typename T, typename... Args>
    T call_static_method(const jcacheitem* ci, Args... args) {
        return call_static_method<T>(ci->classname, ci->name, ci->signature, args...);
    }

    template <typename T, typename... Args>
    T call_method(jobject obj, const char* methodname, const char* signature, Args... args) {
        jobject object_args[sizeof...(args) + 1]{__filter_object_arg(args)...};
        auto result = CallMethod(env_, obj, methodname, signature, args...);
        __delete_localref_array(env_, object_args);
        return jvalue_cast<T>(result);
    }

    template <typename T, typename... Args>
    T call_method(jobject obj, const jcacheitem* ci, Args... args) {
        return call_method<T>(obj, ci->name, ci->signature, args...);
    }

    template <typename T, typename... Args>
    T call_method_without_release(jobject obj, const char* methodname, const char* signature, Args... args) {
        auto result = CallMethod(env_, obj, methodname, signature, args...);
        return jvalue_cast<T>(result);
    }

    template <typename T, typename... Args>
    T call_method_without_release(jobject obj, const jcacheitem* ci, Args... args) {
        return call_method<T>(obj, ci->name, ci->signature, args...);
    }

    template <typename T, typename... Args>
    T call_static_method_raw(const char* classname, const char* methodname, const char* signature, Args... args) {
        return jvalue_cast<T>(CallStaticMethod(env_, classname, methodname, signature, args...));
    }

    template <typename T, typename... Args>
    T call_static_method_raw(const jcacheitem* ci, Args... args) {
        return jvalue_cast<T>(CallStaticMethod(env_, ci->classname, ci->name, ci->signature, args...));
    }

    template <typename T, typename... Args>
    T call_method_raw(jobject obj, const char* methodname, const char* signature, Args... args) {
        return jvalue_cast<T>(CallMethod(env_, obj, methodname, signature, args...));
    }

    template <typename T, typename... Args>
    T call_method_raw(jobject obj, const jcacheitem* ci, Args... args) {
        return jvalue_cast<T>(CallMethod(env_, ci->name, ci->signature, args...));
    }

    template <typename T, typename std::enable_if<owl::typemap_contains<typemap_j2c_t, T>::value>::type* = nullptr>
    T get_field(jobject obj, const char* fieldname, T default_value = T()) {
        T value = default_value;
        const char* signature = jtype_classname<T>();
        jfieldID fid = jcache::shared()->get_fieldid(env_, obj, fieldname, signature);
        if (fid != nullptr) {
            GetField(env_, obj, fid, value);
        }
        return value;
    }

    template <typename T, typename std::enable_if<owl::typemap_contains<typemap_j2c_t, T>::value>::type* = nullptr>
    void set_field(jobject obj, const char* fieldname, T value) {
        const char* signature = jtype_classname<T>();
        jfieldID fid = jcache::shared()->get_fieldid(env_, obj, fieldname, signature);
        if (fid != nullptr) {
            SetField(env_, obj, fid, value);
        }
    }

    jobject get_object_field(jobject obj,
                             const char* fieldname,
                             const char* signature,
                             jobject default_value = nullptr);
    void set_object_field(jobject obj, const char* fieldname, const char* signature, jobject value);

 private:
    JavaVM* vm_;
    JNIEnv* env_;
    bool attached_;
};

}  // namespace jnicat

template <typename T>
using jref_t = jnicat::jref<T>;

template <typename T>
using jarray_t = jnicat::jarray<T>;

using jbuffer_t = jnicat::jbuffer;
using jcache = jnicat::jcache;
using jnienv_ptr = jnicat::jnienv_ptr;

#define j2c_cast_ref(T, arg) jref_t<T>(env, arg)
#define j2c_cast_out(T, arg) jarray_t<T>(env, arg)

#define j2c_cast_buf(arg) jbuffer_t(env, arg)
#define j2c_cast_obj(T, arg) jnicat::jni_jobject2c<T>(env, arg)
#define c2j_cast_buf(arg) arg.to_bytebuffer(env)
#define c2j_cast_obj(arg) jnicat::jni_c2jobject(env, arg)

#define c2j_new_object(methodinfo, ...) env.new_object(methodinfo, ##__VA_ARGS__)

#define c2j_call_static(T, methodinfo, ...) env.call_static_method<T>(methodinfo, ##__VA_ARGS__)

#define c2j_call(T, obj, methodinfo, ...) env.call_method<T>(obj, methodinfo, ##__VA_ARGS__)

#define c2j_call_without_release(T, obj, methodinfo, ...) \
    env.call_method_without_release<T>(obj, methodinfo, ##__VA_ARGS__)

// PP_CAT
#ifndef PP_JNICAT
#define PP_JNICAT(x, y) PP_JNICAT_I(x, y)
#define PP_JNICAT_I(x, y) x##y
#endif

#define JNICAT_DEFINE_CLASS(classname) \
    JNICAT_DEFINE_CACHE(PP_JNICAT(kClass, __LINE__), classname, "", "", jnicat::kCacheClass)

#define JNICAT_DEFINE_METHOD(varname, classname, methodname, signature) \
    JNICAT_DEFINE_CACHE(varname, classname, methodname, signature, jnicat::kCacheMethod)

#define JNICAT_DEFINE_STATIC_METHOD(varname, classname, methodname, signature) \
    JNICAT_DEFINE_CACHE(varname, classname, methodname, signature, jnicat::kCacheStaticMethod)

#define JNICAT_DEFINE_FIELD(varname, classname, fieldname, signature) \
    JNICAT_DEFINE_CACHE(varname, classname, fieldname, signature, jnicat::kCacheField)

#define JNICAT_DEFINE_STATIC_FIELD(varname, classname, fieldname, signature) \
    JNICAT_DEFINE_CACHE(varname, classname, fieldname, signature, jnicat::kCacheStaticField)

#define JNICAT_DEFINE_CACHE(var, classname, name, signature, type)                         \
    static const jnicat::jcacheitem PP_JNICAT(ci_, var){classname, name, signature, type}; \
    static const jnicat::jcacheitem* const var = jnicat::add_cacheitem(&PP_JNICAT(ci_, var));

#define JNICAT_DEFINE_JNI_METHOD(var, classname, jni_methods, count)                     \
    static const jnicat::jnativeitem PP_JNICAT(ni_, var){classname, jni_methods, count}; \
    static const jnicat::jnativeitem* const var = jnicat::add_nativeitem(&PP_JNICAT(ni_, var));

static constexpr int __sig_index(const char* s, int i) {
    return s[i] == '\0' ? -1 : (s[i] == ')' ? i + 1 : __sig_index(s, i + 1));
}

static constexpr char __sig_char(const char* s) {
    return __sig_index(s, 0) == -1 ? '\0' : s[__sig_index(s, 0)];
}

template <char sig>
struct sig_to_return_type;

#define DECL_SIG_TO_RETURN_TYPE(S, R) \
    template <>                       \
    struct sig_to_return_type<S> {    \
        typedef R type;               \
    };

DECL_SIG_TO_RETURN_TYPE('V', void)
DECL_SIG_TO_RETURN_TYPE('[', jobject)
DECL_SIG_TO_RETURN_TYPE('L', jobject)
DECL_SIG_TO_RETURN_TYPE('Z', jboolean)
DECL_SIG_TO_RETURN_TYPE('B', jbyte)
DECL_SIG_TO_RETURN_TYPE('C', jchar)
DECL_SIG_TO_RETURN_TYPE('S', jshort)
DECL_SIG_TO_RETURN_TYPE('I', jint)
DECL_SIG_TO_RETURN_TYPE('J', jlong)
DECL_SIG_TO_RETURN_TYPE('F', jfloat)
DECL_SIG_TO_RETURN_TYPE('D', jdouble)

template <char sig>
using jreturn_type = typename sig_to_return_type<sig>::type;

class jObject {
 public:
    explicit jObject(jobject object) : object_(object) {
    }

    jObject(const char* classname, const char* init_signature, ...) : object_(0) {
        va_list args;
        va_start(args, init_signature);
        object_ = jnicat::NewObjectV(env_, classname, "<init>", init_signature, args);
        va_end(args);
    }

    virtual ~jObject() {
        env_->DeleteLocalRef(object_);
    }

    jobject raw() const {
        return object_;
    }

 protected:
    jnienv_ptr env_;
    jobject object_;
};

#define JNICAT_CLASS_BEGIN(cpp_classname, java_classname)                                                            \
    JNICAT_DEFINE_CLASS(java_classname)                                                                              \
    class cpp_classname : public jObject {                                                                           \
        typedef cpp_classname CppClassType;                                                                          \
        static constexpr const char* classname() {                                                                   \
            return java_classname;                                                                                   \
        }                                                                                                            \
                                                                                                                     \
     public:                                                                                                         \
        explicit cpp_classname(jobject object) : jObject(object) {                                                   \
        }                                                                                                            \
        template <typename... Args>                                                                                  \
        cpp_classname(const char* init_signature, Args... args) : jObject(java_classname, init_signature, args...) { \
        }

#define JNICAT_INIT_METHOD(methodname, signature)    \
    template <typename... Args>                      \
    static CppClassType* methodname(Args... args) {  \
        return new CppClassType(signature, args...); \
    }

#define JNICAT_METHOD(methodname, signature)                                      \
    template <typename... Args>                                                   \
    auto methodname(Args... args)->jreturn_type<__sig_char(signature)> {          \
        using R = jreturn_type<__sig_char(signature)>;                            \
        return env_.call_method_raw<R>(object_, #methodname, signature, args...); \
    }

#define JNICAT_STATIC_METHOD(methodname, signature)                                         \
    template <typename... Args>                                                             \
    static auto methodname(Args... args)->jreturn_type<__sig_char(signature)> {             \
        using R = jreturn_type<__sig_char(signature)>;                                      \
        jnienv_ptr env;                                                                     \
        return env.call_static_method_raw<R>(classname(), #methodname, signature, args...); \
    }

#define JNICAT_CLASS_END() \
    }                      \
    ;

#endif  // JNIRPC_JNICAT_CORE_H
