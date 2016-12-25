/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
/*
 * httpchecker.h
 *
 *  Created on: 2014-7-1
 *      Author: yanguoyue caoshaokun
 */

#ifndef SDT_SRC_ACTIVECHECK_HTTPCHEKCER_H_
#define SDT_SRC_ACTIVECHECK_HTTPCHEKCER_H_

#include <string>

#include "mars/sdt/sdt.h"

#include "basechecker.h"

namespace mars {
namespace sdt {

class HttpChecker : public BaseChecker {
  public:
    HttpChecker();
    virtual ~HttpChecker();

    virtual int StartDoCheck(CheckRequestProfile& _check_request);
    virtual int CancelDoCheck();

  protected:
    virtual void __DoCheck(CheckRequestProfile& _check_request);
};

}}

#endif /* SDT_SRC_ACTIVECHECK_HTTPCHEKCER_H_ */
