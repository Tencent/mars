//
// Created by Changpeng Pan on 2022/9/13.
//

#ifndef MMNET_BASE_CONTEXT_H
#define MMNET_BASE_CONTEXT_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "base_manager.h"

namespace mars {
namespace boot {

class Context {
 public:
    Context();
    ~Context();

 public:
    int Init();
    int UnInit();
    void SetContextId(const std::string& context_id);
    const std::string& GetContextId();

 public:
    static Context* CreateContext(const std::string& context_id);
    static void DestroyContext(Context* context);

 public:
    template <typename T, typename std::enable_if<std::is_base_of<BaseManager, T>::value>::type* = nullptr>
    void AddManager(T* mgr) {
        std::string clazz_name = ((BaseManager*)mgr)->GetName();
        manager_map_[clazz_name] = mgr;
    }

    template <typename T, typename std::enable_if<std::is_base_of<BaseManager, T>::value>::type* = nullptr>
    T* GetManager() {
        std::string clazz_name = typeid(T).name();
        //when context delete, then manager map will be null.
        if (!manager_map_.empty()) {
            return (T*)manager_map_[clazz_name];
        }
        return NULL;
    }

    template <typename T, typename std::enable_if<std::is_base_of<BaseManager, T>::value>::type* = nullptr>
    void RemoveManager(T* mgr) {
        std::string clazz_name = ((BaseManager*)mgr)->GetName();
        manager_map_.erase(clazz_name);
    }

    void AddManagerWithName(std::string name, BaseManager* mgr) {
        std::string clazz_name = mgr->GetName();
        java_clazz_name_c_clazz_name_map_[name] = clazz_name;
        AddManager(mgr);
    }

    void* GetManagerByName(std::string name) {
        std::string clazz_name = java_clazz_name_c_clazz_name_map_[name];
        return manager_map_[clazz_name];
    }

 private:
    bool is_init_ = false;
    std::string context_id_;
    std::recursive_mutex mutex_;
    std::map<std::string, void*> manager_map_;
    std::map<std::string, std::string> java_clazz_name_c_clazz_name_map_;

 private:
    static std::map<std::string, Context*> s_context_map_;
    static std::recursive_mutex s_mutex_;
};

}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_CONTEXT_H
