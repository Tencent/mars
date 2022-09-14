// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

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
 * app_logic.cc
 *
 *  Created on: 2016/3/3
 *      Author: caoshaokun
 */

#include "mars/app/app_logic.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#include "mars/comm/objc/data_protect_attr.h"
#endif

#include "mars/comm/xlogger/xlogger.h"
#include "mars/comm/bootrun.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"
#include "mars/comm/dns/dns.h"
#include "mars/baseevent/baseprjevent.h"
#include "app_manager.h"
#include "context.h"

using namespace mars::comm;

namespace mars {
namespace app {

void SetCallback(Callback* const callback) {
    BaseContext* context = CreateContext("default");
    context->GetAppManager()->SetCallback(callback);
}

#if !defined(ANDROID) || defined (CPP_CALL_BACK)

    static mars::comm::ProxyInfo sg_proxyInfo;
    static bool sg_gotProxy = false;
    static Mutex sg_slproxymutex;
    static Thread sg_slproxyThread;
    static uint64_t sg_slporxytimetick = gettickcount();
    static int sg_slproxycount = 0;



    static void __GetProxyInfo(const std::string& _host, uint64_t _timetick) {
        BaseContext* context = CreateContext("default");
        context->GetAppManager()->GetProxyInfo(_host, _timetick);
    }

#if TARGET_OS_IPHONE
    static void __ClearProxyInfo() {
        BaseContext* context = CreateContext("default");
        context->GetAppManager()->ClearProxyInfo();
    }

    static void __InitbindBaseprjevent() {
        GetSignalOnNetworkChange().connect(&__ClearProxyInfo);
    }
    BOOT_RUN_STARTUP(__InitbindBaseprjevent);
#endif

    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetProxyInfo(_host);
    }

    std::string GetAppFilePath() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetAppFilePath();
    }

	AccountInfo GetAccountInfo() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetAccountInfo();
        }

	std::string GetUserName() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetUserName();
	}

	std::string GetRecentUserName() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetRecentUserName();
	}

	unsigned int GetClientVersion() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetClientVersion();
	}

	DeviceInfo GetDeviceInfo() {
        BaseContext* context = CreateContext("default");
        return context->GetAppManager()->GetDeviceInfo();
    }

#endif

}
}


