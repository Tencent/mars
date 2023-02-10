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

DEFINE_FIND_METHOD(kStnManagerJniCallback_MakesureAuthed, KC2Java, "makesureAuthed", "(Ljava/lang/String;Ljava/lang/String;)Z")
bool StnManagerJniCallback::MakesureAuthed(const std::string& _host, const std::string& _user_id) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jboolean ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_MakesureAuthed, ScopedJstring(env, _host.c_str()).GetJstr(), ScopedJstring(env, _user_id.c_str()).GetJstr()).z;
    return ret;
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_TrafficData, KC2Java, "trafficData", "(II)V")
void StnManagerJniCallback::TrafficData(ssize_t _send, ssize_t _recv) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_TrafficData, (jint)_send, (jint)_recv);
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnNewDns, KC2Java, "onNewDns", "(Ljava/lang/String;Z)[Ljava/lang/String;")
std::vector<std::string> StnManagerJniCallback::OnNewDns(const std::string& host, bool longlink_host) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

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
    } else {
        xerror2(TSF "host is empty");
    }

    return iplist;
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnPush, KC2Java, "onPush", "(Ljava/lang/String;II[B[B)V")
void StnManagerJniCallback::OnPush(const std::string& _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jbyteArray data_jba = NULL;
    if (_body.Length() > 0) {
        data_jba = JNU_Buffer2JbyteArray(env, _body);
    } else {
        xdebug2(TSF "the data.Lenght() < = 0");
    }
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnPush, ScopedJstring(env, _channel_id.c_str()).GetJstr(), (jint)_cmdid, (jint)_taskid, data_jba, NULL);
    if (data_jba != NULL) {
        JNU_FreeJbyteArray(env, data_jba);
    }
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_Req2Buf, KC2Java, "req2Buf", "(ILjava/lang/Object;Ljava/lang/String;Ljava/io/ByteArrayOutputStream;[IILjava/lang/String;)Z")
bool StnManagerJniCallback::Req2Buf(uint32_t _taskid, void* const _user_context, const std::string& _user_id, AutoBuffer& outbuffer, AutoBuffer& extend, int& error_code, const int channel_select, const std::string& host) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jclass byte_array_outputstream_clz = cache_instance->GetClass(env, "java/io/ByteArrayOutputStream");
    jmethodID construct_mid = cache_instance->GetMethodId(env, byte_array_outputstream_clz, "<init>", "()V");
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
    jboolean ret = JNU_CallMethodByMethodInfo(env,
                                              callback_inst_,
                                              kStnManagerJniCallback_Req2Buf,
                                              (jint)_taskid,
                                              _user_context,
                                              ScopedJstring(env, _user_id.c_str()).GetJstr(),
                                              byte_array_output_stream_obj,
                                              // byte_array_extend_stream_obj,
                                              errcode_array,
                                              (jint)channel_select,
                                              ScopedJstring(env, host.c_str()).GetJstr()).z;
    //    if (byte_array_extend_stream_obj != NULL) {
    //        JNU_FreeJbyteArray(env, byte_array_extend_stream_obj);
    //    }
    xdebug2(TSF "mars2 kStnManagerJniCallback_Req2Buf ret:%_", ret);
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
            xdebug2(TSF "mars2 the retByteArray size %_", alen);
            jbyte* ba = env->GetByteArrayElements(ret_byte_array, NULL);
            outbuffer.Write(ba, alen);
            env->ReleaseByteArrayElements(ret_byte_array, ba, 0);
            env->DeleteLocalRef(ret_byte_array);
        } else {
            xdebug2(TSF "mars2 the retByteArray is null");
        }
    }

    jint* errcode = env->GetIntArrayElements(errcode_array, JNI_FALSE);
    error_code = errcode[0];
    env->ReleaseIntArrayElements(errcode_array, errcode, 0);
    env->DeleteLocalRef(errcode_array);

    env->DeleteLocalRef(byte_array_output_stream_obj);
    return ret;
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_Buf2Resp, KC2Java, "buf2Resp", "(ILjava/lang/Object;Ljava/lang/String;[B[II)I")
int StnManagerJniCallback::Buf2Resp(uint32_t _taskid, void* const _user_context, const std::string& _user_id, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jbyteArray resp_buf_jba = NULL;
    if (_inbuffer.Length() > 0) {
        resp_buf_jba = JNU_Buffer2JbyteArray(env, _inbuffer);
    } else {
        xdebug2(TSF "the decodeBuffer.Lenght() <= 0");
    }
    //    jbyteArray extend_buf_jba = NULL;
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
    xdebug2(TSF "mars2 StnManagerJniCallback::Buf2Res end");
    return ret;
}

