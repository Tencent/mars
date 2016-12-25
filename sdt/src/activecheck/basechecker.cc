/*
 * basechecker.cc
 *
 *  Created on: 2014/06/24
 *      Author: renlibin caoshaokun
 *  Copyright © 2014 Tencent. All rights reserved.
 */

#include "basechecker.h"

#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger.h"

using namespace mars::sdt;

BaseChecker::BaseChecker() {
    xverbose_function();
}

BaseChecker::~BaseChecker() {
    xverbose_function();
}

int BaseChecker::StartDoCheck(CheckRequestProfile& _check_request) {
    xinfo_function();
    // timeout and finish net checker.
    if (_check_request.total_timeout <= 0) {
        xinfo2(TSF"req.total_timeout_=%_, check finish!", _check_request.total_timeout);
        _check_request.check_status = kCheckFinish;
        return 0;
    }
    __DoCheck(_check_request);
    return 1;
}

int BaseChecker::CancelDoCheck() {
    xverbose_function();
    return 1;
}

void BaseChecker::__DoCheck(CheckRequestProfile& _check_request) {
    xverbose_function();
}
