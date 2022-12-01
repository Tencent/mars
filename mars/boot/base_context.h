//
// Created by Changpeng Pan on 2022/9/13.
//

#ifndef MMNET_BASE_CONTEXT_H
#define MMNET_BASE_CONTEXT_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "mars/boot/base_manager.h"
#include "mars/boot/base_app_manager.h"
#include "mars/boot/manager_wrapper.h"

namespace mars {
namespace boot {

class BaseContext {
 public:
    virtual ~BaseContext() {
    }

 public:
    virtual int Init() = 0;
    virtual int UnInit() = 0;
    virtual void SetContextId(const std::string& context_id) = 0;
    virtual const std::string& GetContextId() = 0;

    virtual BaseManager* GetManager(const std::string _class_name) = 0;
    virtual void AddManager(const std::string _class_name, BaseManager* _manager) = 0;

    virtual app::BaseAppManager* GetAppManager() = 0;

//    template <class T>
//    void AddManagerWrapper(const std::string _class_name, ManagerWrapper<T>* manager);

//    template <class T>
//    ManagerWrapper<T>* GetManagerWrapper(const std::string _class_name);

 private:
    std::map<std::string, ManagerWrapper<BaseManager>*> manager_map_;

};

extern BaseContext* (*CreateContext)(const std::string& context_id);
extern void (*DestroyContext)(BaseContext* context);

}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_CONTEXT_H
