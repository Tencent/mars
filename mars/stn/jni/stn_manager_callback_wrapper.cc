//
// Created by Changpeng Pan on 2022/9/14.
//

#include "stn_manager_callback_wrapper.h"

#include "comm/jni/jnicat/jnicat_core.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scope_jenv.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/jni/util/var_cache.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/task_profile.h"

namespace mars {
namespace stn {

DEFINE_FIND_CLASS(KC2Java, "com/tencent/mars/stn/StnManager$CallBack")

StnManagerJniCallback::StnManagerJniCallback(JNIEnv* env, jobject callback) {
    callback_inst_ = env->NewGlobalRef(callback);
    jclass objClass = env->GetObjectClass(callback);
    if (objClass) {
        callbacks_class_ = reinterpret_cast<jclass>(env->NewGlobalRef(objClass));
        env->DeleteLocalRef(objClass);
    }
}

StnManagerJniCallback::~StnManagerJniCallback() {
    jnienv_ptr env;
    env->DeleteGlobalRef(callback_inst_);
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_MakesureAuthed,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "makesureAuthed",
//                     "(Ljava/lang/String;Ljava/lang/String;)Z")

DEFINE_FIND_METHOD(kStnManagerJniCallback_MakesureAuthed, KC2Java, "makesureAuthed", "(Ljava/lang/String;Ljava/lang/String;)Z")
bool StnManagerJniCallback::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
//    jnienv_ptr env;
//    return j2c_cast(
//        c2j_call(jboolean, callback_inst_, kStnManagerJniCallback_MakesureAuthed, c2j_cast(jstring, _host), c2j_cast(jstring, _user_id)));

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    jboolean ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_MakesureAuthed, ScopedJstring(env, _host.c_str()).GetJstr(), ScopedJstring(env, _user_id.c_str()).GetJstr()).z;

    return ret;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_TrafficData,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "trafficData",
//                     "(II)V")

DEFINE_FIND_METHOD(kStnManagerJniCallback_TrafficData, KC2Java, "trafficData", "(II)V")
void StnManagerJniCallback::TrafficData(ssize_t _send, ssize_t _recv) {
//    jnienv_ptr env;
//    c2j_call(void, callback_inst_, kStnManagerJniCallback_TrafficData, c2j_cast(jint, _send), c2j_cast(jint, _recv));
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_TrafficData, (jint)_send, (jint)_recv);
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnNewDns,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onNewDns",
//                     "(Ljava/lang/String;Z)[Ljava/lang/String;")

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnNewDns, KC2Java, "onNewDns", "(Ljava/lang/String;Z)[Ljava/lang/String;")
std::vector<std::string> StnManagerJniCallback::OnNewDns(const std::string& host, bool longlink_host) {
//    jnienv_ptr env;
//    std::vector<std::string> iplist;
//    if (!host.empty()) {
//        jobjectArray ip_strs = c2j_call(jobjectArray,
//                                        callback_inst_,
//                                        kStnManagerJniCallback_OnNewDns,
//                                        c2j_cast(jstring, host),
//                                        c2j_cast(jboolean, longlink_host));
//        if (ip_strs != NULL) {
//            jsize size = env->GetArrayLength(ip_strs);
//            for (int i = 0; i < size; i++) {
//                jstring ip = (jstring)env->GetObjectArrayElement(ip_strs, i);
//                if (ip != NULL) {
//                    iplist.push_back(ScopedJstring(env, ip).GetChar());
//                }
//                JNU_FreeJstring(env, ip);
//            }
//            env->DeleteLocalRef(ip_strs);
//        }
//    } else {
//        // xerror2(TSF"host is empty");
//    }
//    return iplist;

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();
    std::vector<std::string> iplist;

    if (!host.empty()) {
        jobjectArray ip_strs = (jobjectArray)JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnNewDns, ScopedJstring(env, host.c_str()).GetJstr(), longlink_host).l;
        if (ip_strs != NULL) {
            jsize size = env->GetArrayLength(ip_strs);
            for (int i = 0; i < size; i++) {
                jstring ip = (jstring)env->GetObjectArrayElement(ip_strs, i);
                if (ip != NULL) {
                    iplist.push_back(ScopedJstring(env, ip).GetChar());
                }
                JNU_FreeJstring(env, ip);
            }
            env->DeleteLocalRef(ip_strs);
        }
    }
    else {
        xerror2(TSF"host is empty");
    }

