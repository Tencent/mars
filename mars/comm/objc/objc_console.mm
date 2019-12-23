
#import <Foundation/Foundation.h>
#include "comm/xlogger/xlogger.h"
#include "comm/xlogger/loginfo_extract.h"
#import "comm/objc/scope_autoreleasepool.h"

void ConsoleLog(const XLoggerInfo* _info, const char* _log)
{
    SCOPE_POOL();

    if (NULL==_info || NULL==_log) return;
    
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"  // fatal
    };
    
    char strFuncName[128]  = {0};
    ExtractFunctionName(_info->func_name, strFuncName, sizeof(strFuncName));
    
    const char* file_name = ExtractFileName(_info->filename);
    
    char log[16 * 1024] = {0};
    snprintf(log, sizeof(log), "[%s][%s][%s, %s, %d][%s", levelStrings[_info->level], NULL == _info->tag ? "" : _info->tag, file_name, strFuncName, _info->line, _log);
    
    NSLog(@"%@", [NSString stringWithUTF8String:log]);
}
