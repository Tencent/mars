//
//  longlink_identify_checker.h
//  PublicComponent
//
//  Created by 刘粲 on 13-12-6.
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
