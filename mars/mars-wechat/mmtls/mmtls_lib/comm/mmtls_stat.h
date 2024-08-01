#pragma once
#include "mmtls_constants.h"
#include "mmtls_handshake_state.h"
namespace mmtls {
#ifdef DECLAR_INC_ELEMENT
#undef DECLAR_INC_ELEMENT
#endif
#define DECLAR_INC_ELEMENT(type, name) \
 private:                              \
    type name##_;                      \
                                       \
 public:                               \
    type name() const {                \
        return name##_;                \
    }                                  \
    void inc_##name() {                \
        name##_++;                     \
    }

#ifdef DECLAR_SET_ELEMENT
#undef DECLAR_SET_ELEMENT
#endif
#define DECLAR_SET_ELEMENT(type, name) \
 protected:                            \
    type name##_;                      \
                                       \
 public:                               \
    type name() const {                \
        return name##_;                \
    }                                  \
    void set_##name(type val) {        \
        name##_ = (val);               \
    }

#ifdef DECLAR_ADD_ELEMENT
#undef DECLAR_ADD_ELEMENT
#endif
#define DECLAR_ADD_ELEMENT(type, name) \
 protected:                            \
    type name##_;                      \
                                       \
 public:                               \
    type name() const {                \
        return name##_;                \
    }                                  \
    void add_##name(type val) {        \
        name##_ += (val);              \
    }

class Stat {
 public:
    Stat()
    : alert_type_(ALERT_NONE)
    , send_handshake_data_size_(0)
    , receive_handshake_data_size_(0)
    , send_handshake_data_cnt_(0)
    , receive_handshake_data_cnt_(0)
    , send_data_size_(0)
    , receive_data_size_(0)
    , send_data_cnt_(0)
    , receive_data_cnt_(0)
    , send_early_data_cnt_(0)
    , receive_early_data_cnt_(0)
    , send_early_data_size_(0)
    , receive_early_data_size_(0) {
    }

    virtual ~Stat() {
    }

    HandShakeMode mode() const {
        return state()->mode();
    }

    HandShakeStage stage() const {
        return state()->stage();
    }

    void set_alert(AlertType type) {
        alert_type_ = type;
    }
    AlertType alert() const {
        return alert_type_;
    }

    uint64 init_time() const {
        return state()->init_time();
    }

    uint64 active_time() const {
        return state()->active_time();
    }

    uint64 close_time() const {
        return state()->close_time();
    }

    void add_send_handshake_data_size(size_t size) {
        send_handshake_data_size_ += size;
    }
    void add_receive_handshake_data_size(size_t size) {
        receive_handshake_data_size_ += size;
    }
    void add_send_data_size(size_t size) {
        send_data_size_ += size;
    }
    void add_receive_data_size(size_t size) {
        receive_data_size_ += size;
    }
    void add_send_data_cnt(size_t count) {
        send_data_cnt_ += count;
    }
    void add_receive_data_cnt(size_t count) {
        receive_data_cnt_ += count;
    }
    void add_send_early_data_cnt(size_t count) {
        send_early_data_cnt_ += count;
    }
    void add_receive_early_data_cnt(size_t count) {
        receive_early_data_cnt_ += count;
    }
    void add_send_early_data_size(size_t size) {
        send_early_data_size_ += size;
    }
    void add_receive_early_data_size(size_t size) {
        receive_early_data_size_ += size;
    }
    void add_send_handshake_data_cnt(size_t count) {
        send_handshake_data_cnt_ += count;
    }
    void add_receive_handshake_data_cnt(size_t count) {
        receive_handshake_data_cnt_ += count;
    }

    size_t send_data_cnt() const {
        return send_data_cnt_;
    }
    size_t send_data_size() const {
        return send_data_size_;
    }
    size_t send_handshake_data_size() const {
        return send_handshake_data_size_;
    }
    size_t send_early_data_cnt() const {
        return send_early_data_cnt_;
    }
    size_t send_early_data_size() const {
        return send_early_data_size_;
    }
    size_t send_handshake_data_cnt() const {
        return send_handshake_data_cnt_;
    }

    size_t receive_data_size() const {
        return receive_data_size_;
    }
    size_t receive_data_cnt() const {
        return receive_data_cnt_;
    }
    size_t receive_handshake_data_size() const {
        return receive_handshake_data_size_;
    }
    size_t receive_early_data_cnt() const {
        return receive_early_data_cnt_;
    }
    size_t receive_early_data_size() const {
        return receive_early_data_size_;
    }
    size_t receive_handshake_data_cnt() const {
        return receive_handshake_data_cnt_;
    }

 protected:
    virtual const HandShakeState* state() const = 0;

 private:
    AlertType alert_type_;
    size_t send_handshake_data_size_;
    size_t receive_handshake_data_size_;
    size_t send_handshake_data_cnt_;
    size_t receive_handshake_data_cnt_;
    size_t send_data_size_;
    size_t receive_data_size_;
    size_t send_data_cnt_;
    size_t receive_data_cnt_;
    size_t send_early_data_cnt_;
    size_t receive_early_data_cnt_;
    size_t send_early_data_size_;
    size_t receive_early_data_size_;
};
}  // namespace mmtls
