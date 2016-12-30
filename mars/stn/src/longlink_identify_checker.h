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
//  longlink_identify_checker.h
//  PublicComponent
//
//  Created by liucan on 13-12-6.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef STN_SRC_LONGLINK_IDENTIFY_CHECKER_H_
#define STN_SRC_LONGLINK_IDENTIFY_CHECKER_H_

#include <stdint.h>

#include "mars/comm/autobuffer.h"

class LongLinkIdentifyChecker {
  public:
    LongLinkIdentifyChecker();
    ~LongLinkIdentifyChecker();

    bool GetIdentifyBuffer(AutoBuffer& _buffer, uint32_t& _cmd_id);
    void SetSeq(uint32_t _seq);

    bool IsIdentifyResp(uint32_t _seq);
    bool OnIdentifyResp(AutoBuffer& _buffer);

    void Reset();


  private:
    bool has_checked_;
    uint32_t cmd_id_;
    uint32_t taskid_;
    AutoBuffer hash_code_buffer_;
};


#endif // STN_SRC_LONGLINK_IDENTIFY_CHECKER_H_
