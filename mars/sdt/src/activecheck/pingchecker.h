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
//  pingchecker.h
//  netchecker
//
//  Author: renlibin caoshaokun on 25/6/14.
//  Copyright (c) 2014 Tencent. All rights reserved.
//

#ifndef SDT_SRC_ACTIVECHECK_PINGCHEKER_H_
#define SDT_SRC_ACTIVECHECK_PINGCHEKER_H_

#include "mars/sdt/sdt.h"

#include "basechecker.h"

namespace mars {
namespace sdt {

class PingChecker : public BaseChecker {
  public:
    PingChecker();
    virtual ~PingChecker();

    virtual int StartDoCheck(CheckRequestProfile& _check_request);
    virtual int CancelDoCheck();

  protected:
    virtual void __DoCheck(CheckRequestProfile& _check_request);
};

}}

#endif	// SDT_SRC_ACTIVECHECK_PINGCHEKER_H_
