//
//  dnschecker.h
//  netchecker
//
//  Author: renlibin caoshaokun on 24/6/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#ifndef SDT_SRC_ACTIVECHECK_DNSCHEKCER_H_
#define SDT_SRC_ACTIVECHECK_DNSCHEKCER_H_

#include "mars/sdt/sdt.h"

#include "basechecker.h"

namespace mars {
namespace sdt {

class DnsChecker : public BaseChecker {
  public:
    DnsChecker();
    virtual ~DnsChecker();

    virtual int StartDoCheck(CheckRequestProfile& _check_request);
    virtual int CancelDoCheck();

  protected:
    virtual void __DoCheck(CheckRequestProfile& _check_request);
};

}}

#endif	// SDT_SRC_ACTIVECHECK_DNSCHEKCER_H_
