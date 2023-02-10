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

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace app {

class AppManager : public BaseAppManager {
 public:
    explicit AppManager(BaseContext* context);
    ~AppManager() override;
    void Init() override;
    void UnInit() override;

 private:
    BaseContext* context_;

 public:
    void SetCallback(Callback* callback) override;
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host) override;
    std::string GetAppFilePath() override;
    AccountInfo GetAccountInfo() override;
    std::string GetUserName() override;
    std::string GetRecentUserName() override;
    unsigned int GetClientVersion() override;
    DeviceInfo GetDeviceInfo() override;
    //    double GetOsVersion() override;

 public:
    void GetProxyInfo(const std::string& _host, uint64_t _timetick) override;
//    #if TARGET_OS_IPHONE
    void ClearProxyInfo() override;
//    #endif

 private:
    std::shared_ptr<Callback> callback_;
    mars::comm::ProxyInfo proxy_info_;
    bool got_proxy_ = false;
    Mutex slproxymutex_;
    Thread slproxythread_;
    uint64_t slproxytimetick_ = gettickcount();
    int slproxycount_ = 0;
};

}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_MANAGER_H
