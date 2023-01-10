//
// Created by Changpeng Pan on 2022/8/29.
//

#ifndef MMNET_APP_MANAGER_H
#define MMNET_APP_MANAGER_H

#include <memory>

#include "mars/app/app.h"
#include "mars/boot/base_app_manager.h"
#include "mars/boot/base_context.h"
#include "mars/boot/base_manager.h"
#include "mars/comm/comm_data.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"

namespace mars {
namespace app {

class AppManager : public BaseAppManager {
 public:
    explicit AppManager(mars::boot::BaseContext* context);
    ~AppManager() override;
    void Init() override;
    void UnInit() override;

 private:
    mars::boot::BaseContext* context_;

 public:
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
    void SetAppLogicNativeCallback(std::shared_ptr<AppLogicNativeCallback> _cb);
#endif

//TODO mars2 cpan
//#if !defined(ANDROID) || defined(CPP_CALL_BACK)
    void __GetProxyInfo(const std::string& _host, uint64_t _timetick) override;
//#endif

//TODO mars2 cpan
//#if TARGET_OS_IPHONE
    void __ClearProxyInfo() override;
//#endif

 private:
    Callback* sg_callback;
    mars::comm::ProxyInfo sg_proxyInfo;
    bool sg_gotProxy = false;
    mars::comm::Mutex sg_slproxymutex;
    mars::comm::Thread sg_slproxyThread;
    uint64_t sg_slporxytimetick = gettickcount();
    int sg_slproxycount = 0;
};

}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_MANAGER_H
