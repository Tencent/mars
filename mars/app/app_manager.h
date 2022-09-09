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
#include "base_app_manager.h"

using namespace mars::comm;

namespace mars {
namespace app {

class AppManager :public BaseAppManager {
 public:
    explicit AppManager();
    ~AppManager() override;

    void SetCallback(Callback* callback) override;
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) override;
    std::string GetAppFilePath() override;
    AccountInfo GetAccountInfo() override;
    std::string GetUserName() override;
    std::string GetRecentUserName() override;
    unsigned int GetClientVersion() override;
    DeviceInfo GetDeviceInfo() override;
    double GetOsVersion() override;

#ifdef NATIVE_CALLBACK
    void SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb) override;
#endif

 public:
    void __GetProxyInfo(const std::string& _host, uint64_t _timetick);

    //#if TARGET_OS_IPHONE
    void __ClearProxyInfo();
    //#endif

 private:
    Callback* callback_;
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