    return iplist;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnPush,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onPush",
//                     "(Ljava/lang/String;II[B[B)V")

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnPush, KC2Java, "onPush", "(Ljava/lang/String;II[B[B)V")
void StnManagerJniCallback::OnPush(const std::string& _channel_id,
                                   uint32_t _cmdid,
                                   uint32_t _taskid,
                                   const AutoBuffer& _body,
                                   const AutoBuffer& _extend) {
//    jnienv_ptr env;
//    jbyteArray data_jba = NULL;
//    jbyteArray extend_jba = NULL;
//    if (_body.Length() > 0) {
//        data_jba = JNU_Buffer2JbyteArray(env, _body);
//    } else {
//        // xdebug2(TSF"the data.Lenght() < = 0");
//    }
//
//    if (_extend.Length() > 0) {
//        data_jba = JNU_Buffer2JbyteArray(env, _extend);
//    } else {
//        // xdebug2(TSF"the _extend.Lenght() < = 0");
//    }
//
//    c2j_call(void,
//             callback_inst_,
//             kStnManagerJniCallback_OnPush,
//             c2j_cast(jstring, _channel_id),
//             c2j_cast(jint, _cmdid),
//             c2j_cast(jint, _taskid),
//             data_jba,
//             extend_jba);
//
//    if (data_jba != NULL) {
//        JNU_FreeJbyteArray(env, data_jba);
//    }
//
//    if (extend_jba != NULL) {
//        JNU_FreeJbyteArray(env, extend_jba);
//    }

    VarCache* cache_instance = VarCache::Singleton();

    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    jbyteArray data_jba = NULL;

    if (_body.Length() > 0) {
        data_jba = JNU_Buffer2JbyteArray(env, _body);
    } else {
        xdebug2(TSF"the data.Lenght() < = 0");
    }

    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnPush, ScopedJstring(env, _channel_id.c_str()).GetJstr(), (jint)_cmdid, (jint)_taskid, data_jba, NULL);

    if (data_jba != NULL) {
        JNU_FreeJbyteArray(env, data_jba);
    }
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_Req2Buf,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "req2Buf",
//                     "(ILjava/lang/Object;Ljava/lang/String;Ljava/io/ByteArrayOutputStream;[B[IILjava/lang/String;)Z")
//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_Req2Buf,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "req2Buf",
//                     "(ILjava/lang/Object;Ljava/lang/String;Ljava/io/ByteArrayOutputStream;[IILjava/lang/String;)Z")

DEFINE_FIND_METHOD(kStnManagerJniCallback_Req2Buf, KC2Java, "req2Buf", "(ILjava/lang/Object;Ljava/lang/String;Ljava/io/ByteArrayOutputStream;[IILjava/lang/String;)Z")

