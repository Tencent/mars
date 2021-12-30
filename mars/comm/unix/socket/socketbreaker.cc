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

#include "comm/xlogger/xlogger.h"

namespace mars {
namespace comm {

SocketBreaker::SocketBreaker()
: create_success_(false),
broken_(false)
{
    pipes_[0] = -1;
    pipes_[1] = -1;
    ReCreate();
}

SocketBreaker::~SocketBreaker()
{
    Close();
}

bool SocketBreaker::IsCreateSuc() const
{
    return create_success_;
}

bool SocketBreaker::ReCreate()
{
    ScopedLock lock(mutex_);
    if(pipes_[1] >= 0)
        close(pipes_[1]);
    if(pipes_[0] >= 0)
        close(pipes_[0]);
    
    pipes_[0] = -1;
    pipes_[1] = -1;

    int Ret;
    Ret = pipe(pipes_);
    if (Ret == -1)
    {
        xerror2(TSF"pipe errno=%_,%_", errno, strerror(errno));
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    long flags0 = fcntl(pipes_[0], F_GETFL, 0);
    long flags1 = fcntl(pipes_[1], F_GETFL, 0);

    if (flags0 < 0 || flags1 < 0) {
        xerror2(TSF"get old flags error");
        close(pipes_[0]);
        close(pipes_[1]);
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    flags0 |= O_NONBLOCK;
    flags1 |= O_NONBLOCK;
    int ret0 = fcntl(pipes_[0], F_SETFL, flags0);
    int ret1 = fcntl(pipes_[1], F_SETFL, flags1);

    if ((-1 == ret1) || (-1 == ret0)) {
        xerror2(TSF"fcntl error");
        close(pipes_[0]);
        close(pipes_[1]);
        pipes_[0] = -1;
        pipes_[1] = -1;
        create_success_ = false;
        return create_success_;
    }

    create_success_ = true;
    return create_success_;
}

bool SocketBreaker::Break()
{
    ScopedLock lock(mutex_);

    if (broken_) return true;

    const char dummy = '1';
    int ret = (int)write(pipes_[1], &dummy, sizeof(dummy));
    broken_ = true;

    if (ret < 0 || ret != (int)sizeof(dummy))
    {
        xerror2(TSF"Ret:%_, fd %_ errno:(%_, %_)", ret, pipes_[1], errno, strerror(errno));
        broken_ =  false;
    }

    return broken_;
}

bool SocketBreaker::Break(int reason)
{
    reason_ = reason;
    return Break();
}

bool SocketBreaker::Clear()
{
    ScopedLock lock(mutex_);
    char dummy[128];
    int ret = (int)read(pipes_[0], dummy, sizeof(dummy));
    int lasterror = errno;
    if (ret < 0 && EWOULDBLOCK != lasterror){
        xerror2(TSF"clear pipe Ret=%_, errno:(%_, %_)", ret, lasterror, strerror(lasterror));
        return false;
    }

    broken_ =  false;
    return true;
}

void SocketBreaker::Close()
{
    ScopedLock lock(mutex_);
    broken_ =  true;
    if(pipes_[1] >= 0)
        close(pipes_[1]);
    if(pipes_[0] >= 0)
        close(pipes_[0]);
    pipes_[0] = -1;
    pipes_[1] = -1;
}

int SocketBreaker::BreakerFD() const
{
    ScopedLock lock(mutex_);
    return pipes_[0];
}

bool SocketBreaker::IsBreak() const
{
    ScopedLock lock(mutex_);
    return broken_;
}

int SocketBreaker::BreakReason() const{
    return reason_;
}

}
}
