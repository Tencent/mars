/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * flow_limit.h
 *
 *  Created on: 2012-9-3
 *      Author: yerungui
 */

#ifndef STN_SRC_FLOW_LIMIT_H_
#define STN_SRC_FLOW_LIMIT_H_

#include <stdint.h>

namespace mars {
namespace stn {

struct Task;

class FlowLimit {
  public:
    FlowLimit(bool _isactive);
    virtual ~FlowLimit();

    bool Check(const mars::stn::Task& _task, const void* _buffer, int _len);
    void Active(bool _isactive);

  private:
    void __FlashCurVol();

  private:
    int funnel_speed_;
    int cur_funnel_vol_;
    uint64_t time_lastflow_computer_;
};

}}

#endif // STN_SRC_FLOW_LIMIT_H_