DEFINE_FIND_CLASS(KC2JavaStnCgiProfile, "com/tencent/mars/stn/StnLogic$CgiProfile");
DEFINE_FIND_METHOD(kStnManagerJniCallback_OnTaskEnd, KC2Java, "onTaskEnd", "(ILjava/lang/Object;IILcom/tencent/mars/stn/StnLogic$CgiProfile;)I")
int StnManagerJniCallback::OnTaskEnd(uint32_t _taskid, void* const _user_context, const std::string& _user_id, int _error_type, int _error_code, const CgiProfile& _profile) {
    xdebug2(TSF "mars2 recieve task profile: %_, %_, %_", _profile.start_connect_time, _profile.start_send_packet_time, _profile.read_packet_finished_time);
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jclass cgiProfileCls = cache_instance->GetClass(env, KC2JavaStnCgiProfile);
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
    jfieldID fid_protocolType = env->GetFieldID(cgiProfileCls, "protocolType", "I");

    uint64_t tls_start_time = _profile.tls_handshake_successful_time == 0 ? 0 : _profile.start_tls_handshake_time;
    env->SetLongField(jobj_cgiItem, fid_taskStartTime, _profile.start_time);
    env->SetLongField(jobj_cgiItem, fid_startConnectTime, _profile.start_connect_time);
    env->SetLongField(jobj_cgiItem, fid_connectSuccessfulTime, _profile.connect_successful_time);
    env->SetLongField(jobj_cgiItem, fid_startHandshakeTime, tls_start_time);
    env->SetLongField(jobj_cgiItem, fid_handshakeSuccessfulTime, _profile.tls_handshake_successful_time);
    env->SetLongField(jobj_cgiItem, fid_startSendPacketTime, _profile.start_send_packet_time);
    env->SetLongField(jobj_cgiItem, fid_startReadPacketTime, _profile.start_read_packet_time);
    env->SetLongField(jobj_cgiItem, fid_readPacketFinishedTime, _profile.read_packet_finished_time);
    env->SetLongField(jobj_cgiItem, fid_rtt, _profile.rtt);
    env->SetIntField(jobj_cgiItem, fid_channelType, _profile.channel_type);
    env->SetIntField(jobj_cgiItem, fid_protocolType, _profile.transport_protocol);
    int ret = (int)JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnTaskEnd, (jint)_taskid, _user_context, (jint)_error_type, (jint)_error_code, jobj_cgiItem).i;
    return ret;
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_ReportConnectStatus, KC2Java, "reportConnectStatus", "(II)V")
void StnManagerJniCallback::ReportConnectStatus(int _status, int _longlink_status) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_ReportConnectStatus, (jint)_status, (jint)_longlink_status);
    xdebug2(TSF "all_connstatus = %0, longlink_connstatus = %_", _status, _longlink_status);
}

