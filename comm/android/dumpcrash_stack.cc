/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * DumpCrashStack.cpp
 *
 *  Created on: 2012-9-28
 *      Author: 叶润桂
 */

#include "dumpcrash_stack.h"

#include "assert/__assert.h"

static const char gs_crash_dump_header[] =
    "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***";

enum NDK_CRASH_PARSER_STATE {
    EXPECTS_CRASH_DUMP,
    EXPECTS_CRASH_DUMP_HEADER,
    EXPECTS_CRASH_DUMP_CONTENT,
    EXPECTS_CRASH_DUMP_END,
};

static int __ParseStack(FILE* file,  NDK_CRASH_PARSER_STATE& _state, char _strcache[2048], std::string& strout) {
    ASSERT(file);

    if (EXPECTS_CRASH_DUMP_END == _state)
        return -1;

    if (EXPECTS_CRASH_DUMP_HEADER == _state) {
        _state = EXPECTS_CRASH_DUMP_CONTENT;
        strout += _strcache;
    }

    while (fgets(_strcache, 2048, file)) {
        if (strstr(_strcache, gs_crash_dump_header) != NULL) {
            ASSERT(EXPECTS_CRASH_DUMP_HEADER != _state);

            if (EXPECTS_CRASH_DUMP_CONTENT == _state) {
                _state = EXPECTS_CRASH_DUMP_HEADER;
                return 0;
            }

            _state = EXPECTS_CRASH_DUMP_CONTENT;
        } else if (EXPECTS_CRASH_DUMP == _state) {
            continue;
        }

        strout += _strcache;
    }

    if (EXPECTS_CRASH_DUMP == _state)
        return -1;

    _state = EXPECTS_CRASH_DUMP_END;
    return 0;
}

void DumpCrashStack::Dump(const std::string& _processname) {
    vecdump_.clear();

    char processname[256];
    NDK_CRASH_PARSER_STATE state = EXPECTS_CRASH_DUMP;
    char  strcache[2048];

    snprintf(processname, sizeof(processname), ">>> %s <<<", _processname.c_str());

    FILE* stream = popen("logcat -d -v time -s 'DEBUG:I'", "r");    // 将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream

    std::string strstack;

    while (-1 != __ParseStack(stream, state, strcache, strstack)) {
        if (!_processname.empty() && std::string::npos == strstack.find(processname, 0)) {
            strstack.clear();
            continue;
        }

        vecdump_.push_back(strstack);
        strstack.clear();
    }

    pclose(stream);
}

const std::vector<std::string>& DumpCrashStack::StackList() const {
    return vecdump_;
}
