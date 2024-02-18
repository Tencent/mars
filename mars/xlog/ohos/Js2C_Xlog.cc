//
// Created on 2024/1/17.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "napi/native_api.h"
#include "napi_utils.h"
#include "xlogger.h"
#include "xlogger_interface.h"
#include <bits/alltypes.h>
#include "hilog/log.h"
#define LOG_DOMAIN 0x0201
#define LOG_TAG "MY_TAG"
const int32_t STR_DEFAULT_SIZE = 2048;
#define LONGTHREADID2INT(a) ((a >> 32) ^ ((a & 0xFFFF)))
static napi_value newXlogInstance(napi_env env, napi_callback_info info) {
    size_t argc = 9;
    napi_value args[9] = {nullptr};

    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "newXlogInstance");
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int level;
    napi_get_value_int32(env, args[0], &level);
    int mode;
    napi_get_value_int32(env, args[1], &mode);
    std::string logDir;
    NapiUtil::JsValueToString(env, args[2], STR_DEFAULT_SIZE, logDir);
    std::string nameprefix;
    NapiUtil::JsValueToString(env, args[3], STR_DEFAULT_SIZE, nameprefix);
    std::string pubkey;
    NapiUtil::JsValueToString(env, args[4], STR_DEFAULT_SIZE, pubkey);
    int compressmode;
    napi_get_value_int32(env, args[5], &compressmode);
    int compresslevel;
    napi_get_value_int32(env, args[6], &compresslevel);
    std::string cachedir;
    NapiUtil::JsValueToString(env, args[7], STR_DEFAULT_SIZE, cachedir);
    int cachedays;
    napi_get_value_int32(env, args[8], &cachedays);

    mars::xlog::XLogConfig config = {
        (mars::xlog::TAppenderMode)mode,         logDir.c_str(), nameprefix.c_str(), pubkey.c_str(),
        (mars::xlog::TCompressMode)compressmode, compresslevel,  cachedir.c_str(),   cachedays};
    mars::comm::XloggerCategory *category = mars::xlog::NewXloggerInstance(config, (TLogLevel)level);
    if (nullptr == category) {
        return 0;
    }
    OH_LOG_Print(LOG_APP,LOG_WARN, LOG_DOMAIN, LOG_TAG, "[%s:%d, %s]:%s",
                 "a",
                 0,
                 "a",
                 "newXlogInstance");
    napi_value categoryPointer;
    napi_create_int64(env, reinterpret_cast<uintptr_t>(category), &categoryPointer);
    return categoryPointer;
}
static napi_value getXlogInstance(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    std::string nameprefix;
    NapiUtil::JsValueToString(env, args[0], STR_DEFAULT_SIZE, nameprefix);
    mars::comm::XloggerCategory *category = mars::xlog::GetXloggerInstance(nameprefix.c_str());
    if (nullptr == category) {
        return 0;
    }
    napi_value result;
    napi_create_int64(env, reinterpret_cast<uintptr_t>(category), &result);
    return result;
}
static napi_value releaseXlogInstance(napi_env env, napi_callback_info info) {

    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    std::string nameprefix;
    NapiUtil::JsValueToString(env, args[0], STR_DEFAULT_SIZE, nameprefix);
    mars::xlog::ReleaseXloggerInstance(nameprefix.c_str());
    return nullptr;
}

static napi_value appenderOpen(napi_env env, napi_callback_info info) {
    size_t argc = 9;
    napi_value args[9] = {nullptr};
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "appenderOpen111");
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int level;
    napi_get_value_int32(env, args[0], &level);
    int mode;
    napi_get_value_int32(env, args[1], &mode);
    std::string logDir;
    NapiUtil::JsValueToString(env, args[2], STR_DEFAULT_SIZE, logDir);
    std::string nameprefix;
    NapiUtil::JsValueToString(env, args[3], STR_DEFAULT_SIZE, nameprefix);
    std::string pubkey;
    NapiUtil::JsValueToString(env, args[4], STR_DEFAULT_SIZE, pubkey);
    int compressmode;
    napi_get_value_int32(env, args[5], &compressmode);
    int compresslevel;
    napi_get_value_int32(env, args[6], &compresslevel);
    std::string cachedir;
    NapiUtil::JsValueToString(env, args[7], STR_DEFAULT_SIZE, cachedir);
    int cachedays;
    napi_get_value_int32(env, args[8], &cachedays);
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "appenderOpen2222");
    mars::xlog::XLogConfig config = {(mars::xlog::TAppenderMode)mode, logDir.c_str(),nameprefix.c_str(),pubkey.c_str(),
                                     (mars::xlog::TCompressMode)compressmode, compresslevel, cachedir.c_str(),   cachedays};
    appender_open(config);
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "appenderOpen3333");
    xlogger_SetLevel((TLogLevel)level);
    return nullptr;
}

