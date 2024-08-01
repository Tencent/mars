#pragma once

#include <string>

#include "mmtls_string.h"

namespace mmtls {

// The keys for connections of the same state.
// See https://github.com/randombit/botan/blob/6a51315c4bea471a5093b0ce621643b7012a7d8c/src/lib/tls/tls_session_key.h
// Different from TLS1.2, TLS1.3 has 3 sets of key generation requirement, so the key generation operation will be in
// another class.
class ConnectionKeys {
 public:
    ConnectionKeys(const char* client_write_key,
                   size_t client_write_key_size,
                   const char* server_write_key,
                   size_t server_write_key_size,
                   const char* client_write_iv,
                   size_t client_write_iv_size,
                   const char* server_write_iv,
                   size_t server_write_iv_size)
    : client_write_key_(client_write_key, client_write_key_size)
    , server_write_key_(server_write_key, server_write_key_size)
    , client_write_iv_(client_write_iv, client_write_iv_size)
    , server_write_iv_(server_write_iv, server_write_iv_size) {
    }

    ~ConnectionKeys() {
        client_write_key_.safe_clear();
        client_write_iv_.safe_clear();
        server_write_key_.safe_clear();
        server_write_iv_.safe_clear();
    }

    mmtls::String& client_write_key() {
        return client_write_key_;
    }
    mmtls::String& server_write_key() {
        return server_write_key_;
    }

    mmtls::String& client_write_iv() {
        return client_write_iv_;
    }
    mmtls::String& server_write_iv() {
        return server_write_iv_;
    }

 protected:
    ConnectionKeys() {
    }

 private:
    mmtls::String client_write_key_;
    mmtls::String server_write_key_;
    mmtls::String client_write_iv_;
    mmtls::String server_write_iv_;
};

}  // namespace mmtls
