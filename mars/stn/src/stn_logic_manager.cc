//
// Created by Cpan on 2022/3/10.
//

#include "stn_logic_manager.h"
#include "stn_logic_signal.h"

namespace mars {
namespace stn {

void StnLogicManager::OnCreate() {

  GetStnLogicSignalOnCreate()();

}

void StnLogicManager::OnInitConfigBeforeOnCreate(int _packer_encoder_version){
    GetStnLogicSignalOnInitBeforeOnCreate()(_packer_encoder_version);
}

void StnLogicManager::OnDestroy() {
  GetStnLogicSignalOnDestroy()();
}

void StnLogicManager::OnSignalCrash(int _sig) {
  GetStnLogicSignalOnSingalCrash()(_sig);
}

void StnLogicManager::OnExceptionCrash() {
  GetStnLogicSignalOnExceptionCrash()();
}

void StnLogicManager::OnNetworkChange() {
  GetStnLogicSignalOnNetworkChange()();
}

} // namespace stn
} // namespace mars
