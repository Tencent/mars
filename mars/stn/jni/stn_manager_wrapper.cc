//
// Created by Changpeng Pan on 2022/9/14.
//
#include "comm/jni/jnicat/jnicat_core.h"
#include "comm/jni/jnicat/jnicat_object_wrapper.h"
#include "mars/boot/context.h"
#include "mars/comm/jni/util/comm_function.h"
#include "mars/comm/jni/util/scoped_jstring.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"
#include "mars/stn/stn_manager.h"
#include "stn_manager_callback_wrapper.h"

using namespace mars::boot;

namespace mars {
namespace stn {

class JniStnManager {
 public:
    static void JniCreateStnManagerFromHandle(JNIEnv* env, jobject instance, jlong handle) {
        auto stn_manager_cpp = (StnManager*)j2c_cast(handle);
        auto stnManagerWrapper = new jnicat::JniObjectWrapper<StnManager>(stn_manager_cpp);
        stnManagerWrapper->instantiate(env, instance);
    }

    static void JniCreateStnManagerFromContext(JNIEnv* env, jobject instance, jobject context) {
        auto context_cpp = jnicat::JniObjectWrapper<Context>::object(env, context);
        auto stn_manager_cpp = new StnManager(context_cpp);
        auto stnManagerWrapper = new jnicat::JniObjectWrapper<StnManager>(stn_manager_cpp);
        stnManagerWrapper->instantiate(env, instance);
    }

    static void JniOnDestroyStnManager(JNIEnv* env, jobject instance) {
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        jnicat::JniObjectWrapper<Callback>::dispose(env, instance, "callbackHandle");
        stn_manager_cpp->SetCallback(nullptr);
        jnicat::JniObjectWrapper<StnManager>::dispose(env, instance);
    }

    static void JniSetCallback(JNIEnv* env, jobject instance, jobject callback) {
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        auto stnManagerJniCallback = new StnManagerJniCallback(env, callback);
        auto stnManagerCallbackWrapper = new jnicat::JniObjectWrapper<StnManagerJniCallback>(stnManagerJniCallback);
        stnManagerCallbackWrapper->instantiate(env, instance, "callbackHandle");
        stn_manager_cpp->SetCallback(stnManagerJniCallback);
    }

    static void JniSetLonglinkSvrAddr(JNIEnv* env,
                                      jobject instance,
                                      jstring _host,
                                      jintArray _ports,
                                      jstring _debug_ip) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);

        std::string host = (NULL == _host ? "" : ScopedJstring(env, _host).GetChar());
        std::string debug_ip = (NULL == _debug_ip ? "" : ScopedJstring(env, _debug_ip).GetChar());
        std::vector<uint16_t> ports;

        if (NULL != _ports && env->GetArrayLength(_ports) > 0) {
            int ports_len = env->GetArrayLength(_ports);
            jint* p_port = env->GetIntArrayElements(_ports, NULL);

            for (int i = 0; i < ports_len; ++i) {
                ports.push_back((uint16_t)(p_port[i]));
            }
            env->ReleaseIntArrayElements(_ports, p_port, 0);
        }
        stn_manager_cpp->SetLonglinkSvrAddr(host, ports, debug_ip);
    }