DEFINE_FIND_CLASS(StnErrCmdType, "com/tencent/mars/stn/ErrCmdType")
DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLongLinkNetworkError, KC2Java, "onLongLinkNetworkError", "(Lcom/tencent/mars/stn/ErrCmdType;ILjava/lang/String;I)V")
void StnManagerJniCallback::OnLongLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, uint16_t _port) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jclass stn_err_cmd_type_clazz = cache_instance->GetClass(env, StnErrCmdType);
    jmethodID construct_mid = cache_instance->GetStaticMethodId(env, stn_err_cmd_type_clazz, "forNumber", "(I)Lcom/tencent/mars/stn/ErrCmdType;");
    jobject stn_err_cmd_type_obj = (jobject)env->CallStaticObjectMethod(stn_err_cmd_type_clazz, construct_mid, (jint)_err_type);
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnLongLinkNetworkError, stn_err_cmd_type_obj, (jint)_err_code, ScopedJstring(env, _ip.c_str()).GetJstr(), (jint)_port);
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnShortLinkNetworkError, KC2Java, "onShortLinkNetworkError", "(Lcom/tencent/mars/stn/ErrCmdType;ILjava/lang/String;Ljava/lang/String;I)V")
void StnManagerJniCallback::OnShortLinkNetworkError(ErrCmdType _err_type, int _err_code, const std::string& _ip, const std::string& _host, uint16_t _port) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jclass stn_err_cmd_type_clazz = cache_instance->GetClass(env, StnErrCmdType);
    jmethodID construct_mid = cache_instance->GetStaticMethodId(env, stn_err_cmd_type_clazz, "forNumber", "(I)Lcom/tencent/mars/stn/ErrCmdType;");
    jobject stn_err_cmd_type_obj = (jobject)env->CallStaticObjectMethod(stn_err_cmd_type_clazz, construct_mid, (jint)_err_type);
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnShortLinkNetworkError, stn_err_cmd_type_obj, (jint)_err_code, ScopedJstring(env, _ip.c_str()).GetJstr(), ScopedJstring(env, _host.c_str()).GetJstr(), (jint)_port);
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLongLinkStatusChange, KC2Java, "onLongLinkStatusChange", "(I)V")
void StnManagerJniCallback::OnLongLinkStatusChange(int _status) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());

    JNIEnv* env = scope_jenv.GetEnv();
    JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnLongLinkStatusChange, (jint)_status);
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer, KC2Java, "getLongLinkIdentifyCheckBuffer", "(Ljava/lang/String;Ljava/io/ByteArrayOutputStream;Ljava/io/ByteArrayOutputStream;[I)I")
int StnManagerJniCallback::GetLonglinkIdentifyCheckBuffer(const std::string& _channel_id, AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jclass byte_array_output_stream_clz = cache_instance->GetClass(env, "java/io/ByteArrayOutputStream");
    jmethodID construct_mid = cache_instance->GetMethodId(env, byte_array_output_stream_clz, "<init>", "()V");
    jobject byte_array_outputstream_obj = env->NewObject(byte_array_output_stream_clz, construct_mid);
    jobject byte_array_outputstream_hash = env->NewObject(byte_array_output_stream_clz, construct_mid);

    jintArray jcmdid_array = env->NewIntArray(2);
    jint ret = 0;
    ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_GetLonglinkIdentifyCheckBuffer, ScopedJstring(env, _channel_id.c_str()).GetJstr(), byte_array_outputstream_obj, byte_array_outputstream_hash, jcmdid_array).i;
    if (ret == kCheckNext || ret == kCheckNever) {
        xwarn2(TSF "getLongLinkIdentifyCheckBuffer uin == 0, not ready");
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
        xdebug2(TSF "the retByteArray is NULL");
    }
    env->DeleteLocalRef(byte_array_outputstream_obj);
    env->DeleteLocalRef(byte_array_outputstream_hash);
    return ret;
}

DEFINE_FIND_METHOD(kStnManagerJniCallback_OnLonglinkIdentifyResponse, KC2Java, "onLongLinkIdentifyResponse", "(Ljava/lang/String;[B[B)Z")
bool StnManagerJniCallback::OnLonglinkIdentifyResponse(const std::string& _channel_id, const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash) {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    jbyteArray data_jba = NULL;
    if (_response_buffer.Length() > 0) {
        data_jba = JNU_Buffer2JbyteArray(env, _response_buffer);
    } else {
        xdebug2(TSF "the respbuffer.Lenght() < = 0");
    }

    jbyteArray hash_jba = NULL;
    if (_identify_buffer_hash.Length() > 0) {
        hash_jba = JNU_Buffer2JbyteArray(env, _identify_buffer_hash);
    } else {
        xdebug2(TSF "the hashCodeBuffer.Lenght() < = 0");
    }
    jboolean ret = JNU_CallMethodByMethodInfo(env, callback_inst_, kStnManagerJniCallback_OnLonglinkIdentifyResponse, ScopedJstring(env, _channel_id.c_str()).GetJstr(), data_jba, hash_jba).z;
    if (data_jba != NULL) {
        JNU_FreeJbyteArray(env, data_jba);
    }
    if (hash_jba != NULL) {
        JNU_FreeJbyteArray(env, hash_jba);
    }
    return ret != 0;
}

DEFINE_FIND_METHOD(KC2Java_requestSync, KC2Java, "requestSync", "()V")
void StnManagerJniCallback::RequestSync() {
    VarCache* cache_instance = VarCache::Singleton();
    ScopeJEnv scope_jenv(cache_instance->GetJvm());
    JNIEnv* env = scope_jenv.GetEnv();

    JNU_CallMethodByMethodInfo(env, callback_inst_, KC2Java_requestSync);
}

}  // namespace stn
}  // namespace mars