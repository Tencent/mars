//
// Created by gjt on 10/17/24.
//

#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace xlog {
void ConsoleLog(const XLoggerInfo* _info, const char* _log) {
    if (NULL == _info || NULL == _log)
        return;
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"   // fatal
    };
    char strFuncName[128] = {0};
    ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
    const char* file_name = ExtractFileName(_info->filename);
    char log[16 * 1024] = {0};
    snprintf(log,
             sizeof(log),
             "[%s][%s][%s, %s, %d][%s\n",
             levelStrings[_info->level],
             NULL == _info->tag ? "" : _info->tag,
             file_name,
             strFuncName,
             _info->line,
             _log);
    printf("%s", log);
}
}  // namespace xlog
}  // namespace mars
