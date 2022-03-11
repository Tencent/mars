//
// Created by Cpan on 2022/3/10.
//

#ifndef MMNET_STN_LOGIC_MANAGER_H
#define MMNET_STN_LOGIC_MANAGER_H

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "mars/comm/autobuffer.h"
#include "mars/stn/stn.h"

namespace mars {

namespace comm {
class ProxyInfo;
}

namespace stn {

class StnLogicManager {
public:
  void OnCreate();
  void OnInitConfigBeforeOnCreate(int _packer_encoder_version);
  void OnDestroy();
  void OnSignalCrash(int _sig);
  void OnExceptionCrash();
  void OnNetworkChange();
};
} // namespace stn
} // namespace mars

#endif // MMNET_STN_LOGIC_MANAGER_H
