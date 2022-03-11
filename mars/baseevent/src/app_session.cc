//
// Created by Cpan on 2022/3/10.
//

#include "app_session.h"
#include <string>

namespace mars {
namespace baseevent {

AppSession::AppSession(std::string _name) {}

void AppSession::OnCreate() {
  stn_logic_manager = new mars::stn::StnLogicManager();
  stn_logic_manager->OnCreate();
}

void AppSession::OnInitConfigBeforeOnCreate(int _packer_encoder_version){
    stn_logic_manager->OnInitConfigBeforeOnCreate(_packer_encoder_version);
}

void AppSession::OnDestroy() { stn_logic_manager->OnDestroy(); }

} // namespace boot
} // namespace mars
