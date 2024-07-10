//
// Created by gjt on 5/15/24.
//

#ifndef MARS_CACHE_WITH_TIMEOUT_H
#define MARS_CACHE_WITH_TIMEOUT_H

#include <cinttypes>
#include <mutex>

#include "mars/comm/xlogger/xlogger.h"
#include "stlutil.h"
#include "time_utils.h"

template <typename T>
class CacheWithTimeout {
 public:
    explicit CacheWithTimeout(uint64_t timeout_ms) {
        xinfo2(TSF "timeout_ms:%_", timeout_ms);
        timeout_ms_ = timeout_ms;
    }
    void DeleteTimeOut() {
        uint64_t now_time = gettickcount();
        std::lock_guard<std::mutex> lock(mtx_);
        stlutil::RemoveIfAndErase(cache_, [&](const TWithTime& f) {
            bool timeout = now_time - f.second >= timeout_ms_;
            if (timeout) {
                xinfo2(TSF "cache time:%_", f.second);
            }
            return timeout;
        });
    }
    void Update(const T& fragment) {
        std::lock_guard<std::mutex> lock(mtx_);
        stlutil::RemoveIfAndErase(cache_, [&](const TWithTime& it) {
            return it.first == fragment;
        });
        cache_.emplace_back(fragment, gettickcount());
    }
    bool Hit(const T& fragment) {
        std::lock_guard<std::mutex> lock(mtx_);
        return stlutil::AnyOf(cache_, [&](const TWithTime& f) {
            return f.first == fragment;
        });
    }
    using GetIfFunc = std::function<bool(const T&)>;
    bool GetIf(const GetIfFunc& f, T& t) {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = std::find_if(cache_.begin(), cache_.end(), [&](const TWithTime& t) {
            return f(t.first);
        });
        if (it != cache_.end()) {
            t = it->first;
            return true;
        }
        return false;
    }

 private:
    uint64_t timeout_ms_ = 0;
    using TWithTime = std::pair<T, uint64_t>;
    std::vector<TWithTime> cache_;
    std::mutex mtx_;
};

#endif  // MARS_CACHE_WITH_TIMEOUT_H
