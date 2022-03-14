//
// Created by Cpan on 2022/3/10.
//

#include "stn_logic_manager.h"
#include "stn_logic_signal.h"


using namespace mars::boot;

namespace mars {
namespace stn {

StnLogicManager::StnLogicManager(){
    
}

void StnLogicManager::OnCreate() {
  GetStnLogicSignalOnCreate()();
}

void StnLogicManager::OnInitConfigBeforeOnCreate(int _packer_encoder_version){
    GetStnLogicSignalOnInitBeforeOnCreate()(_packer_encoder_version);
}



void StnLogicManager::OnForeground(bool _is_forground) {
    GetStnLogicSignalOnForeground()(_is_forground);
}

uint64_t StnLogicManager::LastForeGroundChangeTime() {
    return 0;
}

bool StnLogicManager::IsActive() {
    return true;
}

void StnLogicManager::SwitchActiveStateForDebug(bool _active) {
    
}

void StnLogicManager::OnNetworkChange() {
    GetStnLogicSignalOnNetworkChange()();
}

void StnLogicManager::onNetworkDataChange(const char *_tag, int32_t _send,
                                          int32_t _recv) {
    
}
void StnLogicManager::OnAlarm(int64_t _id){
    
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

void StnLogicManager::OnMemoryLow(){
    
}




} // namespace stn
} // namespace mars