bool StnManagerJniCallback::Req2Buf(uint32_t _taskid,
                                    void* const _user_context,
                                    const std::string& _user_id,
                                    AutoBuffer& outbuffer,
                                    AutoBuffer& extend,
                                    int& error_code,
                                    const int channel_select,
                                    const std::string& host) {
//    jnienv_ptr env;

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    // obtain the class ByteArrayOutputStream
    jclass byte_array_outputstream_clz = cache_instance->GetClass(env, "java/io/ByteArrayOutputStream");

    // obtain the method id of construct method
    jmethodID construct_mid = cache_instance->GetMethodId(env, byte_array_outputstream_clz, "<init>", "()V");

    // construct  the object of ByteArrayOutputStream
    jobject byte_array_output_stream_obj = env->NewObject(byte_array_outputstream_clz, construct_mid);

//    jbyteArray byte_array_extend_stream_obj = NULL;
//    if (extend.Length() > 0) {
//        byte_array_extend_stream_obj = JNU_Buffer2JbyteArray(env, extend);
//    } else {
//        // xdebug2(TSF"the hashCodeBuffer.Lenght() < = 0");
//    }

    jintArray errcode_array = env->NewIntArray(2);
    if (errcode_array) {
        int len = env->GetArrayLength(errcode_array);
        xinfo2(TSF "mars2 jintArray is no empty len:%_", len);
    } else {
        xerror2(TSF "mars2 jintArray is empty.");
    }

//    jboolean ret =  c2j_call_without_release(jboolean,
//                                 callback_inst_,
//                                 kStnManagerJniCallback_Req2Buf,
//                                 c2j_cast(jint, _taskid),
//                                 _user_context,
//                                 c2j_cast(jstring, _user_id),
//                                 byte_array_output_stream_obj,
////                                 byte_array_extend_stream_obj,
//                                 errcode_array,
//                                 c2j_cast(jint, channel_select),
//                                 c2j_cast(jstring, host));
    jboolean ret = JNU_CallMethodByMethodInfo(env,
                                        callback_inst_,
                                        kStnManagerJniCallback_Req2Buf,
                                         (jint)_taskid,
                                        _user_context,
                                        ScopedJstring(env, _user_id.c_str()).GetJstr(),
                                        byte_array_output_stream_obj,
                                        //byte_array_extend_stream_obj,
                                        errcode_array,
                                        (jint)channel_select,
                                        ScopedJstring(env, host.c_str()).GetJstr()).z;
    //    if (byte_array_extend_stream_obj != NULL) {
//        JNU_FreeJbyteArray(env, byte_array_extend_stream_obj);
//    }
    xinfo2(TSF "mars2 kStnManagerJniCallback_Req2Buf ret:%_", ret);
    if (errcode_array) {
        int len = env->GetArrayLength(errcode_array);
        xinfo2(TSF "mars2 jintArray is no empty len:%_", len);
    } else {
        xerror2(TSF "mars2 jintArray is empty.");
    }

    if (ret) {
        jbyteArray ret_byte_array = (jbyteArray)JNU_CallMethodByName(env, byte_array_output_stream_obj, "toByteArray", "()[B").l;
        if (ret_byte_array != NULL) {
            jsize alen = env->GetArrayLength(ret_byte_array);
            xdebug2(TSF"mars2 the retByteArray size %_", alen);
            jbyte* ba = env->GetByteArrayElements(ret_byte_array, NULL);
            outbuffer.Write(ba, alen);
            env->ReleaseByteArrayElements(ret_byte_array, ba, 0);
            env->DeleteLocalRef(ret_byte_array);
        } else {
            xdebug2(TSF"mars2 the retByteArray is null");
        }
    }

    jint* errcode = env->GetIntArrayElements(errcode_array, JNI_FALSE);
    error_code = errcode[0];
    env->ReleaseIntArrayElements(errcode_array, errcode, 0);
    env->DeleteLocalRef(errcode_array);

    env->DeleteLocalRef(byte_array_output_stream_obj);
    return ret;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_Buf2Resp,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "buf2Resp",
//                     "(ILjava/lang/Object;Ljava/lang/String;[B[B[II)I")

DEFINE_FIND_METHOD(kStnManagerJniCallback_Buf2Resp, KC2Java, "buf2Resp", "(ILjava/lang/Object;Ljava/lang/String;[B[II)I")
int StnManagerJniCallback::Buf2Resp(uint32_t _taskid,
                                    void* const _user_context,
                                    const std::string& _user_id,
                                    const AutoBuffer& _inbuffer,
                                    const AutoBuffer& _extend,
                                    int& _error_code,
                                    const int _channel_select) {
//    jnienv_ptr env;
//
//    jbyteArray resp_buf_jba = NULL;
//
//    if (_inbuffer.Length() > 0) {
//        resp_buf_jba = JNU_Buffer2JbyteArray(env, _inbuffer);
//    } else {
//        // xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
//    }
//
//    jbyteArray extend_buf_jba = NULL;
//
//    if (_extend.Length() > 0) {
//        extend_buf_jba = JNU_Buffer2JbyteArray(env, _extend);
//    } else {
//        // xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
//    }
//
//    jintArray errcode_array = env->NewIntArray(1);
//
//    int ret = j2c_cast(c2j_call_without_release(jint,
//                                callback_inst_,
//                                kStnManagerJniCallback_Buf2Resp,
//                                c2j_cast(jint, _taskid),
//                                _user_context,
//                                c2j_cast(jstring, _user_id),
//                                resp_buf_jba,
//                                extend_buf_jba,
//                                errcode_array,
//                                c2j_cast(jint, _channel_select)));
//
//    xinfo2(TSF "mars2 kStnManagerJniCallback_Buf2Resp ret:%_", ret);
//    if (errcode_array) {
//        int len = env->GetArrayLength(errcode_array);
//        xinfo2(TSF "mars2 kStnManagerJniCallback_Buf2Resp jintArray is no empty len:%_", len);
//    } else {
//        xerror2(TSF "mars2 kStnManagerJniCallback_Buf2Resp jintArray is empty.");
//    }
//
//    jint* errcode = env->GetIntArrayElements(errcode_array, NULL);
//    _error_code = errcode[0];
//    env->ReleaseIntArrayElements(errcode_array, errcode, 0);
//    env->DeleteLocalRef(errcode_array);
//
//    if (resp_buf_jba != NULL) {
//        env->DeleteLocalRef(resp_buf_jba);
//    }
//    if (extend_buf_jba != NULL) {
//        env->DeleteLocalRef(extend_buf_jba);
//    }
//    return ret;

    xdebug2(TSF"mars2 StnManagerJniCallback::Buf2Res start");
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    jbyteArray resp_buf_jba = NULL;

    if (_inbuffer.Length() > 0) {
        resp_buf_jba =  JNU_Buffer2JbyteArray(env, _inbuffer);
    } else {
        xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
    }
//    jbyteArray extend_buf_jba = NULL;
//
//    if (_extend.Length() > 0) {
//        extend_buf_jba = JNU_Buffer2JbyteArray(env, _extend);
//    } else {
//        // xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
//    }

    jintArray errcode_array = env->NewIntArray(1);

    jint ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_Buf2Resp, (jint)_taskid, _user_context, ScopedJstring(env, _user_id.c_str()).GetJstr(), resp_buf_jba, errcode_array, _channel_select).i;

    if (resp_buf_jba != NULL) {
        env->DeleteLocalRef(resp_buf_jba);
    }

    jint* errcode = env->GetIntArrayElements(errcode_array, NULL);
    _error_code = errcode[0];
    env->ReleaseIntArrayElements(errcode_array, errcode, 0);
    env->DeleteLocalRef(errcode_array);
    xdebug2(TSF"mars2 StnManagerJniCallback::Buf2Res end");
    return ret;
}

