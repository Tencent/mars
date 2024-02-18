#include "hilog/log.h"
#include <stdio.h>
#include <string.h>

#include "mars/comm/autobuffer.h"
#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/xlogger/xloggerbase.h" 


namespace mars {
namespace xlog {
#define LOG_DOMAIN 0x0201
//这里不能加日志，会导致循环调用
void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    char result_log[16 * 1024] = {0};
    if (_info) {
        const char* filename = ExtractFileName(_info->filename);
        const char* strFuncName = NULL == _info->func_name ? "" : _info->func_name;
        OH_LOG_Print(LOG_APP,LOG_WARN, LOG_DOMAIN, _info->tag ? _info->tag : "", "[%s:%d, %s]:%s",
                 filename,
                 _info->line,
                 strFuncName,
                 _log ? _log : "NULL==log!!!");
    } else {
        snprintf(result_log, sizeof(result_log), "%s", _log ? _log : "NULL==log!!!");
        OH_LOG_Print(LOG_APP,LOG_WARN, LOG_DOMAIN, "", "%s", result_log);
    }
}

}  // namespace xlog
}  // namespace mars