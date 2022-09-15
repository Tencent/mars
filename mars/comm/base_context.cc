//
// Created by Changpeng Pan on 2022/9/15.
//
#include "base_context.h"
namespace mars {
namespace boot {

BaseContext* (*CreateContext)(const std::string& context_id) = [](const std::string& context_id) -> BaseContext* {
    return nullptr;
};
void (*DestroyContext)(BaseContext* context) = [](BaseContext* context) {
};

}  // namespace boot
}  // namespace mars