DEFINE_FIND_CLASS(KC2JavaStnCgiProfile, "com/tencent/mars/stn/StnLogic$CgiProfile");
//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnTaskEnd,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onTaskEnd",
//                     "(ILjava/lang/Object;Ljava/lang/String;IILcom/tencent/mars/stn/StnLogic$CgiProfile;)I")

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnTaskEnd, KC2Java, "onTaskEnd", "(ILjava/lang/Object;IILcom/tencent/mars/stn/StnLogic$CgiProfile;)I")
int StnManagerJniCallback::OnTaskEnd(uint32_t _taskid,
                                     void* const _user_context,
                                     const std::string& _user_id,
                                     int _error_type,
                                     int _error_code,
                                     const CgiProfile& _profile) {
//    jnienv_ptr env;
//    xverbose_function();
    xdebug2(TSF"mars2 recieve task profile: %_, %_, %_", _profile.start_connect_time, _profile.start_send_packet_time, _profile.read_packet_finished_time);

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    jclass cgiProfileCls = cache_instance->GetClass(env, KC2JavaStnCgiProfile);
    xdebug2(TSF"mars2 cgiProfileCls class find.");
    jmethodID jobj_init = env->GetMethodID(cgiProfileCls, "<init>", "()V");
    jobject jobj_cgiItem = env->NewObject(cgiProfileCls, jobj_init);
    if (nullptr == jobj_cgiItem) {
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), "C2Java_OnTaskEnd: create jobject failed.");
        return -1;
    }

    jfieldID fid_taskStartTime = env->GetFieldID(cgiProfileCls, "taskStartTime", "J");
    jfieldID fid_startConnectTime = env->GetFieldID(cgiProfileCls, "startConnectTime", "J");
    jfieldID fid_connectSuccessfulTime = env->GetFieldID(cgiProfileCls, "connectSuccessfulTime", "J");
    jfieldID fid_startHandshakeTime = env->GetFieldID(cgiProfileCls, "startHandshakeTime", "J");
    jfieldID fid_handshakeSuccessfulTime = env->GetFieldID(cgiProfileCls, "handshakeSuccessfulTime", "J");
    jfieldID fid_startSendPacketTime = env->GetFieldID(cgiProfileCls, "startSendPacketTime", "J");
    jfieldID fid_startReadPacketTime = env->GetFieldID(cgiProfileCls, "startReadPacketTime", "J");
    jfieldID fid_readPacketFinishedTime = env->GetFieldID(cgiProfileCls, "readPacketFinishedTime", "J");
    jfieldID fid_rtt = env->GetFieldID(cgiProfileCls, "rtt", "J");
    jfieldID fid_channelType = env->GetFieldID(cgiProfileCls, "channelType", "I");
    jfieldID fid_protocolType = env->GetFieldID(cgiProfileCls, "protocolType","I");

