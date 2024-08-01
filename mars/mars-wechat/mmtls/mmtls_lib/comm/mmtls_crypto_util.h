#pragma once

#include "mmtls_key_pair.h"
#include "mmtls_string.h"

namespace mmtls {

// The cryptograpic primitive interface.
class CryptoUtil {
 public:
    virtual ~CryptoUtil() {
    }

    virtual int HkdfExtract(const mmtls::String& salt, const mmtls::String& key, mmtls::String& pseudorandom_key) = 0;

    virtual int HkdfExpand(const mmtls::String& pseudorandom_key,
                           const mmtls::String& info,
                           const size_t key_material_len,
                           mmtls::String& key_material) = 0;

    virtual int HkdfDeriveKey(const mmtls::String& secret,
                              const mmtls::String& handshake_hash,
                              const mmtls::String& label,
                              size_t result_len,
                              mmtls::String& result) = 0;

    virtual int Tls1Prf(const mmtls::String& secret,
                        const mmtls::String& seed,
                        const size_t result_len,
                        mmtls::String& result) = 0;

    virtual int MessageAuthCode(const mmtls::String& key, const mmtls::String& message, mmtls::String& mac) = 0;

    virtual int GenEcdhKeyPair(int nid, KeyPair& key_pair) = 0;

    virtual int ComputeDh(int nid,
                          const mmtls::String& public_value,
                          const mmtls::String& private_value,
                          mmtls::String& result) = 0;

    virtual int SignMessage(const mmtls::String& private_key,
                            const mmtls::String& message,
                            mmtls::String& signature) = 0;

    virtual int VerifyMessage(const mmtls::String& public_key,
                              const mmtls::String& signature,
                              const mmtls::String& message) = 0;

    virtual int MessageHash(const mmtls::String& message, mmtls::String& result) = 0;

    virtual int GenerateRandom(size_t result_len, mmtls::String& result) = 0;

    virtual int HkdfExtract(const byte* salt,
                            size_t salt_size,
                            const byte* key,
                            size_t key_size,
                            mmtls::String& pseudorandom_key) = 0;

    virtual int HkdfExpand(const byte* pseudorandom_key,
                           size_t pseudorandom_key_size,
                           const byte* info,
                           size_t info_size,
                           const size_t key_material_len,
                           mmtls::String& key_material) = 0;

    virtual int Tls1Prf(const byte* secret,
                        size_t secret_size,
                        const byte* seed,
                        size_t seed_size,
                        const size_t result_len,
                        mmtls::String& result) = 0;

    virtual int HkdfDeriveKey(const byte* secret,
                              size_t secret_size,
                              const byte* handshake_hash,
                              size_t handshake_hash_size,
                              const byte* label,
                              size_t label_size,
                              size_t result_len,
                              mmtls::String& result) = 0;

    virtual int MessageAuthCode(const byte* key,
                                size_t key_size,
                                const byte* message,
                                size_t message_size,
                                mmtls::String& mac) = 0;

    virtual int ComputeDh(int nid,
                          const byte* public_material,
                          size_t public_material_size,
                          const byte* private_material,
                          size_t private_material_size,
                          mmtls::String& result) = 0;

    virtual int SignMessage(const byte* private_key,
                            size_t private_key_size,
                            const byte* message,
                            size_t message_size,
                            mmtls::String& signature) = 0;

    virtual int VerifyMessage(const byte* public_key,
                              size_t public_key_size,
                              const byte* signature,
                              size_t signature_size,
                              const byte* message,
                              size_t message_size) = 0;

    virtual int MessageHash(const byte* message, size_t message_size, mmtls::String& result) = 0;

    class Digest {
     public:
        virtual ~Digest() {
        }
        virtual int Update(const byte* message, size_t message_size) = 0;
        virtual int Final(mmtls::String& result) = 0;

     protected:
        Digest() {
        }
    };
    virtual Digest* CreateDigest() = 0;

    class Hash {
     public:
        virtual ~Hash() {
        }
        virtual Hash* Clone() const = 0;
        virtual int Update(const byte* message, size_t message_size) = 0;
        virtual int Final(mmtls::String& result) = 0;

     protected:
        Hash() {
        }
    };

    virtual Hash* CreateHash() = 0;
};

}  // namespace mmtls
