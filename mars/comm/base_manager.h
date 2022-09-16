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

    /** transition logic  */
 private:
    std::string context_id_;

 public:
    static BaseManager* CreateManager(const std::string& context_id);
    static void DestroyManager(BaseManager* manager);

 protected:
    static std::map<std::string, BaseManager*> s_app_manager_map_;
    static std::recursive_mutex s_mutex_;
    /** transition logic  */
};
}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_MANAGER_H