static napi_value appenderClose(napi_env env, napi_callback_info info) { 
    mars::xlog::appender_close(); 
    return nullptr;
}
static napi_value appenderFlush(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    bool _is_sync;
    napi_get_value_bool(env, args[1], &_is_sync);
    mars::xlog::Flush(_log_instance_ptr, _is_sync);
    return nullptr;
}
static napi_value logWrite2(napi_env env, napi_callback_info info) {
    size_t argc = 10;
    napi_value args[10] = {nullptr};
OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "logWriter2");
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int level;
    napi_get_value_int32(env, args[1], &level);
    
    std::string tag;
    NapiUtil::JsValueToString(env, args[2], STR_DEFAULT_SIZE, tag);
    std::string filename;
    NapiUtil::JsValueToString(env, args[3], STR_DEFAULT_SIZE, filename);

    std::string funcname;
    NapiUtil::JsValueToString(env, args[4], STR_DEFAULT_SIZE, funcname);

    int line;
    napi_get_value_int32(env, args[5], &line);

    int64_t pid;
    napi_get_value_int64(env, args[6], &pid);

    int64_t tid;
    napi_get_value_int64(env, args[7], &tid);

    int maintid;
    napi_get_value_int32(env, args[8], &maintid);

    std::string log;
    NapiUtil::JsValueToString(env, args[9], STR_DEFAULT_SIZE, log);


    XLoggerInfo xlog_info = XLOGGER_INFO_INITIALIZER;
    gettimeofday(&xlog_info.timeval, NULL);
    xlog_info.level = (TLogLevel)level;
    xlog_info.line = (int)line;
    xlog_info.pid = (int)pid;
    xlog_info.tid = LONGTHREADID2INT(tid);
    xlog_info.maintid = LONGTHREADID2INT(maintid);

    xlog_info.tag = NULL == tag.c_str() ? "" : tag.c_str();
    xlog_info.filename = NULL == filename.c_str() ? "" : filename.c_str();
    xlog_info.func_name = NULL == funcname.c_str() ? "" : funcname.c_str();
OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "logWriter2 11111");
    mars::xlog::XloggerWrite(_log_instance_ptr, &xlog_info, NULL == log.c_str() ? "NULL == log" : log.c_str());
OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "logWriter2 22222");
    return nullptr;
}
static napi_value getLogLevel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    napi_value result;
    napi_create_int32(env, mars::xlog::GetLevel(_log_instance_ptr), &result);
    return result;
}
static napi_value setAppenderMode(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);

    int mode;
    napi_get_value_int32(env, args[1], &mode);

    mars::xlog::SetAppenderMode(_log_instance_ptr, (mars::xlog::TAppenderMode)mode);
    return nullptr;
}
static napi_value setConsoleLogOpen(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _is_open;
    napi_get_value_int32(env, args[1], &_is_open);

    mars::xlog::SetConsoleLogOpen(_log_instance_ptr, _is_open);
    
    return nullptr;
}

static napi_value setMaxFileSize(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _max_size;
    napi_get_value_int32(env, args[1], &_max_size);

    mars::xlog::SetMaxFileSize(_log_instance_ptr, _max_size);
    return nullptr;
}
static napi_value setMaxAliveTime(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _max_time;
    napi_get_value_int32(env, args[1], &_max_time);

    mars::xlog::SetMaxAliveTime(_log_instance_ptr, _max_time);
    return nullptr;
}
EXTERN_C_START 
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"newXlogInstance", nullptr, newXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getXlogInstance", nullptr, getXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"releaseXlogInstance", nullptr, releaseXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getLogLevel", nullptr, getLogLevel, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"appenderClose", nullptr, appenderClose, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"appenderFlush", nullptr, appenderFlush, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logWrite2", nullptr, logWrite2, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setAppenderMode", nullptr, setAppenderMode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setConsoleLogOpen", nullptr, setConsoleLogOpen, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"appenderOpen", nullptr, appenderOpen, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setMaxFileSize", nullptr, setMaxFileSize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setMaxAliveTime", nullptr, setMaxAliveTime, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    OH_LOG_Print(LOG_APP, LOG_WARN, LOG_DOMAIN, LOG_TAG, "init ...");
    return exports;
}
EXTERN_C_END

    static napi_module demoModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = Init,
        .nm_modname = "xlog",
        .nm_priv = ((void *)0),
        .reserved = {0},
    };

    extern "C" __attribute__((constructor)) void RegisterXlogModule(void) { napi_module_register(&demoModule); }

void ExportXlog() {
}