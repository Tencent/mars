//
// Created by gjt on 5/16/24.
//

#include <thread>
#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "channel.h"
using namespace mars::comm;

int main() {
    const int capacity = 10;
    const int thread_num = 10;
    const int run_time = 10;
    const int sleep_time_ms = 5;
    const int small_time_interval_ms = 10;
    const int large_time_interval_ms = 1 * 1000;
    Channel<std::string> channel(capacity);
    for (uint32_t thread_i = 0; thread_i < capacity; ++thread_i) {
        std::thread([&channel, thread_i]() {
            for (int i = 0; i < run_time; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(random() % sleep_time_ms));
                channel.Send("thread:" + std::to_string(thread_i) + "\trun:" + std::to_string(i));
            }
        }).detach();
    }
    for (int recv_i = 0; recv_i < thread_num * run_time; ++recv_i) {
        std::string recv;
        bool succ = channel.RecvWithTimeoutMs(recv, small_time_interval_ms);
        EXPECT_TRUE(succ);
        std::cout << "succ:" << succ << "\trecv:" << recv << std::endl;
    }
    std::string recv;
    bool succ = channel.RecvWithTimeoutMs(recv, large_time_interval_ms);
    EXPECT_TRUE(!succ);
    std::cout << "one more recv:" << succ << std::endl;
}