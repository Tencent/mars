//
//  tcpchecker.h
//  netchecker
//
//  Author: renlibin caoshaokun on 24/7/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#ifndef SDT_SRC_ACTIVECHECK_TCPCHEKER_H_
#define SDT_SRC_ACTIVECHECK_TCPCHEKER_H_

#include "mars/comm/autobuffer.h"
#include "mars/sdt/sdt.h"

#include "basechecker.h"

namespace mars {
namespace sdt {

class TcpChecker : public BaseChecker {
  public:
    TcpChecker();
    virtual ~TcpChecker();

    virtual int StartDoCheck(CheckRequestProfile& _check_request);
    virtual int CancelDoCheck();

  protected:
    virtual void __DoCheck(CheckRequestProfile& _check_request);

  private:
    void __NoopReq(AutoBuffer& noop_send);
    bool __NoopResp(const AutoBuffer& _packed, uint32_t& _cmdid, uint32_t& _seq, size_t& _package_len, AutoBuffer& _body);
};

}}

#endif	//SDT_SRC_ACTIVECHECK_TCPCHEKER_H_
