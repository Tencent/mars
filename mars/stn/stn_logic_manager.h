//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_STN_LOGIC_MANAGER_H
#define MMNET_STN_LOGIC_MANAGER_H

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "mars/boot/base_logic_manager.h"
#include "mars/stn/stn.h"

using namespace mars::boot;

namespace mars {

namespace stn {

class StnLogicManager : public BaseLogicManager {
public:
  StnLogicManager();
  void OnCreate() override;
  void OnInitConfigBeforeOnCreate(int _packer_encoder_version) override;
  void OnForeground(bool _is_forground) ;
  uint64_t LastForeGroundChangeTime() ;
  bool IsActive() ;
  void SwitchActiveStateForDebug(bool _active) ;
  void OnNetworkChange() ;
  void onNetworkDataChange(const char *_tag, int32_t _send,
                           int32_t _recv) ;
  void OnAlarm(int64_t _id) ;
  void OnDestroy() ;
  void OnSignalCrash(int _sig) ;
  void OnExceptionCrash() ;
  void OnMemoryLow() ;
};

} // namespace stn
} // namespace mars

#endif // MMNET_STN_LOGIC_MANAGER_H
