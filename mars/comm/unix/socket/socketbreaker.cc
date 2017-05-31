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


SocketBreaker::SocketBreaker()
: create_success_(true),
broken_(false)
{
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
    pipes_[0] = -1;
    pipes_[1] = -1;

    int Ret;
    Ret = pipe(pipes_);
    xassert2(-1 != Ret, "pipe errno=%d", errno);

    if (Ret == -1)
    {
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
        xerror2(TSF"Ret:%_, errno:(%_, %_)", ret, errno, strerror(errno));
        broken_ =  false;
    }

    return broken_;
}

bool SocketBreaker::Clear()
{
    ScopedLock lock(mutex_);
    char dummy[128];
    int ret = (int)read(pipes_[0], dummy, sizeof(dummy));

    if (ret < 0)
    {
        xverbose2(TSF"Ret=%0", ret);
        return false;
    }

    broken_ =  false;
    return true;
}

void SocketBreaker::Close()
{
    broken_ =  true;
    if(pipes_[1] >= 0)
        close(pipes_[1]);
    if(pipes_[0] >= 0)
        close(pipes_[0]);
}

int SocketBreaker::BreakerFD() const
{
    return pipes_[0];
}

bool SocketBreaker::IsBreak() const
{
    return broken_;
}