//    return j2c_cast(c2j_call(jint,
//                             callback_inst_,
//                             kStnManagerJniCallback_OnTaskEnd,
//                             c2j_cast(jint, _taskid),
//                             _user_context,
//                             c2j_cast(jstring, _user_id),
//                             c2j_cast(jint, _error_type),
//                             c2j_cast(jint, _error_code),
//                             jobj_cgiItem));
    uint64_t tls_start_time = _profile.tls_handshake_successful_time == 0 ? 0 : _profile.start_tls_handshake_time;
    env->SetLongField(jobj_cgiItem, fid_taskStartTime, _profile.start_time);
    env->SetLongField(jobj_cgiItem, fid_startConnectTime, _profile.start_connect_time);
    env->SetLongField(jobj_cgiItem, fid_connectSuccessfulTime, _profile.connect_successful_time);
    env->SetLongField(jobj_cgiItem, fid_startHandshakeTime,tls_start_time);
    env->SetLongField(jobj_cgiItem, fid_handshakeSuccessfulTime, _profile.tls_handshake_successful_time);
    env->SetLongField(jobj_cgiItem, fid_startSendPacketTime, _profile.start_send_packet_time);
    env->SetLongField(jobj_cgiItem, fid_startReadPacketTime, _profile.start_read_packet_time);
    env->SetLongField(jobj_cgiItem, fid_readPacketFinishedTime, _profile.read_packet_finished_time);
    env->SetLongField(jobj_cgiItem, fid_rtt, _profile.rtt);
    env->SetIntField(jobj_cgiItem, fid_channelType, _profile.channel_type);
    env->SetIntField(jobj_cgiItem, fid_protocolType, _profile.transport_protocol);
    xdebug2(TSF "mars2 OnTaskEnd start. _taskid:%_, _user_id:%_, _error_type:%_, _error_code:%_",
            _taskid,
            _user_id,
            _error_type,
            _error_code);
    int ret = (int)JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnTaskEnd, (jint)_taskid, _user_context, (jint)_error_type, (jint)_error_code, jobj_cgiItem).i;
    xdebug2(TSF"mars2 OnTaskEnd finish.");
    return ret;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_ReportConnectStatus,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "reportConnectStatus",
//                     "(II)V")

DEFINE_FIND_METHOD(kStnManagerJniCallback_ReportConnectStatus, KC2Java, "reportConnectStatus", "(II)V")
void StnManagerJniCallback::ReportConnectStatus(int _status, int _longlink_status) {
//    jnienv_ptr env;
//    c2j_call(void,
//             callback_inst_,
//             kStnManagerJniCallback_ReportConnectStatus,
//             c2j_cast(jint, _status),
//             c2j_cast(jint, _longlink_status));
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_ReportConnectStatus, (jint)_status, (jint)_longlink_status);
    xdebug2(TSF"all_connstatus = %0, longlink_connstatus = %_", _status, _longlink_status);

}

JNICAT_DEFINE_CLASS("com/tencent/mars/stn/Stn$ErrCmdType")
//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLongLinkNetworkError,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onLongLinkNetworkError",
//                     "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;I)V")
DEFINE_FIND_CLASS(StnErrCmdType,"com/tencent/mars/stn/Stn$ErrCmdType")
DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLongLinkNetworkError, KC2Java, "onLongLinkNetworkError", "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;I)V")
void StnManagerJniCallback::OnLongLinkNetworkError(ErrCmdType _err_type,
                                                   int _err_code,
                                                   const std::string& _ip,
                                                   uint16_t _port) {
//    jnienv_ptr env;
//
//    c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType");
//
//    c2j_call(void,
//             callback_inst_,
//             kStnManagerJniCallback_OnLongLinkNetworkError,
//             c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType"),
//             c2j_cast(jint, _err_code),
//             c2j_cast(jstring, _ip),
//             c2j_cast(jint, _port));
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    //    jmethodID construct_mid = cache_instance->GetMethodId(env, StnErrCmdType, "<init>", "()V");
    //    xinfo2(TSF "mars2 construct_mid:%_", construct_mid);
    //    jobject stn_err_cmd_type_obj = env->NewObject(StnErrCmdType, construct_mid);

    //TODO cpan mars2
//    JNU_CallMethodByMethodInfo(env,
//                               callback_inst_,
//                               kStnManagerJniCallback_OnLongLinkNetworkError,
//                               NULL,
//                               (jint)_err_code,
//                               ScopedJstring(env, _ip.c_str()).SafeGetChar(),
//                               (jint)_port);
}

