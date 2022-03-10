//
//  LogLevelDef.h
//  MMCommon
//
//  Created by yanyang on 14-4-4.
//  Copyright (c) 2014年 goldenliu. All rights reserved.
//

#ifndef MMCommon_LogLevelDef_h
#define MMCommon_LogLevelDef_h

typedef enum
{
    ELevelAll = 0,
    ELevelVerbose = 0,
    ELevelDebug,    // Detailed information on the flow through the system.
    ELevelInfo,     // Interesting runtime events (startup/shutdown), should be conservative and keep to a minimum.
    ELevelWarn,     // Other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
    ELevelError,    // Other runtime errors or unexpected conditions.
    ELevelFatal,    // Severe errors that cause premature termination.
    ELevelNone,     // Special level used to disable all log messages.
}WTLogLevel;

#endif
