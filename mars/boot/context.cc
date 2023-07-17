//
// Created by Cpan on 2022/10/19.
//

#include "context.h"

#include <sstream>

namespace mars {
namespace boot {

std::map<std::string, Context*> Context::s_context_map_;
std::recursive_mutex Context::s_mutex_;
std::atomic<int> Context::s_context_index;

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
        s_context_index++;
        auto context = new Context();
        std::stringstream ss;
        ss << std::to_string(s_context_index) << ":" << std::to_string(reinterpret_cast<uint64_t>(context));
        auto tmp_context_id = ss.str();
        context->SetContextId(tmp_context_id);
        s_context_map_[tmp_context_id] = context;
        return context;
    }
}

void Context::DestroyContext(Context* context) {
    S_SCOPED_LOCK();
    if (context != nullptr) {
        auto context_id = context->GetContextId();
        s_context_map_.erase(context_id);
        delete context;
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
        is_init_ = true;
    }
    return 0;
}

int Context::UnInit() {
    S_SCOPED_LOCK();
    if (!is_init_) {
        return -1;
    }
    is_init_ = false;
    return 0;
}

void Context::SetContextId(const std::string& context_id) {
    context_id_ = context_id;
}

const std::string& Context::GetContextId() {
    return context_id_;
}

}  // namespace boot
}  // namespace mars

void ExportContext() {
}
