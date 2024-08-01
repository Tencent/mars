#pragma once

#include <string>
#include <vector>

#include "mmtls_constants.h"
#include "mmtls_key_pair.h"
#include "mmtls_psk.h"
#include "mmtls_string.h"

namespace mmtls {

// The interface for credential storage.
// Different platform will have different implementations.
class CredentialStorage {
 public:
    CredentialStorage() {
    }
    virtual ~CredentialStorage() {
    }

    virtual int GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs) = 0;

    virtual int GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair) = 0;

    virtual int GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs) = 0;

    virtual int GetEcdsaKeyPair(uint32 version, KeyPair& key_pair) = 0;
};

}  // namespace mmtls
