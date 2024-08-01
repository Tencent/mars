#include "mmtls_ciphersuite.h"

namespace mmtls {

std::vector<CipherSuite::CipherSuiteInfo> CipherSuite::ciphersuite_infos;

CipherSuite CipherSuite::non_ciphersuite(0);

void CipherSuite::InitCipherSuiteInfo() {
    // Only C++11 support initialize vector from array. So we can only initialize in constructor.
    if (ciphersuite_infos.empty()) {
        // Nonce length illustration:
        // 1. In rfc5288 the nonce length for AES-GCM cipher suite in TLS is 12bytes.
        // 2. In TLS1.3 draft specification nonce length is max(8, N_MIN). In rfc5116 N_MIN is 12.
        // In AEAD mode mac key is not used so its zero.
        ciphersuite_infos.push_back(CipherSuiteInfo(0, "", "", "", "", "", 0, 0, 0));
        ciphersuite_infos.push_back(CipherSuiteInfo(TLS_PSK_WITH_AES_128_GCM_SHA256,
                                                    "PSK",
                                                    "ECDSA",
                                                    "SHA256",
                                                    "AES_128_GCM",
                                                    "AEAD",
                                                    16,
                                                    0,
                                                    12));
        ciphersuite_infos.push_back(CipherSuiteInfo(TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
                                                    "ECDHE",
                                                    "ECDSA",
                                                    "SHA256",
                                                    "AES_128_GCM",
                                                    "AEAD",
                                                    16,
                                                    0,
                                                    12));
    }
}

CipherSuite::CipherSuite() : ciphersuite_code_(0) {
    InitCipherSuiteInfo();
    ciphersuite_info_ = &ciphersuite_infos[0];
}

CipherSuite::CipherSuite(uint16 ciphersuite_code) : ciphersuite_code_(ciphersuite_code) {
    InitCipherSuiteInfo();
    ciphersuite_info_ = &ciphersuite_infos[0];
    for (size_t i = 1; i < ciphersuite_infos.size(); ++i) {
        if (ciphersuite_code_ == ciphersuite_infos[i].ciphersuite_code) {
            ciphersuite_info_ = &ciphersuite_infos[i];
            break;
        }
    }
}

CipherSuite::CipherSuite(uint16 ciphersuite_code, const CipherSuiteInfo* ciphersuite_info)
: ciphersuite_code_(ciphersuite_code), ciphersuite_info_(ciphersuite_info) {
}

int CipherSuite::DoSerialize(SerializeWriter& writer) {
    writer.Write(ciphersuite_code_);

    return OK;
}

int CipherSuite::DoDeserialize(DeserializeReader& reader) {
    int ret = 0;

    ret = reader.Get(ciphersuite_code_);
    MMTLS_CHECK_EQ(ret, OK, ret, "get ciphersuite_code failed");

    return OK;
}

bool CipherSuite::IsValid() const {
    return ciphersuite_code_ == TLS_PSK_WITH_AES_128_GCM_SHA256
           || ciphersuite_code_ == TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256;
}

bool CipherSuite::IsPskCipherSuite() const {
    return ciphersuite_code_ == TLS_PSK_WITH_AES_128_GCM_SHA256;
}

bool CipherSuite::IsEccCipherSuite() const {
    return ciphersuite_code_ == TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256;
}

mmtls::String CipherSuite::ToString() const {
    return "TLS_" + key_exchange_algo() + "_" + sig_algo() + "_WITH_" + cipher_algo() + "_" + mac_algo();
}

CipherSuite CipherSuite::GetByCode(uint16_t ciphersuite_code) {
    return CipherSuite(ciphersuite_code);
}

std::vector<CipherSuite> CipherSuite::KnownCipherSuites() {
    std::vector<CipherSuite> suites;
    suites.push_back(GetByCode(TLS_PSK_WITH_AES_128_GCM_SHA256));
    suites.push_back(GetByCode(TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256));
    return suites;
}

}  // namespace mmtls
