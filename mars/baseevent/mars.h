//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_MARS_H
#define MMNET_MARS_H

//#include "app_session.h"
#include <string.h>
#include "stn/stn_logic_manager.h"

namespace mars {
namespace baseevent {

class Mars {
public:
  Mars();

  void OnCreate(std::string _name);
  void OnInitConfigBeforeOnCreate(int _packer_encoder_version);
  void OnForeground(bool _is_foreground);
  uint64_t LastForeGroundChangeTime();
  bool IsActive();
  void SwitchActiveStateForDebug(bool _active);
  void OnNetworkChange();
  void onNetworkDataChange(const char *_tag, int32_t _send, int32_t _recv);
  void OnAlarm(int64_t _id);
  void OnDestroy();
  void OnSignalCrash(int _sig);
  void OnExceptionCrash();
  void OnMemoryLow();

//  stn::StnLogicManager CreateStnLogicManager();


private:
  bool is_foreground_;
  bool is_active_;
  uint64_t last_foreground_change_time_;
//  AppSession appSession;
  mars::stn::StnLogicManager *stn_logic_manager;
};

} // namespace boot
} // namespace mars

#endif // MMNET_MARS_H
