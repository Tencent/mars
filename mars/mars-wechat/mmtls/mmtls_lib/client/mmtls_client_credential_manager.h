#pragma once

#include "../../mmtls_client_key.h"
#include "mars/boot/context.h"
#include "mmtls_client_psk.h"
#include "mmtls_credential_manager.h"
#include "mmtls_credential_storage.h"

namespace mmtls {
class ClientCredStorage;
// The credential manager implementation for client.
class ClientCredentialManager : public CredentialManager {
 public:
    static ClientCredentialManager& GetInstance(mars::boot::Context* _context, const std::string& _name);

    ClientCredentialManager(mars::boot::Context* _context, const std::string& _name);

    virtual ~ClientCredentialManager();

    virtual int GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs);

    virtual int GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair);

    virtual int GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs);

    virtual int GetEcdsaKeyPair(uint32 version, KeyPair& key_pair);

    virtual int SavePsk(const ClientPsk& psk);

    virtual int GetValidPsk(PskType type, ClientPsk** psk);

    virtual int DeletePsk(PskType type);

    static bool AddKey(const std::string& _group, const NewMMTlsKey& _key);

    static const NewMMTlsKey& TlsKey(const std::string& _group);

 private:
    ClientCredStorage* client_storage_;
};

}  // namespace mmtls