//JNICAT_DEFINE_CLASS("com/tencent/mars/stn/Stn$ErrCmdType")
//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnShortLinkNetworkError,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onShortLinkNetworkError",
//                     "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;Ljava/lang/String;I)V")
//DEFINE_FIND_CLASS(StnErrCmdType,"com/tencent/mars/stn/Stn$ErrCmdType")
DEFINE_FIND_METHOD(kStnManagerJniCallback_OnShortLinkNetworkError, KC2Java, "onShortLinkNetworkError", "(Lcom/tencent/mars/stn/Stn$ErrCmdType;ILjava/lang/String;Ljava/lang/String;I)V")
void StnManagerJniCallback::OnShortLinkNetworkError(ErrCmdType _err_type,
                                                    int _err_code,
                                                    const std::string& _ip,
                                                    const std::string& _host,
                                                    uint16_t _port) {
//    jnienv_ptr env;
//
//    c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType");
//
//    c2j_call(void,
//             callback_inst_,
//             kStnManagerJniCallback_OnShortLinkNetworkError,
//             c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType"),
//             c2j_cast(jint, _err_code),
//             c2j_cast(jstring, _ip),
//             c2j_cast(jstring, _host),
//             c2j_cast(jint, _port));
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

//    jclass stn_err_cmd_type_clazz = cache_instance->GetClass(env, StnErrCmdType);
//    jmethodID construct_mid = cache_instance->GetMethodId(env, StnErrCmdType, "<init>", "()V");
//    xinfo2(TSF "mars2 construct_mid:%_", construct_mid);
//    jobject stn_err_cmd_type_obj = env->NewObject(StnErrCmdType, construct_mid);

    //TODO cpan mars2
//    JNU_CallMethodByMethodInfo(env,
//                               callback_inst_,
//                               kStnManagerJniCallback_OnShortLinkNetworkError,
////                               c2j_enum_cast(_err_type, "com/tencent/mars/stn/Stn$ErrCmdType"),
//                               NULL,
//                               (jint)_err_code,
//                               ScopedJstring(env, _ip.c_str()).SafeGetChar(),
//                               ScopedJstring(env, _host.c_str()).SafeGetChar(),
//                               (jint)_port);
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLongLinkStatusChange,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onLongLinkStatusChange",
//                     "(I)V")
DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLongLinkStatusChange, KC2Java, "onLongLinkStatusChange", "(I)V")
void StnManagerJniCallback::OnLongLinkStatusChange(int _status) {
//    jnienv_ptr env;
//    c2j_call(void, callback_inst_, kStnManagerJniCallback_OnLongLinkStatusChange, c2j_cast(jint, _status));
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env,callback_inst_,  kStnManagerJniCallback_OnLongLinkStatusChange, (jint)_status);
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "getLongLinkIdentifyCheckBuffer",
//                     "(Ljava/lang/String;Ljava/io/ByteArrayOutputStream;Ljava/io/ByteArrayOutputStream;[I)I")

