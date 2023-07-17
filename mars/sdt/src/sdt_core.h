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

#include <list>
#include <map>
#include <vector>

#include "mars/boot/context.h"
#include "mars/comm/singleton.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/sdt/constants.h"
#include "mars/sdt/netchecker_profile.h"
#include "mars/sdt/sdt.h"

namespace mars {
namespace sdt {

class BaseChecker;

class SdtCore {
 public:
    void StartCheck(CheckIPPorts& _longlink_items,
                    CheckIPPorts& _shortlink_items,
                    int _mode,
                    int _timeout = UNUSE_TIMEOUT);
    /*
     * Stop and cancel net check.
     */
    void CancelCheck();
    void CancelAndWait();

 public:
    SdtCore(mars::boot::Context* context);
    virtual ~SdtCore();

 private:
    void __InitCheckReq(CheckIPPorts& _longlink_items, CheckIPPorts& _shortlink_items, int _mode, int _timeout);
    void __Reset();

    // Run on.
    void __RunOn();

    void __DumpCheckResult();

 public:
    void SetHttpNetcheckCGI(std::string cgi);

 private:
    mars::boot::Context* context_;
    //  MessageQueue::ScopeRegister     async_reg_;
    comm::Thread thread_;

    std::list<BaseChecker*> check_list_;

    CheckRequestProfile check_request_;
    volatile bool cancel_;
    volatile bool checking_;
    comm::Mutex checking_mutex_;
    std::string netcheck_cgi_;
};

}  // namespace sdt
}  // namespace mars

#endif /* SDT_SRC_ACTIVECHECK_NETCHECKER_SERVICE_H_ */
