/**
 * created on : 2012-7-19
 * author : 闫国跃
 */

#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "mars/comm/xlogger/xloggerbase.h"
#include "mars/comm/xlogger/loginfo_extract.h"
#include "mars/comm/autobuffer.h"


//这里不能加日志，会导致循环调用
void ConsoleLog(const XLoggerInfo* _info, const char* _log)
{
	char result_log[2048] = {0};
    if (_info)
    {
        const char* filename = ExtractFileName(_info->filename);
        char strFuncName [128] = {0};
        ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));

        snprintf(result_log,  sizeof(result_log), "[%s, %s, %d]:%s", filename, strFuncName, _info->line, _log?_log:"NULL==log!!!");
        __android_log_write(_info->level+2, _info->tag?_info->tag:"", (const char*)result_log);
    }
    else
    {
    	snprintf(result_log,  sizeof(result_log) , "%s", _log?_log:"NULL==log!!!");
        __android_log_write(ANDROID_LOG_WARN, "", (const char*)result_log);
    }
    
}

