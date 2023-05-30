// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  longlink_identify_checker.cc
//  PublicComponent
//
//  Created by liucan on 13-12-6.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#include "longlink_identify_checker.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/stn/stn.h"

#include "stn/proto/longlink_packer.h"
#include "mars/stn/stn_manager.h"

using namespace mars::stn;

LongLinkIdentifyChecker::LongLinkIdentifyChecker(mars::boot::Context* _context, mars::stn::LongLinkEncoder& _encoder, const std::string& _channel_id, bool _is_minorlong)
: context_(_context)
, has_checked_(false)
, cmd_id_(0)
, taskid_(0)
, encoder_(_encoder)
, channel_id_(_channel_id)
, is_minorlong_(_is_minorlong)
{ }

LongLinkIdentifyChecker::~LongLinkIdentifyChecker() { }

bool LongLinkIdentifyChecker::GetIdentifyBuffer(AutoBuffer &_buffer, uint32_t &_cmdid)
{
    if (has_checked_) return false;
    
    hash_code_buffer_.Reset();
    _buffer.Reset();

    if (is_minorlong_)
        _cmdid |= Task::kMinorLonglinkCmdMask;
    IdentifyMode mode = (IdentifyMode)context_->GetManager<StnManager>()->GetLonglinkIdentifyCheckBuffer(channel_id_, _buffer, hash_code_buffer_, (int32_t&)_cmdid);

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

void LongLinkIdentifyChecker::SetID(uint32_t  _taskid) { taskid_ = _taskid;}

bool LongLinkIdentifyChecker::IsIdentifyResp(uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _buffer, const AutoBuffer& _extend) const {
    return encoder_.longlink_identify_isresp(taskid_, _cmdid, _taskid, _buffer, _extend);
}

bool LongLinkIdentifyChecker::OnIdentifyResp(AutoBuffer& _buffer) {
    xinfo2(TSF"identifycheck(synccheck) resp");
    bool ret = context_->GetManager<StnManager>()->OnLonglinkIdentifyResponse(channel_id_, _buffer, hash_code_buffer_);
    taskid_ = 0;
    if (ret) {
        has_checked_ = true;
        return true;
    }
    return false;
}


void LongLinkIdentifyChecker::Reset() {
    has_checked_ = false;
    taskid_ = 0;
    cmd_id_ = 0;
    hash_code_buffer_.Reset();
}
