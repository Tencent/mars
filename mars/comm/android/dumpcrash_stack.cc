// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * DumpCrashStack.cpp
 *
 *  Created on: 2012-9-28
 *      Author: yerungui
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
