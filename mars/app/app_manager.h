//
// Created by Changpeng Pan on 2022/8/29.
//

#ifndef MMNET_APP_MANAGER_H
#define MMNET_APP_MANAGER_H

#include <memory>

#include "mars/app/app.h"
#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "mars/comm/comm_data.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/thread.h"
#include "mars/comm/time_utils.h"

using namespace mars::comm;
using namespace mars::boot;

namespace mars {
namespace app {

class AppManager : public mars::boot::BaseManager {
 public:
    explicit AppManager(Context* context);
    ~AppManager() override;
    std::string GetName() override;

 private:
    Context* context_;

 public:
    void SetCallback(Callback* callback);
    mars::comm::ProxyInfo GetProxyInfo(const std::string& _host);
    std::string GetAppFilePath();
    AccountInfo GetAccountInfo();
    std::string GetUserName();
    std::string GetRecentUserName();
    unsigned int GetClientVersion();
    DeviceInfo GetDeviceInfo();

 public:
    void GetProxyInfo(const std::string& _host, uint64_t _timetick);
    //    #if TARGET_OS_IPHONE
    void ClearProxyInfo();
    //    #endif

 private:
    Callback* callback_;
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