DEFINE_FIND_METHOD(kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer, KC2Java, "getLongLinkIdentifyCheckBuffer", "(Ljava/lang/String;Ljava/io/ByteArrayOutputStream;Ljava/io/ByteArrayOutputStream;[I)I")
int StnManagerJniCallback::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id,
                                                          AutoBuffer& _identify_buffer,
                                                          AutoBuffer& _buffer_hash,
                                                          int32_t& _cmdid) {
//    jnienv_ptr env;
//    VarCache* cache_instance = VarCache::Singleton();
//    ScopeJEnv scope_jenv(cache_instance->GetJvm());
//    //    JNIEnv *env = scope_jenv.GetEnv();
//
//    // obtain the class ByteArrayOutputStream
//    jclass byte_array_output_stream_clz = cache_instance->GetClass(env, "java/io/ByteArrayOutputStream");
//    xinfo2(TSF "mars2 byte_array_output_stream_clz:%_", byte_array_output_stream_clz);
//
//    // obtain the method id of construct method
//    jmethodID construct_mid = cache_instance->GetMethodId(env, byte_array_output_stream_clz, "<init>", "()V");
//    xinfo2(TSF "mars2 construct_mid:%_", construct_mid);
//
//    // construct  the object of ByteArrayOutputStream
//    jobject byte_array_outputstream_obj = env->NewObject(byte_array_output_stream_clz, construct_mid);
//    jobject byte_array_outputstream_hash = env->NewObject(byte_array_output_stream_clz, construct_mid);
//
//    xinfo2(TSF "mars2 byte_array_outputstream_obj:%_", byte_array_outputstream_obj);
//
//    jintArray cmdId_array = env->NewIntArray(2);
//
//    int ret = j2c_cast(c2j_call(jint,
//                                callback_inst_,
//                                kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer,
//                                c2j_cast(jstring, _channel_id),
//                                byte_array_outputstream_obj,
//                                byte_array_outputstream_hash,
//                                cmdId_array));
//    //    int ret = JNU_CallStaticMethodByMethodInfo(env, kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer,
//    //    ScopedJstring(env, _channel_id.c_str()).GetJstr(), byte_array_outputstream_obj, byte_array_outputstream_hash,
//    //    cmdId_array).i;
//
//    if (ret == kCheckNext || ret == kCheckNever) {
//        xwarn2(TSF "getLongLinkIdentifyCheckBuffer uin == 0, not ready");
//        env->DeleteLocalRef(byte_array_outputstream_obj);
//        env->DeleteLocalRef(byte_array_outputstream_hash);
//        env->DeleteLocalRef(cmdId_array);
//        return ret;
//    }
//
//    jbyteArray ret_byte_array = NULL;
//    ret_byte_array = (jbyteArray)JNU_CallMethodByName(env, byte_array_outputstream_obj, "toByteArray", "()[B").l;
//
//    jbyteArray ret_byte_hash = NULL;
//    ret_byte_hash = (jbyteArray)JNU_CallMethodByName(env, byte_array_outputstream_hash, "toByteArray", "()[B").l;
//
//    jint* cmdId = env->GetIntArrayElements(cmdId_array, NULL);
//    _cmdid = cmdId[0];
//    env->ReleaseIntArrayElements(cmdId_array, cmdId, 0);
//    env->DeleteLocalRef(cmdId_array);
//
//    if (ret_byte_hash != NULL) {
//        jsize alen2 = env->GetArrayLength(ret_byte_hash);
//        jbyte* ba2 = env->GetByteArrayElements(ret_byte_hash, NULL);
//        _buffer_hash.Write(ba2, alen2);
//        env->ReleaseByteArrayElements(ret_byte_hash, ba2, 0);
//        env->DeleteLocalRef(ret_byte_hash);
//    }
//    if (ret_byte_array != NULL) {
//        jsize alen = env->GetArrayLength(ret_byte_array);
//        jbyte* ba = env->GetByteArrayElements(ret_byte_array, NULL);
//        _identify_buffer.Write(ba, alen);
//        env->ReleaseByteArrayElements(ret_byte_array, ba, 0);
//        env->DeleteLocalRef(ret_byte_array);
//
//    } else {
//        xdebug2(TSF "the retByteArray is NULL");
//    }
//
//    env->DeleteLocalRef(byte_array_outputstream_obj);
//    env->DeleteLocalRef(byte_array_outputstream_hash);
//    return ret;

    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    //obtain the class ByteArrayOutputStream
    jclass byte_array_output_stream_clz = cache_instance->GetClass(env, "java/io/ByteArrayOutputStream");

    //obtain the method id of construct method
    jmethodID construct_mid = cache_instance->GetMethodId(env, byte_array_output_stream_clz, "<init>", "()V");

    //construct  the object of ByteArrayOutputStream
    jobject byte_array_outputstream_obj = env->NewObject(byte_array_output_stream_clz, construct_mid);
    jobject byte_array_outputstream_hash = env->NewObject(byte_array_output_stream_clz, construct_mid);

    jintArray jcmdid_array = env->NewIntArray(2);

    jint ret = 0;
    ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer, ScopedJstring(env, _channel_id.c_str()).GetJstr(), byte_array_outputstream_obj, byte_array_outputstream_hash, jcmdid_array).i;
    if (ret == kCheckNext || ret == kCheckNever)
    {
        xwarn2(TSF"getLongLinkIdentifyCheckBuffer uin == 0, not ready");
        env->DeleteLocalRef(byte_array_outputstream_obj);
        env->DeleteLocalRef(byte_array_outputstream_hash);
        env->DeleteLocalRef(jcmdid_array);
        return ret;
    }

    jbyteArray ret_byte_array = NULL;
    ret_byte_array = (jbyteArray)JNU_CallMethodByName(env, byte_array_outputstream_obj, "toByteArray", "()[B").l;

    jbyteArray ret_byte_hash = NULL;
    ret_byte_hash = (jbyteArray)JNU_CallMethodByName(env, byte_array_outputstream_hash, "toByteArray", "()[B").l;


    jint* jcmdids = env->GetIntArrayElements(jcmdid_array, NULL);
    _cmdid = (int)jcmdids[0];
    env->ReleaseIntArrayElements(jcmdid_array, jcmdids, 0);
    env->DeleteLocalRef(jcmdid_array);


    if (ret_byte_hash != NULL) {
        jsize alen2 = env->GetArrayLength(ret_byte_hash);
        jbyte* ba2 = env->GetByteArrayElements(ret_byte_hash, NULL);
        _buffer_hash.Write(ba2, alen2);
        env->ReleaseByteArrayElements(ret_byte_hash, ba2, 0);
        env->DeleteLocalRef(ret_byte_hash);
    }
    if (ret_byte_array != NULL) {
        jsize alen = env->GetArrayLength(ret_byte_array);
        jbyte* ba = env->GetByteArrayElements(ret_byte_array, NULL);
        _identify_buffer.Write(ba, alen);
        env->ReleaseByteArrayElements(ret_byte_array, ba, 0);
        env->DeleteLocalRef(ret_byte_array);

    } else {
        xdebug2(TSF"the retByteArray is NULL");
    }

    //free the local reference
    env->DeleteLocalRef(byte_array_outputstream_obj);
    env->DeleteLocalRef(byte_array_outputstream_hash);

    return ret;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_OnLonglinkIdentifyResponse,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "onLongLinkIdentifyResponse",
