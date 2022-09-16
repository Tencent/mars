//
// Created by Changpeng Pan on 2022/7/22.
//

#include "sdt_manager.h"
#include "mars/log/appender.h"
#include "mars/comm/xlogger/xlogger.h"
#include "sdt/src/sdt_core.h"

namespace mars {
namespace sdt {

#define SDT_WEAK_CALL(func) \
    if (!sdt_core_) {\
        xwarn2(TSF"sdt uncreate");\
        return;\
    }\
	sdt_core_->func

SdtManager::SdtManager(const std::string& context_id) {

}

SdtManager::~SdtManager() {
}


void SdtManager::Init() {

}
void SdtManager::UnInit() {

}

/** transition logic  */
std::map<std::string, SdtManager *> SdtManager::s_sdt_manager_map_;

std::recursive_mutex SdtManager::s_mutex_;

SdtManager* SdtManager::CreateSdtManager(const std::string& context_id) {
    if (!context_id.empty()) {
        if (s_sdt_manager_map_.find(context_id) != s_sdt_manager_map_.end()) {
            return s_sdt_manager_map_[context_id];
        } else {
            auto app_manager = new SdtManager(context_id);
            s_sdt_manager_map_[context_id] = app_manager;
            return app_manager;
        }
    }
    return nullptr;
}

void SdtManager::DestroySdtManager(SdtManager* manager) {
    if (manager != nullptr) {
        auto* temp = dynamic_cast<SdtManager*>(manager);
        auto context_id = temp->context_id_;
        if (s_sdt_manager_map_.find(context_id) != s_sdt_manager_map_.end()) {
            s_sdt_manager_map_.erase(context_id);
        }
        delete temp;
        manager = nullptr;
    }
}
/** transition logic  */

void SdtManager::OnCreate() {
  sdt_core_ = new SdtCore();
}

void SdtManager::OnDestroy() {
  delete sdt_core_;
}

void SdtManager::SetCallBack(Callback* const callback) {
  callback_ = callback;
}

void SdtManager::SetHttpNetcheckCGI(std::string cgi) {

}

void SdtManager::StartActiveCheck(CheckIPPorts& _longlink_check_item, CheckIPPorts& _shortlink_check_item, int _mode, int _timeout) {
    SDT_WEAK_CALL(StartCheck(_longlink_check_item, _shortlink_check_item, _mode, _timeout));
}

void SdtManager::CancelActiveCheck() {
  SDT_WEAK_CALL(CancelCheck());
}

void SdtManager::ReportNetCheckResult(const std::vector<CheckResultProfile> &_check_results) {
    //SDT_WEAK_CALL(R)
}

}
}
