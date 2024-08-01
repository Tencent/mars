#include "mmtls_client_handshake_state.h"

namespace mmtls {

ClientHandShakeState::~ClientHandShakeState() {
    ClearAll();
}

void ClientHandShakeState::ActiveSession() {
    HandShakeState::ActiveSession();
    ClearAll();
}

void ClientHandShakeState::ClearAll() {
    if (access_psk_ != NULL) {
        delete access_psk_;
        access_psk_ = NULL;
    }
    if (refresh_psk_ != NULL) {
        delete refresh_psk_;
        refresh_psk_ = NULL;
    }
    if (new_access_psk_ != NULL) {
        delete new_access_psk_;
        new_access_psk_ = NULL;
    }
    if (new_refresh_psk_ != NULL) {
        delete new_refresh_psk_;
        new_refresh_psk_ = NULL;
    }

    static_server_ecdh_keys_.clear();
    ephemeral_client_ecdh_keys_.clear();
    ecdsa_key_.Clear();
    ecdsa_keys_.clear();
}

}  // namespace mmtls
