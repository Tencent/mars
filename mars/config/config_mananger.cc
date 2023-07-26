////
//// Created by Changpeng Pan on 2023/7/25.
////
//
//#include "config_mananger.h"
//
//#include "mars/comm/xlogger/xlogger.h"
//
//using namespace mars::boot;
//
//namespace mars {
//namespace cfg {
//
//ConfigManager::ConfigManager(mars::boot::Context* context) : context_(context) {
//    xinfo_function(TSF "context:%_", context_->GetContextId());
//}
//
//ConfigManager::~ConfigManager() {
//    xinfo_function(TSF "");
//}
//
//std::string ConfigManager::GetName() {
//    return typeid(ConfigManager).name();
//}
//
//}  // namespace cfg
//}  // namespace mars