//
// Created by Cpan on 2022/9/7.
//

#ifndef MMNET_APP_MANAGER_CALLBACK_WRAPPER_H
#define MMNET_APP_MANAGER_CALLBACK_WRAPPER_H

#include <jni.h>

#include "mars/app/app.h"

namespace mars {
namespace app {

class AppManagerJniCallback : public Callback {
 public:
    AppManagerJniCallback(JNIEnv* env, jobject callback);
    virtual ~AppManagerJniCallback();

 private:
    jobject callback_inst_;
    jclass callbacks_class_;

 public:
    virtual bool GetProxyInfo(const std::string& _host, mars::comm::ProxyInfo& _proxy_info) override;

    virtual std::string GetAppFilePath() override;

    virtual AccountInfo GetAccountInfo() override;

    virtual unsigned int GetClientVersion() override;

    virtual DeviceInfo GetDeviceInfo() override;
};

}  // namespace app
}  // namespace mars

#endif  // MMNET_APP_MANAGER_CALLBACK_WRAPPER_H
