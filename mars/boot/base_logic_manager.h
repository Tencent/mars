//
// Created by Cpan on 2022/3/14.
//

#ifndef MMNET_BASE_LOGIC_MANAGER_H
#define MMNET_BASE_LOGIC_MANAGER_H

#include <string.h>

namespace mars{
namespace boot{

class BaseLogicManager{
public:
  
  virtual void OnCreate() = 0;
  virtual void OnInitConfigBeforeOnCreate(int _packer_encoder_version) = 0 ;
  virtual void OnForeground(bool _is_forground) = 0 ;
  virtual uint64_t LastForeGroundChangeTime() = 0 ;
  virtual bool IsActive() = 0 ;
  virtual void SwitchActiveStateForDebug(bool _active) = 0 ;
  virtual void OnNetworkChange() = 0 ;
  virtual void onNetworkDataChange(const char *_tag, int32_t _send, int32_t _recv) = 0 ;
  virtual void OnAlarm(int64_t _id) = 0 ;
  virtual void OnDestroy() = 0 ;
  virtual void OnSignalCrash(int _sig) = 0 ;
  virtual void OnExceptionCrash() = 0 ;
  virtual void OnMemoryLow() = 0 ;
  
};
}
}

#endif // MMNET_BASE_LOGIC_MANAGER_H
