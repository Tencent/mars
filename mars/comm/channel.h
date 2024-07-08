//
// Created by gjt on 5/15/24.
//

#ifndef MARS_CHANNEL_H
#define MARS_CHANNEL_H

#include <cinttypes>
#include <condition_variable>
#include <list>
#include "boost/optional.hpp"

namespace mars {
namespace comm {
// T 必须有默认构造函数，如果没有，请使用std::shared_ptr
// todo 支持unique_ptr，考虑std::forward
template <typename T>
class Channel {
 public:
    explicit Channel(uint64_t capacity) {
        capacity_ = capacity;
    }
    void Send(T t) {
        std::lock_guard<std::mutex> lock(mtx_);
        list_.push_back(std::move(t));
        cv_.notify_one();
    }
    T Recv() {
        T t;
        RecvWithTimeoutMs(t, INT64_MAX);
        return t;
    }
    boost::optional<T> RecvWithTimeoutMs(uint64_t timeout_ms) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!list_.empty()) {
            T t = std::move(list_.front());
            list_.pop_front();
            return t;
        }
        auto timeout_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
        if (cv_.wait_until(lock, timeout_time) == std::cv_status::timeout) {
            return {};
        }
        T t = std::move(list_.front());
        list_.pop_front();
        return t;
    }

 private:
    std::list<T> list_;
    std::mutex mtx_;
    std::condition_variable cv_;
    uint64_t capacity_ = INT32_MAX;
};
}  // namespace comm
}  // namespace mars

#endif  // MARS_CHANNEL_H
