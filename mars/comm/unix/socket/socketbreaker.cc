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
 * socketbreaker.cpp
 *
 *  Created on: 2015-12-9
 *      Author: yerungui
 */

#include "socketbreaker.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstdint>

#include "comm/xlogger/xlogger.h"

namespace mars {
namespace comm {

SocketBreaker::SocketBreaker() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pipes_[0] = -1;
        pipes_[1] = -1;
    }

    ReCreate();
}

SocketBreaker::~SocketBreaker() {
    Close();
}

bool SocketBreaker::IsCreateSuc() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return create_success_;
}

bool SocketBreaker::ReCreate() {
    std::lock_guard<std::mutex> lock(mutex_);
    _Cleanup();

    if (pipe(pipes_) == -1) {
        xerror2(TSF "pipe errno %_,%_", errno, strerror(errno));
        return false;
    }

    int flags0 = fcntl(pipes_[0], F_GETFL, 0);
    int flags1 = fcntl(pipes_[1], F_GETFL, 0);
    if (flags0 == -1 || flags1 == -1) {
        xerror2(TSF "get old flags error");
        _Cleanup();
        return false;
    }

    int ret0 = fcntl(pipes_[0], F_SETFL, flags0 | O_NONBLOCK);
    int ret1 = fcntl(pipes_[1], F_SETFL, flags1 | O_NONBLOCK);
    if (ret0 == -1 || ret1 == -1) {
        xerror2(TSF "fcntl error %_,%_", errno, strerror(errno));
        _Cleanup();
        return false;
    }

    create_success_ = true;
    breaked_ = false;
    return true;
}

bool SocketBreaker::Break() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (breaked_)
        return true;
    if (!create_success_)
        return false;

    const char dummy = '1';
    ssize_t writebytes = write(pipes_[1], &dummy, sizeof(dummy));
    if (writebytes != sizeof(dummy)) {
        xerror2(TSF "write ret %_, fd %_ error %_,%_", writebytes, pipes_[1], errno, strerror(errno));
        return false;
    }

    breaked_ = true;
    return true;
}

bool SocketBreaker::Break(int reason) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        reason_ = reason;
    }

    return Break();
}

bool SocketBreaker::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!breaked_)
        return true;

    char dummy[1024];
    ssize_t readbytes = 0;
    for (;;) {
        ssize_t rv = read(pipes_[0], dummy, sizeof(dummy));
        if (rv > 0) {
            readbytes += rv;
            continue;
        }
        if (rv == 0) {  // zero indicates end of file
            if (readbytes > 0) {
                breaked_ = false;
                return true;
            }

            return false;
        }

        if (rv == -1) {  // On error, -1 is returned, and errno is set to indicate the error.
            if (errno == EWOULDBLOCK && readbytes > 0) {
                breaked_ = false;
                return true;
            }

            xerror2(TSF "read ret %_, fd %_ error %_,%_", rv, pipes_[0], errno, strerror(errno));
            return false;
        }
    }

    xassert2(false);
    return false;
}

bool SocketBreaker::PreciseBreak(uint32_t cookie) {
    std::lock_guard<std::mutex> lock(mutex_);

    ssize_t writebytes = write(pipes_[1], &cookie, sizeof(cookie));
    if (writebytes != sizeof(cookie)) {
        xerror2(TSF "write ret %_, fd %_ error %_,%_", writebytes, pipes_[1], errno, strerror(errno));
        return false;
    }

    xinfo2(TSF "pipe [%_,%_] write cookie %_", pipes_[0], pipes_[1], cookie);
    return true;
}

bool SocketBreaker::PreciseClear(uint32_t* cookie) {
    std::lock_guard<std::mutex> lock(mutex_);

    struct pollfd pipepfd = {pipes_[0], POLLIN | POLLHUP, 0};
    int ret = poll(&pipepfd, 1, 0);
    if (ret != 1) {
        xwarn2(TSF "fd %_ no POLLIN event.", pipes_[0]);
        return false;
    }

    ssize_t rv = read(pipes_[0], cookie, sizeof(uint32_t));
    if (rv != sizeof(uint32_t)) {
        xerror2(TSF "read ret %_, fd %_ error %_,%_", rv, pipes_[0], errno, strerror(errno));
        return false;
    }

    xinfo2(TSF "pipe [%_,%_] read cookie %_", pipes_[0], pipes_[1], *cookie);
    return true;
}

void SocketBreaker::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    _Cleanup();
}

int SocketBreaker::BreakerFD() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pipes_[0];
}

bool SocketBreaker::IsBreak() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return breaked_;
}

int SocketBreaker::BreakReason() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return reason_;
}

void SocketBreaker::_Cleanup() {
    if (pipes_[1] >= 0)
        close(pipes_[1]);
    if (pipes_[0] >= 0)
        close(pipes_[0]);
    pipes_[0] = -1;
    pipes_[1] = -1;
    create_success_ = false;
    breaked_ = false;
    reason_ = 0;
}

}  // namespace comm
}  // namespace mars
