//
// Created by Cpan on 2022/3/10.
//
#include "mars/boot/mars.h"
#include "memory"

using namespace mars::stn;

namespace mars {
namespace boot {

Mars::Mars() : stn_manager_(new StnManager()) {}

void Mars::OnInitConfigBeforeOnCreate(int _packer_encoder_version) {
    stn_manager_ = new stn::StnManager();
    //stn_manager_->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

uint64_t Mars::LastForeGroundChangeTime() {
    return last_foreground_change_time_;
}
bool Mars::IsActive() { return is_active_; }

void Mars::SwitchActiveStateForDebug(bool _active) {
    is_active_ = _active;
}

StnManager* Mars::GetStnManager() {
    return stn_manager_;
}

SdtManager* Mars::GetSdtManager() {
    return sdt_manager;
}

void Mars::OnCreate(const std::string& _name) {

}

void Mars::OnDestroy() {

}

void Mars::OnSignalCrash(int _sig) {

}

void Mars::OnExceptionCrash() {

}

void Mars::OnNetworkChange() {

}

void Mars::OnNetworkDataChange(const char *_tag, int32_t _send, int32_t _recv) {

}

} // namespace boot
} // namespace mars