    static void JniSetShortlinkSvrAddr(JNIEnv* env, jobject instance, jint _port, jstring _debug_ip) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);

        std::string debug_ip = (NULL == _debug_ip ? "" : ScopedJstring(env, _debug_ip).GetChar());
        stn_manager_cpp->SetShortlinkSvrAddr(_port, debug_ip);
    }

    static void JniSetDebugIP(JNIEnv* env, jobject instance, jstring _host, jstring _debug_ip) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);

        std::string host = (NULL == _host ? "" : ScopedJstring(env, _host).GetChar());
        std::string debug_ip = (NULL == _debug_ip ? "" : ScopedJstring(env, _debug_ip).GetChar());
        stn_manager_cpp->SetDebugIP(host, debug_ip);
    }

    static void JniStartTask(JNIEnv* env, jobject instance, jobject _task) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        // get the field value of the netcmd
        jint taskid = JNU_GetField(env, _task, "taskID", "I").i;
        jint cmdid = JNU_GetField(env, _task, "cmdID", "I").i;
        jint channel_select = JNU_GetField(env, _task, "channelSelect", "I").i;
        jobject hostlist = JNU_GetField(env, _task, "shortLinkHostList", "Ljava/util/ArrayList;").l;
        jstring cgi = (jstring)JNU_GetField(env, _task, "cgi", "Ljava/lang/String;").l;

        jboolean send_only = JNU_GetField(env, _task, "sendOnly", "Z").z;
        jboolean need_authed = JNU_GetField(env, _task, "needAuthed", "Z").z;
        jboolean limit_flow = JNU_GetField(env, _task, "limitFlow", "Z").z;
        jboolean limit_frequency = JNU_GetField(env, _task, "limitFrequency", "Z").z;

        jint channel_strategy = JNU_GetField(env, _task, "channelStrategy", "I").i;
        jboolean network_status_sensitive = JNU_GetField(env, _task, "networkStatusSensitive", "Z").z;
        jint priority = JNU_GetField(env, _task, "priority", "I").i;

        jint retrycount = JNU_GetField(env, _task, "retryCount", "I").i;
        jint server_process_cost = JNU_GetField(env, _task, "serverProcessCost", "I").i;
        jint total_timetout = JNU_GetField(env, _task, "totalTimeout", "I").i;
        jstring report_arg = (jstring)JNU_GetField(env, _task, "reportArg", "Ljava/lang/String;").l;

        jboolean long_polling = JNU_GetField(env, _task, "longPolling", "Z").z;
        jint long_polling_timeout = JNU_GetField(env, _task, "longPollingTimeout", "I").i;

        jobject oHeaders = JNU_GetField(env, _task, "headers", "Ljava/util/Map;").l;
        std::map<std::string, std::string> headers = JNU_JObject2Map(env, oHeaders);
        jint client_sequence_id = JNU_GetField(env, _task, "clientSequenceId", "I").i;

        // init struct Task
        struct Task task(taskid);
        task.cmdid = cmdid;
        task.channel_select = channel_select;

        task.send_only = send_only;
        task.need_authed = need_authed;
        task.limit_flow = limit_flow;
        task.limit_frequency = limit_frequency;

        task.channel_strategy = channel_strategy;
        task.network_status_sensitive = network_status_sensitive;
        task.priority = priority;

        task.retry_count = retrycount;
        task.server_process_cost = server_process_cost;
        task.total_timeout = total_timetout;
        task.headers = headers;

        task.long_polling = long_polling;
        task.long_polling_timeout = long_polling_timeout;

        if (NULL != report_arg) {
            task.report_arg = ScopedJstring(env, report_arg).GetChar();
        }

        if (NULL != hostlist) {
            jclass cls_arraylist = env->GetObjectClass(hostlist);
            // method in class ArrayList
            jmethodID arraylist_get = env->GetMethodID(cls_arraylist, "get", "(I)Ljava/lang/Object;");
            jmethodID arraylist_size = env->GetMethodID(cls_arraylist, "size", "()I");
            jint len = env->CallIntMethod(hostlist, arraylist_size);
            for (int i = 0; i < len; i++) {
                jstring host = (jstring)env->CallObjectMethod(hostlist, arraylist_get, i);
                if (NULL != host) {
                    task.shortlink_host_list.push_back(ScopedJstring(env, host).GetChar());
                    env->DeleteLocalRef(host);
                }
            }
            env->DeleteLocalRef(hostlist);
        }

        if (NULL != cgi) {
            task.cgi = ScopedJstring(env, cgi).GetChar();
            env->DeleteLocalRef(cgi);
        }

        jobject _user_context = JNU_GetField(env, _task, "userContext", "Ljava/lang/Object;").l;
        if (NULL != _user_context) {
            task.user_context = env->NewGlobalRef(_user_context);
            env->DeleteLocalRef(_user_context);
        }
        task.client_sequence_id = client_sequence_id;
        stn_manager_cpp->StartTask(task);
    }

    static void JniStopTask(JNIEnv* env, jobject instance, jint _taskid) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->StopTask(_taskid);
    }

    static jboolean JniHasTask(JNIEnv* env, jobject instance, jint _taskid) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        return stn_manager_cpp->HasTask(_taskid);
    }

    static void JniRedoTask(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->RedoTasks();
    }

    static void JniTouchTasks(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->TouchTasks();
    }

    static void JniClearTask(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->ClearTasks();
    }

    static void JniReset(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->Reset();
    }

    static void JniResetAndInitEncoderVersion(JNIEnv* env, jobject instance, jint _packer_encoder_version) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->ResetAndInitEncoderVersion(_packer_encoder_version);
    }

    static void JniSetBackupIPs(JNIEnv* env, jobject instance, jstring _host, jobjectArray _objarray) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        std::string host = (NULL == _host ? "" : ScopedJstring(env, _host).GetChar());
        std::vector<std::string> backupip_list;

        jsize size = env->GetArrayLength(_objarray);
        for (int i = 0; i < size; i++) {
            jstring ip = (jstring)env->GetObjectArrayElement(_objarray, i);
            if (ip != NULL) {
                backupip_list.push_back(ScopedJstring(env, ip).GetChar());
            }
            env->DeleteLocalRef(ip);
        }
        stn_manager_cpp->SetBackupIPs(host, backupip_list);
    }

    static void JniMakesureLongLinkConnected(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->MakesureLonglinkConnected();
    }

    static void JniSetSignallingStrategy(JNIEnv* env, jobject instance, jlong _period, jlong _keep_time) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->SetSignallingStrategy((long)_period, (long)_keep_time);
    }

    static void JniKeepSignalling(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->KeepSignalling();
    }

    static void JniStopSignalling(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        stn_manager_cpp->StopSignalling();
    }

    static jint JniGenTaskID(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        return stn_manager_cpp->GenTaskID();
    }

    static jint JniGenSequenceId(JNIEnv* env, jobject instance) {
        xverbose_function();
        auto stn_manager_cpp = jnicat::JniObjectWrapper<StnManager>::object(env, instance);
        return stn_manager_cpp->GenSequenceId();
    }
};

