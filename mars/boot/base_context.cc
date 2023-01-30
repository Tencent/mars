//
// Created by Cpan on 2022/10/19.
//

#include "base_context.h"

#include "mars/comm/xlogger/xlogger.h"

namespace mars {
namespace boot {

BaseContext* (*CreateContext)(const std::string& context_id) = nullptr;

void (*DestroyContext)(BaseContext* context) = nullptr;

// template<typename T>
// void BaseContext::AddManagerWrapper(const std::string _class_name, ManagerWrapper<T>* manager){
//     manager_map_[_class_name] = manager;
// }

// template<typename T>
// ManagerWrapper<T>* BaseContext::GetManagerWrapper(const std::string _class_name){
//     return manager_map_[_class_name];
// }

}  // namespace boot
}  // namespace mars
