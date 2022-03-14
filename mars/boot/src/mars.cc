//
// Created by Cpan on 2022/3/10.
//
#include "mars/boot//mars.h"
#include "memory"

namespace mars {
namespace boot {

Mars::Mars() : _stn_logic_manager(new stn::StnLogicManager()) {}

void Mars::OnCreate(std::string _name) {
  //    appSession = CreateAppSession("default");

  _stn_logic_manager->OnCreate();
}
void Mars::OnInitConfigBeforeOnCreate(int _packer_encoder_version) {
  //    appSession.OnInitConfigBeforeOnCreate(_packer_encoder_version);
  //    _stn_logic_manager = new stn::StnLogicManager();
  _stn_logic_manager->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}
void Mars::OnForeground(bool _is_foreground) {}
uint64_t Mars::LastForeGroundChangeTime() {
  return last_foreground_change_time_;
}
bool Mars::IsActive() { return is_active_; }
void Mars::SwitchActiveStateForDebug(bool _active) {}
void Mars::OnNetworkChange() { _stn_logic_manager->OnNetworkChange(); }
void Mars::onNetworkDataChange(const char *_tag, int32_t _send, int32_t _recv) {
}
void Mars::OnAlarm(int64_t _id) {}
void Mars::OnDestroy() { _stn_logic_manager->OnDestroy(); }
void Mars::OnSignalCrash(int _sig) { _stn_logic_manager->OnSignalCrash(_sig); }
void Mars::OnExceptionCrash() { _stn_logic_manager->OnExceptionCrash(); }
void Mars::OnMemoryLow() {}

// stn::StnLogicManager CreateStnLogicManager(){
//  return stn::StnLogicManager();
//}

} // namespace boot
} // namespace mars
