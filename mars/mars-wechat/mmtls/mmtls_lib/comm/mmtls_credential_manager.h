#pragma once

#include <string>

#include "mmtls_constants.h"
#include "mmtls_key_pair.h"
#include "mmtls_psk.h"

namespace mmtls {

// The credential manager interface. It manages long term credentials.
// 1. ECDH static public/private value
// 3. ECDSA public/private key
class CredentialManager {
 public:
    virtual ~CredentialManager() {
    }

    virtual int GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs) = 0;

    virtual int GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair) = 0;

    virtual int GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs) = 0;

    virtual int GetEcdsaKeyPair(uint32 version, KeyPair& key_pair) = 0;
};

}  // namespace mmtls
