//
// Created by Changpeng Pan on 2022/7/22.
//

#include "sdt_manager.h"

#include "mars/comm/xlogger/xlogger.h"
#include "mars/log/appender.h"
#include "sdt/src/activecheck/httpchecker.h"
#include "sdt/src/sdt_core.h"

namespace mars {
namespace sdt {

SdtManager::SdtManager(Context* context) : context_(context), sdt_core_(new SdtCore(context)) {
    xdebug_function(TSF "contextId:%_", context_->GetContextId());
}

SdtManager::~SdtManager() {
}

std::string SdtManager::GetName() {
    return typeid(SdtManager).name();
}

void SdtManager::Init() {
}

void SdtManager::UnInit() {
}

void SdtManager::OnCreate() {
}

void SdtManager::OnDestroy() {
    delete sdt_core_;
    if (callback_) {
        delete callback_;
    }
}

void SdtManager::SetCallBack(Callback* const _callback) {
    xdebug_function();
    callback_ = _callback;
}

void SdtManager::SetHttpNetcheckCGI(std::string cgi) {
    xdebug_function(TSF "%_", cgi);
    sdt_core_->SetHttpNetcheckCGI(cgi);
}

void SdtManager::StartActiveCheck(CheckIPPorts& _longlink_check_item,
                                  CheckIPPorts& _shortlink_check_item,
                                  int _mode,
                                  int _timeout) {
    xdebug_function();
    sdt_core_->StartCheck(_longlink_check_item, _shortlink_check_item, _mode, _timeout);
}

void SdtManager::CancelActiveCheck() {
    xdebug_function();
    sdt_core_->CancelCheck();
}

void SdtManager::ReportNetCheckResult(const std::vector<CheckResultProfile>& _check_results) {
    xdebug_function();
    // TODO mars2 need call mmsdt_manager->MMReportNetCheckResult
}

#ifdef NATIVE_CALLBACK
void SdtManager::SetSdtNativeCallback(std::shared_ptr<SdtNativeCallback> _cb) {
    xdebug_function();
}
#endif

}  // namespace sdt
}  // namespace mars
