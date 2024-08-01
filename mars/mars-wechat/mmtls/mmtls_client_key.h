#ifndef _MMTLS_CLIENT_KEY_H_
#define _MMTLS_CLIENT_KEY_H_

#include <string>
#include <vector>

namespace mmtls {

struct MMTlsKey {
 public:
    std::string key_version;
    std::string ecdh_key_0;
    std::string ecdh_key_0_md5;
    std::string ecdh_key_1;
    std::string ecdh_key_1_md5;
    std::string ecdsa_key;
    std::string ecdsa_key_md5;
};

struct NewMMTlsKey {
 public:
    NewMMTlsKey() {
    }
    NewMMTlsKey(const char* _version,
                const char* _mmtls_g_ecdh_keys[],
                int _ecdh_key_count,
                const char* _mmtls_g_ecdsa_keys[],
                int _ecdsa_key_count) {
        key_version = _version;
        ecdh_key_list.clear();
        ecdsa_key_list.clear();
        for (int i = 0; i < _ecdh_key_count; i++) {
            ecdh_key_list.push_back(_mmtls_g_ecdh_keys[i]);
        }
        for (int i = 0; i < _ecdsa_key_count; i++) {
            ecdsa_key_list.push_back(_mmtls_g_ecdsa_keys[i]);
        }
    }

 public:
    std::string key_version;
    std::vector<std::string> ecdh_key_list;
    std::vector<std::string> ecdsa_key_list;
};

extern NewMMTlsKey gDefaultKey;
extern NewMMTlsKey gILinkKey;
// extern MMTlsKey gExternalTlsKey;

}  // namespace mmtls

#endif
