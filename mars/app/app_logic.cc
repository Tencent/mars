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
#include "mars/comm/macro.h"
#include "mars/boot/context.h"
#include "app_manager.h"

using namespace mars::boot;
using namespace mars::comm;

namespace mars {
namespace app {

//mars2
//static Callback* sg_callback = NULL;

void SetCallback(Callback* const callback) {
	/* mars2
	sg_callback = callback;
        */
    AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
    xassert2(NULL != app_manager, "mars2 app_manager is empty.");
    if (app_manager) {
        app_manager->SetCallback(callback);
    }
}

#if !defined(ANDROID) || defined (CPP_CALL_BACK)

/* mars2
NO_DESTROY static mars::comm::ProxyInfo sg_proxyInfo;
    static bool sg_gotProxy = false;
NO_DESTROY static Mutex sg_slproxymutex;
NO_DESTROY static Thread sg_slproxyThread;
    static uint64_t sg_slporxytimetick = gettickcount();
    static int sg_slproxycount = 0;
*/    

    
    static void __GetProxyInfo(const std::string& _host, uint64_t _timetick) {
        /* mars2
        xinfo_function(TSF"timetick:%_, host:%_", _timetick, _host);
    
        mars::comm::ProxyInfo proxy_info;
        if (!sg_callback->GetProxyInfo(_host, proxy_info)) {
            ScopedLock lock(sg_slproxymutex);
            if (_timetick != sg_slporxytimetick) {
                return;
            }
            ++ sg_slproxycount;
            return;
        }
        
        ScopedLock lock(sg_slproxymutex);
        if (_timetick != sg_slporxytimetick) {
            return;
        }
        
        ++ sg_slproxycount;
        
        sg_proxyInfo = proxy_info;
        
        if (mars::comm::kProxyNone == sg_proxyInfo.type || !sg_proxyInfo.ip.empty() || sg_proxyInfo.host.empty()) {
            sg_gotProxy = true;
            return;
        }
        
        std::string host = sg_proxyInfo.host;
        lock.unlock();
        
        static DNS s_dns;
        std::vector<std::string> ips;
        s_dns.GetHostByName(host, ips);
        
        if (ips.empty()) {
            return;
        }
        
        lock.lock();
        sg_proxyInfo.ip = ips.front();
        sg_gotProxy = true;
        */
        AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            app_manager->GetProxyInfo(_host, _timetick);
        }
    }

#if TARGET_OS_IPHONE
    static void __ClearProxyInfo() {
        /*mars2
        ScopedLock lock(sg_slproxymutex);
        sg_slporxytimetick = gettickcount();
        sg_slproxycount = 0;
        sg_gotProxy = false;
        sg_proxyInfo.type = mars::comm::kProxyNone;
        */
	AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            app_manager->ClearProxyInfo();
        }
    }

    static void __InitbindBaseprjevent() {
        GetSignalOnNetworkChange().connect(&__ClearProxyInfo);
    }
    BOOT_RUN_STARTUP(__InitbindBaseprjevent);
#endif
    
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) {
        /* mars2
        xassert2(sg_callback != NULL);
        
#if !TARGET_OS_IPHONE
        mars::comm::ProxyInfo proxy_info;
        sg_callback->GetProxyInfo(_host, proxy_info);
        return proxy_info;
#endif
        
        if (sg_gotProxy) {
            return sg_proxyInfo;
        }
        
        ScopedLock lock(sg_slproxymutex, false);
        if (!lock.timedlock(500))   return mars::comm::ProxyInfo();
        
        if (sg_slproxycount < 3 || 5 * 1000 > gettickspan(sg_slporxytimetick)) {
            sg_slproxyThread.start(boost::bind(&__GetProxyInfo, _host, sg_slporxytimetick));
        }
        
        if (sg_gotProxy) {
            return sg_proxyInfo;
        }
        
        return mars::comm::ProxyInfo();
        */
	AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            return app_manager->GetProxyInfo(_host);
        }
        return mars::comm::ProxyInfo();
    }

    std::string GetAppFilePath() {
        /* mars2
        xassert2(sg_callback != NULL);

        std::string path = sg_callback->GetAppFilePath();
#ifdef __APPLE__
        setAttrProtectionNone(path.c_str());
#endif

        return path;
        */
        AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            return app_manager->GetAppFilePath();
        }
        return "";
    }
    
	AccountInfo GetAccountInfo() {
            /* mars2
		xassert2(sg_callback != NULL);
		return sg_callback->GetAccountInfo();
            */
            AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
            xassert2(NULL != app_manager, "mars2 app_manager is empty.");
            if (app_manager) {
                return app_manager->GetAccountInfo();
            }
            return AccountInfo();
	}

	std::string GetUserName() {
		/*mars2
		xassert2(sg_callback != NULL);
		AccountInfo info = sg_callback->GetAccountInfo();
		return info.username;
		*/
		AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
                xassert2(NULL != app_manager, "mars2 app_manager is empty.");
                if (app_manager) {
                    return app_manager->GetUserName();
                }
                return "";
	}

	std::string GetRecentUserName() {
		/* mars2
		xassert2(sg_callback != NULL);
		return GetUserName();
		*/
	    AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
            xassert2(NULL != app_manager, "mars2 app_manager is empty.");
            if (app_manager) {
                return app_manager->GetRecentUserName();
            }
            return "";
	}

	unsigned int GetClientVersion() {
		/* mars2
		xassert2(sg_callback != NULL);
		return sg_callback->GetClientVersion();
		*/
		AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
                xassert2(NULL != app_manager, "mars2 app_manager is empty.");
                if (app_manager) {
                    return app_manager->GetClientVersion();
                }
                return 0;
	}


	DeviceInfo GetDeviceInfo() {
		/*mars2
		xassert2(sg_callback != NULL);
        
    static DeviceInfo device_info;
    if (!device_info.devicename.empty() || !device_info.devicetype.empty()) {
        return device_info;
    }
    
    device_info = sg_callback->GetDeviceInfo();
    return device_info;
		*/
		AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
                xassert2(NULL != app_manager, "mars2 app_manager is empty.");
                if (app_manager) {
                    return app_manager->GetDeviceInfo();
                }
                return DeviceInfo();
	}

#endif

    void UpdateAppConfig(Config _app_config) {
        AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            app_manager->UpdateAppConfig(_app_config);
        }
    }

    AppConfig* GetAppConfig() {
        AppManager* app_manager = Context::CreateContext("default")->GetManager<AppManager>();
        xassert2(NULL != app_manager, "mars2 app_manager is empty.");
        if (app_manager) {
            return app_manager->GetAppConfig();
        }
        return NULL;
    }

}
}


