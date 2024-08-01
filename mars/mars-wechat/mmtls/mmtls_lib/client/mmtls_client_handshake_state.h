#pragma once

#include <vector>

#include "mmtls_client_psk.h"
#include "mmtls_handshake_state.h"
#include "mmtls_key_pair.h"

namespace mmtls {

class ClientPsk;

// Stores handshake releated data in client.
class ClientHandShakeState : public HandShakeState {
 public:
    ClientHandShakeState()
    : HandShakeState(CONN_SIDE_CLIENT)
    , access_psk_(NULL)
    , refresh_psk_(NULL)
    , new_access_psk_(NULL)
    , new_refresh_psk_(NULL)
    , cert_region_(0) {
    }

    virtual ~ClientHandShakeState();

    virtual void ActiveSession();

    // Field setter/getter

    ClientPsk* access_psk() const {
        return access_psk_;
    }
    ClientPsk* refresh_psk() const {
        return refresh_psk_;
    }
    ClientPsk* new_access_psk() const {
        return new_access_psk_;
    }
    ClientPsk* new_refresh_psk() const {
        return new_refresh_psk_;
    }

    const std::vector<KeyPair>& static_server_ecdh_keys() const {
        return static_server_ecdh_keys_;
    }
    const std::vector<KeyPair>& ephemeral_client_ecdh_keys() const {
        return ephemeral_client_ecdh_keys_;
    }
    const KeyPair& ecdsa_key() const {
        return ecdsa_key_;
    }
    const std::vector<KeyPair>& ecdsa_keys() const {
        return ecdsa_keys_;
    }
    const uint32 cert_region() const {
        return cert_region_;
    }
    const bool has_cert_region() const {
        return cert_region_ > 0;
    }
    const std::vector<std::string>& fallback_urls_vec() const {
        return fallback_urls_vec_;
    }

    void set_access_psk(ClientPsk* psk) {
        if (access_psk_ != NULL) {
            delete access_psk_;
            access_psk_ = NULL;
        }
        access_psk_ = psk;
    }
    void set_refresh_psk(ClientPsk* psk) {
        if (refresh_psk_ != NULL) {
            delete refresh_psk_;
            refresh_psk_ = NULL;
        }
        refresh_psk_ = psk;
    }
    void set_new_access_psk(ClientPsk* psk) {
        if (new_access_psk_ != NULL) {
            delete new_access_psk_;
            new_access_psk_ = NULL;
        }
        new_access_psk_ = psk;
    }
    void set_new_refresh_psk(ClientPsk* psk) {
        if (new_refresh_psk_ != NULL) {
            delete new_refresh_psk_;
            new_refresh_psk_ = NULL;
        }
        new_refresh_psk_ = psk;
    }
    void set_static_server_ecdh_keys(const std::vector<KeyPair>& static_server_ecdh_keys) {
        static_server_ecdh_keys_ = static_server_ecdh_keys;
    }
    void set_ephemeral_client_ecdh_keys(const std::vector<KeyPair>& ephemeral_client_ecdh_keys) {
        ephemeral_client_ecdh_keys_ = ephemeral_client_ecdh_keys;
    }
    int set_ecdsa_key(uint32 version) {
        for (size_t i = 0; i < ecdsa_keys_.size(); ++i) {
            if (version == ecdsa_keys_[i].version()) {
                set_ecdsa_key(ecdsa_keys_[i]);
                MMTLSLOG_DEBUG("found certificate version %u", version);
                break;
            }

            MMTLSLOG_DEBUG("not found certificate version %u", version);
        }

        MMTLS_CHECK_GT(ecdsa_key().version(), (uint32)0, ERR_UNEXPECT_CHECK_FAIL, "not found ecdsa key");
        return OK;
    }
    void set_ecdsa_key(const KeyPair& ecdsa_key) {
        ecdsa_key_ = ecdsa_key;
    }
    void set_ecdsa_keys(const std::vector<KeyPair>& ecdsa_keys) {
        ecdsa_keys_ = ecdsa_keys;
    }
    void set_cert_region(uint32 cert_region) {
        cert_region_ = cert_region;
    }
    void set_fallback_urls_vec(const std::vector<std::string>& fallback_urls_vec) {
        fallback_urls_vec_ = fallback_urls_vec;
    }

 protected:
    void ClearAll();

 private:
    std::vector<KeyPair> static_server_ecdh_keys_;
    std::vector<KeyPair> ephemeral_client_ecdh_keys_;
    KeyPair ecdsa_key_;
    std::vector<KeyPair> ecdsa_keys_;

    ClientPsk* access_psk_;
    ClientPsk* refresh_psk_;

    ClientPsk* new_access_psk_;
    ClientPsk* new_refresh_psk_;

    uint32 cert_region_;
    std::vector<std::string> fallback_urls_vec_;
};

}  // namespace mmtls
