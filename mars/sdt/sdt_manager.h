//
// Created by Changpeng Pan on 2022/7/22.
//

#ifndef MMNET_SDT_MANAGER_H
#define MMNET_SDT_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "mars/boot/base_manager.h"
#include "mars/boot/context.h"
#include "sdt.h"

namespace mars {
namespace sdt {

class SdtCore;

class SdtManager : public mars::boot::BaseManager {
 public:
    explicit SdtManager(mars::boot::Context* context);
    virtual ~SdtManager() override;
    std::string GetName() override;
    void Init();
    void UnInit();

 private:
    mars::boot::Context* context_;

 public:
    void OnCreate();
    void OnDestroy();

 public:
    void SetCallBack(Callback* const callback);
    void SetHttpNetcheckCGI(std::string cgi);
    void StartActiveCheck(CheckIPPorts& _longlink_check_item,
                          CheckIPPorts& _shortlink_check_item,
                          int _mode,
                          int _timeout);
    void CancelActiveCheck();
    void ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results);
//    extern void (*ReportNetCheckResult)(const std::vector<CheckResultProfile>& _check_results);
#ifdef NATIVE_CALLBACK
    void SetSdtNativeCallback(std::shared_ptr<SdtNativeCallback> _cb);
#endif

 private:
    Callback* callback_ = NULL;
    SdtCore* sdt_core_ = NULL;
};

}  // namespace sdt
}  // namespace mars

#endif  // MMNET_SDT_MANAGER_H
