//
// Created by Cpan on 2022/10/19.
//

#include "context.h"

#include "mars/comm/bootrun.h"

#define S_SCOPED_LOCK() std::unique_lock<std::recursive_mutex> s_lock(s_mutex_)
#define S_SCOPED_UNLOCK() s_lock.unlock()
static int context_instance_counter_ = 0;

namespace mars {

namespace boot {

std::map<std::string, Context*> Context::s_context_map_;
std::recursive_mutex Context::s_mutex_;

Context* Context::CreateContext(const std::string& context_id) {
    S_SCOPED_LOCK();
    if (!context_id.empty()) {
        auto iter = s_context_map_.find(context_id);
        if (iter != s_context_map_.end()) {
            return iter->second;
        } else {
            auto context = new Context();
            context->SetContextId(context_id);
            s_context_map_[context_id] = context;
            return context;
        }
    } else {
        auto context = new Context();
        auto tmp_context_id = std::to_string(reinterpret_cast<uint64_t>(context));
        context->SetContextId(tmp_context_id);
        s_context_map_[tmp_context_id] = context;
        return context;
    }
}

void Context::DestroyContext(Context* context) {
    S_SCOPED_LOCK();
    if (context != nullptr) {
        auto* temp_context = dynamic_cast<Context*>(context);
        auto context_id = temp_context->GetContextId();

        auto iter = s_context_map_.find(context_id);
        if (iter != s_context_map_.end()) {
            s_context_map_.erase(context_id);
        }
        delete temp_context;
    }
}

Context::Context() {
    Init();
}

Context::~Context() {
    UnInit();
}

int Context::Init() {
    S_SCOPED_LOCK();
    if (!is_init_) {
        context_instance_counter_++;
        is_init_ = true;
    }
    return 0;
}

int Context::UnInit() {
    S_SCOPED_LOCK();
    if (is_init_) {
        // do somethings
        is_init_ = false;
    } else {
        return -1;
    }
    return 0;
}

void Context::SetContextId(const std::string& context_id) {
//    std::call_once(set_context_id_flag_, [this, context_id] {
        context_id_ = context_id;
//    });
}

const std::string& Context::GetContextId() {
    return context_id_;
}

}  // namespace boot
}  // namespace mars

void ExportContext() {
}
