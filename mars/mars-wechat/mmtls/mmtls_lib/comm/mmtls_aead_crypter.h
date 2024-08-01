#pragma once

#include <string>

#include "mmtls_constants.h"
#include "mmtls_version.h"

namespace mmtls {

// The interface for decryption.
// This interface defines APIs that are compatible for all supported algorithm. For now we only need to compatible with
// aead encryption. This interface is refer to
// https://chromium.googlesource.com/chromium/src/net/+/master/quic/crypto/quic_decrypter.h
class AeadCrypter {
 public:
    virtual ~AeadCrypter() {
    }

    static AeadCrypter* Create(CipherType type, bool is_encrypter);

    // The key is the server_write_key or client_write_key derived from SS, ES or master secret.
    // Return code see ErrorCode.
    virtual int SetKey(const byte* key, size_t key_size) = 0;
    virtual int SetKeyBySwap(mmtls::String& key) = 0;
    virtual const mmtls::String& GetKey() = 0;

    // This API is for aead nonce in TLS1.3. The iv is the server_write_iv or client_write_iv derived from SS, ES or
    // master secret. In TLS1.2, nonce is a concatenation of a fixed nonce prefix and a sequence number. In TLS1.3,
    // nonce is iv xor sequence See https://tlswg.github.io/tls13-spec/#record-payload-protection for nonce
    // illustration. Return code see ErrorCode.
    virtual int SetIv(const byte* iv, size_t iv_size) = 0;
    virtual int SetIvBySwap(mmtls::String& iv) = 0;
    virtual const mmtls::String& GetIv() = 0;

    // Get crypt output size
    // @param input_size, the plaintext size
    // @return the ciphertext size
    virtual size_t GetCipherTextSize(size_t input_size) const = 0;

    // This API specify all parameters explicitly without relying on any internal state.
    // Return code see ErrorCode.
    virtual int Crypt(const byte* nonce,
                      size_t nonce_size,
                      const byte* aad,
                      size_t aad_size,
                      const byte* input,
                      size_t input_size,
                      mmtls::String& output) = 0;

 protected:
    static const size_t kMinIvSize = 8;
    static const size_t kMaxIvSize = 16;
    static const size_t kMaxKeySize = 32;
    static const size_t kMaxTagSize = 16;
};

};  // namespace mmtls
