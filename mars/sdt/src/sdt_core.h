// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.


/*
 * netchecker_service.h
 *
 *  Created on: 2014-6-17
 *      Author: renlibin caoshaokun
 */

#ifndef SDT_SRC_ACTIVECHECK_NETCHECKER_SERVICE_H_
#define SDT_SRC_ACTIVECHECK_NETCHECKER_SERVICE_H_

#include <map>
#include <vector>
#include <list>

#include "mars/comm/singleton.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/thread/mutex.h"
#include "mars/sdt/constants.h"
#include "mars/sdt/sdt.h"
#include "mars/sdt/netchecker_profile.h"

namespace mars {
namespace sdt {

class BaseChecker;

class SdtCore {
  public:
    SINGLETON_INTRUSIVE(SdtCore, new SdtCore, delete);

  public:

    void StartCheck(CheckIPPorts& _longlink_items, CheckIPPorts& _shortlink_items, int _mode, int _timeout = UNUSE_TIMEOUT);
    /*
     * Stop and cancel net check.
     */
    void CancelCheck();
    void CancelAndWait();

  private:
    SdtCore();
    virtual ~SdtCore();

    void __InitCheckReq(CheckIPPorts& _longlink_items, CheckIPPorts& _shortlink_items, int _mode, int _timeout);
    void __Reset();

    // Run on.
    void __RunOn();
    
    void __DumpCheckResult();

  private:
    //  MessageQueue::ScopeRegister     async_reg_;
    Thread thread_;

    std::list<BaseChecker*>   check_list_;

    CheckRequestProfile		  check_request_;
    volatile bool             cancel_;
    volatile bool             checking_;
    Mutex					  checking_mutex_;
};

}}

#endif /* SDT_SRC_ACTIVECHECK_NETCHECKER_SERVICE_H_ */
