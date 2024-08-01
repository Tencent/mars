/*
 * mmtls_client_credential_storage.h
 *
 *  Created on: 2015��11��16��
 *      Author: elviswu
 */

#ifndef MMTLS_CLIENT_CREDENTIAL_STORAGE_H_
#define MMTLS_CLIENT_CREDENTIAL_STORAGE_H_

#include <map>

#include "mars/boot/context.h"
#include "mmtls_credential_storage.h"

namespace mars {
namespace comm {
class Mutex;
}
}  // namespace mars

namespace mmtls {

#define PSK_KEY_FILENAME "psk.key."
#define ECDH_KEY_FILENAME "ecdh.key."
#define ECDSA_KEY_FILENAME "ecdsa.key."

// The client implementation for credential storage.
class ClientCredStorage : public CredentialStorage {
 public:
    ClientCredStorage(mars::boot::Context* _context, const std::string& _dir, const std::string& _group);
    ~ClientCredStorage();

    virtual int SavePsk(PskType type, const mmtls::String& serialized_client_psk);

    virtual int GetPsk(PskType type, mmtls::String& serialized_client_psk);

    virtual int DeletePsk(PskType type);

    virtual int GetStaticEcdhKeyPairs(std::vector<KeyPair>& key_pairs);

    virtual int GetStaticEcdhKeyPair(uint32 version, KeyPair& key_pair);

    virtual int GetEcdsaKeyPairs(std::vector<KeyPair>& key_pairs);

    virtual int GetEcdsaKeyPair(uint32 version, KeyPair& key_pair);

    // Accessor
    inline void set_key_dir(const mmtls::String& dir) {
        key_dir_ = dir;
    }

    // Initialize client credential storage.
    void Init();

    void InitTestKeys();

    // Get singleton.
    // @key_dir, the key file directory
    // static ClientCredStorage &GetInstance();
    // static void Release();

 private:
    enum KeyType {
        KEY_TYPE_PSK = 1,
        KEY_TYPE_ECDH = 2,
        KEY_TYPE_ECDSA = 3,
    };

    struct KeyContainerSt {
        uint64 update_time_;
        std::map<uint32, KeyPair> keys_;
    };

    typedef struct KeyContainerSt KeyContainer;

 private:
    // Init PSK key.
    int InitPskKeys();
    // Init ECDH key.
    int InitEcdhKeys();
    // Init ECDSA key.
    int InitEcdsaKeys();

    int SaveRefreshPskToFile(bool is_delete = false);
    int LoadRefreshPskFromFile();

 private:
    mars::boot::Context* context_;
    mmtls::String serialized_acess_psk_;
    mmtls::String serialized_refresh_psk_;

    KeyContainer ecdh_keys_;
    KeyContainer ecdsa_keys_;

    // key's main directory
    mmtls::String key_dir_;

    // protect all data members
    mars::comm::Mutex* member_mutex;
    std::string group_;

    // static ClientCredStorage* s_Instance;
    // static Mutex* s_mutex;
};

}  // end namespace mmtls

#endif /* MMTLS_CLIENT_CREDENTIAL_STORAGE_H_ */
