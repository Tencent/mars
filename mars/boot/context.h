//
// Created by Changpeng Pan on 2022/9/9.
//

#ifndef MMNET_CONTEXT_H
#define MMNET_CONTEXT_H

#include <string>
#include <map>
#include <vector>
#include <mutex>

#include "mars/comm/base_context.h"


namespace mars {

namespace stn {
class StnManager;
}

namespace app {
class AppManager;
}

namespace boot {
class Context : public BaseContext{
 public:
    explicit Context();
    ~Context() override;
 public:
    static Context* CreateContext(const std::string &context_id);
    static void DeleteContext(BaseContext* context);
public:
    AppManager* appManager() {return app_manager_;}
    StnManager* stnManager() {return stn_manager_;}
 public:
    int Init() override;
    int UnInit() override;
    
    BaseAppManager* GetAppManager() override;
    BaseStnManager* GetStnManager() override;

public:
    void SetContextId(const std::string &context_id);
    const std::string &GetContextId();
    
 private:
    static std::map<std::string, Context*> s_context_map_;
    static std::recursive_mutex s_mutex_;

 private:
    bool is_init_ = false;
    std::string context_id_;
    std::once_flag set_context_id_flag_;
    std::recursive_mutex mutex_;
    AppManager* app_manager_ = nullptr;
    StnManager* stn_manager_ = nullptr;

};

}  // namespace app
}  // namespace mars

#endif  // MMNET_CONTEXT_H
