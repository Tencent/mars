#pragma once
#include "mmtls_types.h"
#include "mmtls_utils.h"

namespace mmtls {
class ConnectionSeqNumbers {
 public:
    ConnectionSeqNumbers() : read_seq_no_(0), write_seq_no_(0), is_heartbeat_(false), heartbeat_start_time_(0) {
    }
    ~ConnectionSeqNumbers(){};

    uint64 NextWriteSeq() {
        if (is_heartbeat_) {
            return write_seq_no_;
        } else {
            return write_seq_no_++;
        }
    }

    uint64 NextReadSeq() {
        if (is_heartbeat_) {
            return read_seq_no_;
        } else {
            return read_seq_no_++;
        }
    }

    void EnableHeartbeat() {
        if (is_heartbeat_ == false) {
            heartbeat_start_time_ = NowTime();
        }

        is_heartbeat_ = true;
    }

    void DisableHeartbeat() {
        if (is_heartbeat_ == true) {
            heartbeat_start_time_ = 0;
        }

        is_heartbeat_ = false;
    }

    const bool is_heartbeat() const {
        return is_heartbeat_;
    }

    const uint32 heartbeat_start_time() const {
        return heartbeat_start_time_;
    }

 private:
    uint64 read_seq_no_;
    uint64 write_seq_no_;

    bool is_heartbeat_;
    uint32 heartbeat_start_time_;
};
};  // namespace mmtls
