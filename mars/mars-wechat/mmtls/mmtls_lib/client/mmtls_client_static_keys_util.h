/*
 * mmtls_client_static_keys_util.h
 *
 *  Created on: 2015年12月14日
 *      Author: elviswu
 */

#ifndef SRC_MMTLS_CLIENT_MMTLS_CLIENT_STATIC_KEYS_UTIL_H_
#define SRC_MMTLS_CLIENT_MMTLS_CLIENT_STATIC_KEYS_UTIL_H_

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "../../mmtls_client_key.h"
#include "mars/comm/strutil.h"
#include "mars/comm/xlogger/xlogger.h"
#include "mmtls_client_credential_manager.h"
#include "mmtls_key_pair.h"
#include "openssl/md5.h"

static unsigned char* str2hex(const char* str) {
    size_t length = strlen(str);
    char* src = (char*)malloc(length + 1);
    char* src_start = src;
    strncpy(src, str, length + 1);
    unsigned char* buf;
    unsigned char* dst;
    unsigned int value;

    for (size_t i = 0; i < length; i++) {
        if (!isxdigit(src[i])) {
            xerror2(TSF "error: the string given as an argument is not in hexadecimal, str=%_.", str);
            free(src);
            return NULL;
        }
    }

    if ((length % 2) != 0) {
        xerror2(TSF "error: the length of the string given as an argument have to be a multiple of 2, str=%_.", str);
    }

    buf = (unsigned char*)calloc(length / 2 + 1, sizeof(unsigned char));

    dst = buf;

    while (*src && (sscanf(src, "%2x", &value) == 1)) {
        *dst = (unsigned char)value;
        src += 2;
        dst++;
    }
    free(src_start);
    return buf;
}

static bool CheckStaticKeyIntegrity(const char* p_static_key,
                                    size_t static_key_len,
                                    const char* p_key_md5,
                                    size_t key_md5_len) {
    // xassert2(32==key_md5_len);
    // if (32!=key_md5_len) {
    // 	xerror2(TSF"error md5 len. key_md5_len=%_", key_md5_len);
    // 	return false;
    // }
    // unsigned char sig[16] = {0};
    // MD5((const unsigned char*)p_static_key, static_key_len, sig);

    // std::string des = strutil::MD5DigestToBase16(sig);
    // return 0==strncmp(des.c_str(), p_key_md5, 32);
    return true;
}

static std::vector<mmtls::KeyPair> GetEcdhStaticKey(const std::string& _group) {
    std::vector<mmtls::KeyPair> key_list;
    const mmtls::NewMMTlsKey& key = mmtls::ClientCredentialManager::TlsKey(_group);
    for (size_t i = 0; i < key.ecdh_key_list.size(); i++) {
        mmtls::KeyPair static_key_pair;
        bool isIntegrity = CheckStaticKeyIntegrity(key.ecdh_key_list[i].c_str(), key.ecdh_key_list[i].length(), "", 0);
        if (isIntegrity) {
            unsigned char* p_static_key = str2hex(key.ecdh_key_list[i].c_str());
            mmtls::String static_key((char*)p_static_key, key.ecdh_key_list[i].length() / 2);
            mmtls::DataReader reader(static_key);
            int ret = static_key_pair.Deserialize(reader);
            xassert2(ret == 0);
            free(p_static_key);
            key_list.push_back(static_key_pair);
        } else {
            xerror2(TSF "ecdh list :%_ md5 check failed.", i);
        }
    }
    xinfo2(TSF "init ecdh size: %_, group:%_, key_list:%_", key_list.size(), _group, key.ecdh_key_list.size());
    return key_list;
}

mmtls::KeyPair GetEcdhNormalKey(const std::string& _group) {
    // const mmtls::NewMMTlsKey& key = mmtls::ClientCredentialManager::TlsKey(_group);
    mmtls::KeyPair static_key_pair;
    // bool isIntegrity = CheckStaticKeyIntegrity(key.ecdh_key_1.c_str(), key.ecdh_key_1.length(),
    // 							key.ecdh_key_1_md5.c_str(), key.ecdh_key_1_md5.length());
    // if (isIntegrity) {
    // 	unsigned char* p_static_key = str2hex(key.ecdh_key_1.c_str());
    // 	mmtls::String static_key((char*)p_static_key, key.ecdh_key_1.length()/2);
    // 	mmtls::DataReader reader(static_key);
    // 	int ret = static_key_pair.Deserialize(reader);
    // 	xassert2(ret == 0);
    // 	free(p_static_key);
    // } else {
    // 	xerror2(TSF"mmtls_g_ecdh_key_1 md5 check failed.");
    // }
    return static_key_pair;
}

std::vector<mmtls::KeyPair> GetEcdsaKey(const std::string& _group) {
    std::vector<mmtls::KeyPair> key_list;
    const mmtls::NewMMTlsKey& key = mmtls::ClientCredentialManager::TlsKey(_group);
    for (size_t i = 0; i < key.ecdsa_key_list.size(); i++) {
        mmtls::KeyPair static_key_pair;
        bool isIntegrity =
            CheckStaticKeyIntegrity(key.ecdsa_key_list[i].c_str(), key.ecdsa_key_list[i].length(), "", 0);
        if (isIntegrity) {
            unsigned char* p_static_key = str2hex(key.ecdsa_key_list[i].c_str());
            mmtls::String static_key((char*)p_static_key, key.ecdsa_key_list[i].length() / 2);
            mmtls::DataReader reader(static_key);
            int ret = static_key_pair.Deserialize(reader);
            xassert2(ret == 0);
            free(p_static_key);
            key_list.push_back(static_key_pair);
        } else {
            xerror2(TSF "ecdsa list %_ md5 check failed.", i);
        }
    }
    xinfo2(TSF "init ecdsa size: %_", key_list.size());
    return key_list;
}

#endif /* SRC_MMTLS_CLIENT_MMTLS_CLIENT_STATIC_KEYS_UTIL_H_ */