//                     "(Ljava/lang/String;[B[B)Z")

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLonglinkIdentifyResponse, KC2Java, "onLongLinkIdentifyResponse", "(Ljava/lang/String;[B[B)Z")
bool StnManagerJniCallback::OnLonglinkIdentifyResponse(const std::string& _channel_id,
                                                       const AutoBuffer& _response_buffer,
                                                       const AutoBuffer& _identify_buffer_hash) {
//    jnienv_ptr env;
//
//    jbyteArray resp_buf_jba = NULL;
//
//    if (_response_buffer.Length() > 0) {
//        resp_buf_jba = JNU_Buffer2JbyteArray(env, _response_buffer);
//    } else {
//        // xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
//    }
//
//    jbyteArray identif_buf_jba = NULL;
//
//    if (_identify_buffer_hash.Length() > 0) {
//        identif_buf_jba = JNU_Buffer2JbyteArray(env, _identify_buffer_hash);
//    } else {
//        // xdebug2(TSF"the decodeBuffer.Lenght() <= 0");
//    }
//
//    bool ret = j2c_cast(c2j_call(jboolean,
//                                 callback_inst_,
//                                 kStnManagerJniCallback_OnLonglinkIdentifyResponse,
//                                 c2j_cast(jstring, _channel_id),
//                                 resp_buf_jba,
//                                 identif_buf_jba));
//
//    if (resp_buf_jba != NULL) {
//        env->DeleteLocalRef(resp_buf_jba);
//    }
//    if (identif_buf_jba != NULL) {
//        env->DeleteLocalRef(identif_buf_jba);
//    }
//    return ret;

    VarCache* cache_instance = VarCache::Singleton();

    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();

    jbyteArray data_jba = NULL;

    if (_response_buffer.Length() > 0) {
        data_jba = JNU_Buffer2JbyteArray(env, _response_buffer);
    } else {
        xdebug2(TSF"the respbuffer.Lenght() < = 0");
    }

    jbyteArray hash_jba = NULL;
    if (_identify_buffer_hash.Length() > 0) {
        hash_jba = JNU_Buffer2JbyteArray(env, _identify_buffer_hash);
    } else {
        xdebug2(TSF"the hashCodeBuffer.Lenght() < = 0");
    }

    jboolean ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnLonglinkIdentifyResponse, ScopedJstring(env, _channel_id.c_str()).GetJstr(),data_jba, hash_jba).z;

    if (data_jba != NULL) {
        JNU_FreeJbyteArray(env, data_jba);
    }

    if (hash_jba != NULL) {
        JNU_FreeJbyteArray(env, hash_jba);
    }

    return ret != 0;
}

//JNICAT_DEFINE_METHOD(kStnManagerJniCallback_RequestSync,
//                     "com/tencent/mars/stn/StnManager$CallBack",
//                     "requestSync",
//                     "()V")
DEFINE_FIND_METHOD(KC2Java_requestSync, KC2Java, "requestSync", "()V")
void StnManagerJniCallback::RequestSync() {
//    jnienv_ptr env;
//    c2j_call(void, callback_inst_, kStnManagerJniCallback_RequestSync);
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv *env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env,callback_inst_,  KC2Java_requestSync);
}

}  // namespace stn
}  // namespace mars