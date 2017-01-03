// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.



#include "tickcount.h"
#include "time_utils.h"

static uint64_t sg_tick_start = ::gettickcount();
static const uint64_t sg_tick_init = 2000000000;

tickcount_t::tickcount_t(bool _now)
:tickcount_(0)
{
    if (_now) gettickcount();
}

tickcount_t& tickcount_t::gettickcount()
{
    tickcount_ = sg_tick_init + ::gettickcount() - sg_tick_start;
    return *this;
}
