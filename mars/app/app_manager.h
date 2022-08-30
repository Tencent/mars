//
// Created by Changpeng Pan on 2022/8/29.
//

#ifndef MMNET_APP_MANAGER_H
#define MMNET_APP_MANAGER_H

#include "mars/app/app.h"
#include "mars/comm/comm_data.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"

using namespace mars::comm;

namespace mars {
namespace app {

class AppManager {
 public:
    void SetCallback(Callback* const callback);
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host);
    std::string GetAppFilePath();
    AccountInfo GetAccountInfo();
    std::string GetUserName();
    std::string GetRecentUserName();
    unsigned int GetClientVersion();
    DeviceInfo GetDeviceInfo();
    double GetOsVersion();

#ifdef NATIVE_CALLBACK
    void SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb);
#endif

 public:
    void __GetProxyInfo(const std::string& _host, uint64_t _timetick);

    //#if TARGET_OS_IPHONE
    void __ClearProxyInfo();
    //#endif

 private:
    std::shared_ptr<Callback> callback_;
    bool got_proxy_ = false;
    mars::comm::ProxyInfo proxy_info_;
    Mutex slproxymutex_;
    Thread slproxythread_;
    uint64_t slproxytimetick_ = gettickcount();
    int slproxycount_ = 0;
};

}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_MANAGER_H
