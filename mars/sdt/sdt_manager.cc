//
// Created by Cpan on 2022/4/6.
//

#include "sdt_manager.h"
#include "sdt/src/sdt_core.h"

namespace mars {
namespace sdt {

SdtManager::SdtManager() {
   sdt_core_ = new mars::sdt::SdtCore();
}

void SdtManager::OnCreate() {
    
}

void SdtManager::OnDestroy() {
    delete sdt_core_;
}

void SdtManager::SetCallBack(StnManagerCallback *const _callback) {
    callback_ = _callback;
}

void SdtManager::SetHttpNetCheckCGI(std::string cgi) {
}

void SdtManager::StartActiveCheck(CheckIPPorts &_longlink_check_item, CheckIPPorts &_shortlink_check_item, int _mode, int _timeout) {
    sdt_core_->StartCheck(_longlink_check_item, _shortlink_check_item, _mode, _timeout);
}

void SdtManager::CancelActiveCheck() {
    sdt_core_->CancelCheck();
}

}
}
