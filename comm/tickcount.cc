/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

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
