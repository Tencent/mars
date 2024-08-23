//
// Created by Cpan on 2022/9/16.
//

#ifndef MMNET_BASE_MANAGER_H
#define MMNET_BASE_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "base_event.h"

namespace mars {
namespace boot {

class BaseManager : public BaseEvent {
 public:
    virtual ~BaseManager() {
    }
    virtual std::string GetName() = 0;
};
}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_MANAGER_H
