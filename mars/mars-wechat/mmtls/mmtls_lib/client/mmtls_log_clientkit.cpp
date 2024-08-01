/*
        Only use in client. It depends on xlog.
*/
#include <stdlib.h>

#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_log.h"

using namespace mmtls;

mmtls::ILog mmtls::g_Log;

static TLogLevel AdaptLogLevel(const int level) {
    TLogLevel ret_level = kLevelAll;
    switch (level) {
        case ILog::MMTLS_LOG_DISABLE:
            ret_level = kLevelNone;
            break;
        case ILog::MMTLS_LOG_INFO:
            // case ILog::MMTLS_LOG_IMPT:
            ret_level = kLevelInfo;
            break;
        // case ILog::MMTLS_LOG_INIT:
        case ILog::MMTLS_LOG_ERR:
            // case ILog::MMTLS_LOG_MSG:
            ret_level = kLevelError;
            break;
        case ILog::MMTLS_LOG_DEBUG:
            ret_level = kLevelDebug;
            break;
        case ILog::MMTLS_LOG_MONITOR:
        case ILog::MMTLS_LOG_IN:
            ret_level = kLevelVerbose;
            break;
        case ILog::MMTLS_LOG_ALL:
            ret_level = kLevelAll;
            break;
        default:
            break;
    }
    return ret_level;
}

struct mmtls::LogImpt_t {
    char* module_name_;
};

ILog::ILog() {
    impt_ = (struct mmtls::LogImpt_t*)malloc(sizeof(struct mmtls::LogImpt_t));
    memset(impt_, 0, sizeof(struct mmtls::LogImpt_t));
}

ILog::~ILog() {
    Destory();
}

int ILog::Init(const char* module_name, const int logLevel, const char* path, const int max_size) {
    if (impt_ && !impt_->module_name_) {
        size_t len = (strlen(module_name) >= 1024) ? 1024 : strlen(module_name);
        impt_->module_name_ = (char*)malloc(len);
        memset(impt_->module_name_, 0, len);
        strncpy(impt_->module_name_, module_name, len - 1);
    }
    return 0;
}

void ILog::Destory() {
    if (impt_) {
        if (impt_->module_name_) {
            free(impt_->module_name_);
            impt_->module_name_ = NULL;
        }
        free(impt_);
        impt_ = NULL;
    }
}

bool ILog::IsDiscard(int level) const {
#ifdef DEBUG
    return false;
#else
    return (level >= MMTLS_LOG_DEBUG);
#endif
}

void ILog::Log(const int level,
               const char* func,
               const char* file_basename,
               unsigned int line,
               const char* format,
               ...) {
    char buf[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    const char* tag = (NULL != impt_ && NULL != impt_->module_name_) ? (impt_->module_name_) : "MMTLS";
    xlogger(AdaptLogLevel(level), tag, file_basename, func, line, buf);
}
