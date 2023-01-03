//
// Created by Cpan on 2022/9/16.
//

#ifndef MMNET_BASE_MANAGER_H
#define MMNET_BASE_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace mars {
namespace boot {

class BaseManager {
 public:
    virtual ~BaseManager() {
    }
    virtual void Init() = 0;
    virtual void UnInit() = 0;
};
}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_MANAGER_H
