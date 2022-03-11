//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_MARS_APP_H
#define MMNET_MARS_APP_H
#include "stn/stn_logic_manager.h"
#include <string>

namespace mars {
namespace baseevent {


class AppSession {

public:
  AppSession(std::string name);

  void OnCreate();
  void OnInitConfigBeforeOnCreate(int _packer_encoder_version);
  void OnDestroy();

  mars::stn::StnLogicManager *stn_logic_manager;

};

} // namespace boot
} // namespace mars
#endif // MMNET_MARS_APP_H