static const JNINativeMethod kStnManagerJniMethods[] = {
    {"OnJniCreateStnManagerFromHandle", "(J)V", (void*)&mars::stn::JniStnManager::JniCreateStnManagerFromHandle},
    {"OnJniCreateStnManagerFromContext",
     "(Ljava/lang/Object;)V",
     (void*)&mars::stn::JniStnManager::JniCreateStnManagerFromContext},
    {"OnJniDestroyStnManager", "()V", (void*)&mars::stn::JniStnManager::JniOnDestroyStnManager},
    {"OnJniSetCallback", "(Ljava/lang/Object;)V", (void*)&mars::stn::JniStnManager::JniSetCallback},
    //{"OnJniSetCallback", "(Lcom/tencent/mars/stn/StnManager$CallBack;)V",
    //(void*)&mars::stn::JniStnManager::JniSetCallback},
    {"OnJniSetLonglinkSvrAddr",
     "(Ljava/lang/String;[ILjava/lang/String;)V",
     (void*)&mars::stn::JniStnManager::JniSetLonglinkSvrAddr},
    {"OnJniSetShortlinkSvrAddr", "(ILjava/lang/String;)V", (void*)&mars::stn::JniStnManager::JniSetShortlinkSvrAddr},
    {"OnJniSetDebugIP", "(Ljava/lang/String;Ljava/lang/String;)V", (void*)&mars::stn::JniStnManager::JniSetDebugIP},
    {"OnJniStartTask", "(Lcom/tencent/mars/stn/StnLogic$Task;)V", (void*)&mars::stn::JniStnManager::JniStartTask},
    {"OnJniStopTask", "(I)V", (void*)&mars::stn::JniStnManager::JniStopTask},
    {"OnJniHasTask", "(I)Z", (void*)&mars::stn::JniStnManager::JniHasTask},
    {"OnJniRedoTask", "()V", (void*)&mars::stn::JniStnManager::JniRedoTask},
    {"OnJniTouchTasks", "()V", (void*)&mars::stn::JniStnManager::JniTouchTasks},
    {"OnJniClearTask", "()V", (void*)&mars::stn::JniStnManager::JniClearTask},
    {"OnJniReset", "()V", (void*)&mars::stn::JniStnManager::JniReset},
    {"OnJniResetAndInitEncoderVersion", "(I)V", (void*)&mars::stn::JniStnManager::JniResetAndInitEncoderVersion},
    {"OnJniSetBackupIPs",
     "(Ljava/lang/String;[Ljava/lang/String;)V",
     (void*)&mars::stn::JniStnManager::JniSetBackupIPs},
    {"OnJniMakesureLongLinkConnected", "()V", (void*)&mars::stn::JniStnManager::JniMakesureLongLinkConnected},
    {"OnJniSetSignallingStrategy", "(JJ)V", (void*)&mars::stn::JniStnManager::JniSetSignallingStrategy},
    {"OnJniKeepSignalling", "()V", (void*)&mars::stn::JniStnManager::JniKeepSignalling},
    {"OnJniStopSignalling", "()V", (void*)&mars::stn::JniStnManager::JniStopSignalling},
    {"OnJniGenTaskID", "()I", (void*)&mars::stn::JniStnManager::JniGenTaskID},
    {"OnJniGenSequenceId", "()I", (void*)&mars::stn::JniStnManager::JniGenSequenceId},
};

static const size_t kStnManagerJniMethodsCount = sizeof(kStnManagerJniMethods) / sizeof(JNINativeMethod);
JNICAT_DEFINE_JNI_METHOD(kStnManagerInterface,
                         "com/tencent/mars/stn/StnManager",
                         kStnManagerJniMethods,
                         kStnManagerJniMethodsCount)

}  // namespace stn
}  // namespace mars

void ExportStnManager() {
}
