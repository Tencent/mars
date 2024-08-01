#pragma once

#include <map>
#include <string>
#include <vector>

#include "mmtls_data_pack.h"
#include "mmtls_string.h"
#include "mmtls_types.h"

namespace mmtls {

/**
 * MMTLS cipher suite
 */
class CipherSuite : public DataPackInterface {
 public:
    // The cipher suites that MMTLS supports.
    // See https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-4
    enum CipherSuiteCode { TLS_PSK_WITH_AES_128_GCM_SHA256 = 0x00A8, TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 = 0xC02B };

    static CipherSuite GetByCode(uint16 ciphersuite_code);
    static std::vector<CipherSuite> KnownCipherSuites();

    CipherSuite();

    bool IsValid() const;

    bool IsPskCipherSuite() const;

    bool IsEccCipherSuite() const;

    uint16 ciphersuite_code() const {
        return ciphersuite_code_;
    }

    const mmtls::String& key_exchange_algo() const {
        return ciphersuite_info_->key_exchange_algo;
    }

    const mmtls::String& prf_algo() const {
        return ciphersuite_info_->prf_algo;
    }

    const mmtls::String& sig_algo() const {
        return ciphersuite_info_->sig_algo;
    }

    const mmtls::String& cipher_algo() const {
        return ciphersuite_info_->cipher_algo;
    }

    const mmtls::String& mac_algo() const {
        return ciphersuite_info_->mac_algo;
    }

    uint32 cipher_key_len() const {
        return ciphersuite_info_->cipher_key_len;
    }

    uint32 mac_key_len() const {
        return ciphersuite_info_->mac_key_len;
    }

    uint32 nonce_len() const {
        return ciphersuite_info_->nonce_len;
    }

    mmtls::String ToString() const;
    bool operator==(const CipherSuite& other) const {
        return this->ciphersuite_code_ == other.ciphersuite_code_;
    }

 protected:
    // Serialize object to network order binary data
    virtual int DoSerialize(SerializeWriter& writer);
    // Deserialize from network order binary data
    virtual int DoDeserialize(DeserializeReader& reader);

 private:
    struct CipherSuiteInfo {
        uint16 ciphersuite_code;
        mmtls::String key_exchange_algo;
        mmtls::String sig_algo;
        mmtls::String prf_algo;
        mmtls::String cipher_algo;
        mmtls::String mac_algo;
        // key length in bytes.
        uint32 cipher_key_len;
        // key length in bytes.
        uint32 mac_key_len;
        // nonce length in bytes.
        uint32 nonce_len;

        CipherSuiteInfo(uint16 ciphersuite_code,
                        const char* key_exchange_algo,
                        const char* sig_algo,
                        const char* prf_algo,
                        const char* cipher_algo,
                        const char* mac_algo,
                        uint32 cipher_key_len,
                        uint32 mac_key_len,
                        uint32 nonce_len)
        : ciphersuite_code(ciphersuite_code)
        , key_exchange_algo(key_exchange_algo)
        , sig_algo(sig_algo)
        , prf_algo(prf_algo)
        , cipher_algo(cipher_algo)
        , mac_algo(mac_algo)
        , cipher_key_len(cipher_key_len)
        , mac_key_len(mac_key_len)
        , nonce_len(nonce_len) {
        }
    };
    static void InitCipherSuiteInfo();
    static std::vector<CipherSuiteInfo> ciphersuite_infos;
    // This static object is to make sure CipherSuite constructor is called then ciphersuite_infos will be initialized.
    static CipherSuite non_ciphersuite;

    CipherSuite(uint16 ciphersuite_code);
    // For testing
    CipherSuite(uint16 ciphersuite_code, const CipherSuiteInfo* ciphersuite_info);

    uint16 ciphersuite_code_;
    const CipherSuiteInfo* ciphersuite_info_;
};

}  // namespace mmtls
