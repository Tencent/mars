//
//  longlink_identify_checker.cc
//  PublicComponent
//
//  Created by 刘粲 on 13-12-6.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#include "longlink_identify_checker.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"

using namespace mars::stn;

LongLinkIdentifyChecker::LongLinkIdentifyChecker()
:has_checked_(false)
, cmd_id_(0)
, taskid_(0)
{
}

LongLinkIdentifyChecker::~LongLinkIdentifyChecker()
{
}


bool LongLinkIdentifyChecker::GetIdentifyBuffer(AutoBuffer &_buffer, uint32_t &_cmdid)
{
    if (has_checked_) return false;
    
    hash_code_buffer_.Reset();
    _buffer.Reset();

    IdentifyMode mode = (IdentifyMode)GetLonglinkIdentifyCheckBuffer(_buffer, hash_code_buffer_, (int&)_cmdid);

    switch (mode)
    {
    case kCheckNever:
        {
            has_checked_ = true;
        }
        break;
    case kCheckNext:
        {
            has_checked_ = false;
        }
        break;
    case kCheckNow:
        {
            cmd_id_ = _cmdid;
            return true;
        }
        break;
    default:
        xassert2(false);
    }
    
    return false;
}

void LongLinkIdentifyChecker::SetSeq(uint32_t  _taskid)
{
    taskid_ = _taskid;
}

bool LongLinkIdentifyChecker::IsIdentifyResp(uint32_t _taskid)
{
    if (taskid_ == _taskid && taskid_ != 0) return true;
    return false;
}

bool LongLinkIdentifyChecker::OnIdentifyResp(AutoBuffer& _buffer)
{
    xinfo2(TSF"identifycheck(synccheck) resp");
    bool ret = ::OnLonglinkIdentifyResponse(_buffer, hash_code_buffer_);
    taskid_ = 0;
    if (ret)
    {
        has_checked_ = true;
        return true;
    }
    return false;
}


void LongLinkIdentifyChecker::Reset()
{
    has_checked_ = false;
    taskid_ = 0;
    cmd_id_ = 0;
    hash_code_buffer_.Reset();
}
