//
// Created by Cpan on 2022/4/6.
//

#include "sdt_manager.h"

namespace mars {
namespace sdt {

SdtManager::SdtManager() {
   sdt_core_ = new SdtCore();
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