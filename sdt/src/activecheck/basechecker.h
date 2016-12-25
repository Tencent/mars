/*
 * basechecker.h
 *
 *  Created on: 2016/06/24
 *      Author: renlibin caoshaokun
 *  Copyright © 2014 Tencent. All rights reserved.
 */

#ifndef SDT_SRC_ACTIVECHECK_BASECHEKCER_H_
#define SDT_SRC_ACTIVECHECK_BASECHEKCER_H_

#include <string>
#include <vector>

#include "mars/comm/platform_comm.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/thread/mutex.h"
#include "mars/sdt/sdt.h"

#include "netchecker_profile.h"

namespace mars {
namespace sdt {

class BaseChecker {
  public:
    BaseChecker();
    virtual ~BaseChecker();

  public:
    virtual int StartDoCheck(CheckRequestProfile& _check_request) = 0;
    virtual int CancelDoCheck() = 0;

  protected:
    virtual void __DoCheck(CheckRequestProfile& _check_request) = 0;
};

}}

#endif	//SDT_SRC_ACTIVECHECK_BASECHEKCER_H_
