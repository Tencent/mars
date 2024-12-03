//
// Created by Changpeng Pan on 2022/9/13.
//

#ifndef MMNET_BASE_CONTEXT_H
#define MMNET_BASE_CONTEXT_H

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <memory>

#include "base_manager.h"

#define S_SCOPED_LOCK() std::unique_lock<std::recursive_mutex> s_lock(s_mutex_)

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
        S_SCOPED_LOCK();
        std::string clazz_name = ((BaseManager*)mgr)->GetName();
        manager_map_[clazz_name] = mgr;
    }

    template <typename T, typename std::enable_if<std::is_base_of<BaseManager, T>::value>::type* = nullptr>
    T* GetManager() {
        S_SCOPED_LOCK();
        std::string clazz_name = typeid(T).name();
        // when context delete, then manager map will be null.
        if (!manager_map_.empty() && manager_map_.find(clazz_name) != manager_map_.end()) {
            return (T*)manager_map_[clazz_name];
        }
        return NULL;
    }

    template <typename T, typename std::enable_if<std::is_base_of<BaseManager, T>::value>::type* = nullptr>
    void RemoveManager(T* mgr) {
        S_SCOPED_LOCK();
        std::string clazz_name = ((BaseManager*)mgr)->GetName();
        if (manager_map_.find(clazz_name) != manager_map_.end()) {
            manager_map_.erase(clazz_name);
        }
    }

    void AddManagerWithName(std::string name, BaseManager* mgr) {
        std::string clazz_name = mgr->GetName();
        java_clazz_name_c_clazz_name_map_[name] = clazz_name;
        AddManager(mgr);
    }

    void* GetManagerByName(std::string name) {
        if (java_clazz_name_c_clazz_name_map_.find(name) != java_clazz_name_c_clazz_name_map_.end()) {
            std::string clazz_name = java_clazz_name_c_clazz_name_map_[name];
            if (manager_map_.find(clazz_name) != manager_map_.end()) {
                return manager_map_[clazz_name];
            }
        }
        return NULL;
    }

    void RemoveManagerWithName(std::string name, BaseManager* mgr) {
        if (java_clazz_name_c_clazz_name_map_.find(name) != java_clazz_name_c_clazz_name_map_.end()) {
            java_clazz_name_c_clazz_name_map_.erase(name);
        }
        RemoveManager(mgr);
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
    static std::atomic<int> s_context_index;
};

template <class... Args>
std::shared_ptr<Context> make_context_ptr(Args&&... args) {
    return std::shared_ptr<Context>(Context::CreateContext(std::forward<Args>(args)...), Context::DestroyContext);
}

}  // namespace boot
}  // namespace mars

#endif  // MMNET_BASE_CONTEXT_H
