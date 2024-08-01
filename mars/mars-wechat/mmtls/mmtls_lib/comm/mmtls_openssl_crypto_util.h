#pragma once

#include <string>

#include "mmtls_ciphersuite.h"
#include "mmtls_crypto_util.h"
#include "mmtls_string.h"
#include "mmtls_types.h"
#include "openssl/evp.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

namespace mmtls {

/**
 * OpenSSL不是线程安全的，参考这里：https://wiki.openssl.org/index.php/Random_Numbers
 * 为了保证OpenSSL的线程安全，需要调用CRYPTO_set_locking_callback.
 * 使用OpenSSL自身提供的锁机制，能实现更小的加锁粒度，获得更好的并发性能
 * 示例代码: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/mmcomm_proj/trunk/mmcomm/mmutil/MmSslSocket.cpp : class SslEnv
 */

// The openssl crypto util implemtation.
class OpenSslCryptoUtil : public CryptoUtil {
 public:
    OpenSslCryptoUtil();

    OpenSslCryptoUtil(const CipherSuite& ciphersuite);

    virtual ~OpenSslCryptoUtil();

    // The API implementations.
    // The hash function hinted by ciphersuite_ determine the length of pseudorandom_key in octets.
    virtual int HkdfExtract(const mmtls::String& salt, const mmtls::String& key, mmtls::String& pseudorandom_key);

    // The key derivation method specified in rfc5869
    virtual int HkdfExpand(const mmtls::String& pseudorandom_key,
                           const mmtls::String& info,
                           const size_t key_material_len,
                           mmtls::String& key_material);

    virtual int HkdfDeriveKey(const mmtls::String& secret,
                              const mmtls::String& handshake_hash,
                              const mmtls::String& label,
                              size_t result_len,
                              mmtls::String& result);

    // The key derivation method specified in rfc5246
    // See https://tools.ietf.org/html/rfc5246#page-14
    virtual int Tls1Prf(const mmtls::String& secret,
                        const mmtls::String& seed,
                        const size_t result_len,
                        mmtls::String& result);

    virtual int MessageAuthCode(const mmtls::String& key, const mmtls::String& message, mmtls::String& mac);

    virtual int GenEcdhKeyPair(int nid, KeyPair& key_pair);

    virtual int ComputeDh(int nid,
                          const mmtls::String& public_material,
                          const mmtls::String& private_material,
                          mmtls::String& result);

    virtual int SignMessage(const mmtls::String& private_key, const mmtls::String& message, mmtls::String& signature);

    virtual int VerifyMessage(const mmtls::String& public_key,
                              const mmtls::String& signature,
                              const mmtls::String& message);

    virtual int MessageHash(const mmtls::String& message, mmtls::String& result);

    virtual int GenerateRandom(size_t result_len, mmtls::String& result);

    virtual int HkdfExtract(const byte* salt,
                            size_t salt_size,
                            const byte* key,
                            size_t key_size,
                            mmtls::String& pseudorandom_key);

    virtual int HkdfExpand(const byte* pseudorandom_key,
                           size_t pseudorandom_key_size,
                           const byte* info,
                           size_t info_size,
                           const size_t key_material_len,
                           mmtls::String& key_material);

    virtual int HkdfDeriveKey(const byte* secret,
                              size_t secret_size,
                              const byte* handshake_hash,
                              size_t handshake_hash_size,
                              const byte* label,
                              size_t label_size,
                              size_t result_len,
                              mmtls::String& result);

    virtual int Tls1Prf(const byte* secret,
                        size_t secret_size,
                        const byte* seed,
                        size_t seed_size,
                        const size_t result_len,
                        mmtls::String& result);

    virtual int MessageAuthCode(const byte* key,
                                size_t key_size,
                                const byte* message,
                                size_t message_size,
                                mmtls::String& mac);

    virtual int ComputeDh(int nid,
                          const byte* public_material,
                          size_t public_material_size,
                          const byte* private_material,
                          size_t private_material_size,
                          mmtls::String& result);

    virtual int SignMessage(const byte* private_key,
                            size_t private_key_size,
                            const byte* message,
                            size_t message_size,
                            mmtls::String& signature);

    virtual int VerifyMessage(const byte* public_key,
                              size_t public_key_size,
                              const byte* signature,
                              size_t signature_size,
                              const byte* message,
                              size_t message_size);

    virtual int MessageHash(const byte* message, size_t message_size, mmtls::String& result);

    // Get default OpenSslCryptoUtil instance, just for being easy to invoke.
    static OpenSslCryptoUtil& GetDefault();

 private:
    unsigned char* HKDF(const EVP_MD* evp_md,
                        const unsigned char* salt,
                        size_t salt_len,
                        const unsigned char* key,
                        size_t key_len,
                        const unsigned char* info,
                        size_t info_len,
                        unsigned char* okm,
                        size_t okm_len);

    unsigned char* HKDF_Extract(const EVP_MD* evp_md,
                                const unsigned char* salt,
                                size_t salt_len,
                                const unsigned char* key,
                                size_t key_len,
                                unsigned char* prk,
                                size_t* prk_len);

    unsigned char* HKDF_Expand(const EVP_MD* evp_md,
                               const unsigned char* prk,
                               size_t prk_len,
                               const unsigned char* info,
                               size_t info_len,
                               unsigned char* okm,
                               size_t okm_len);

    int Tls1_P_Hash(const EVP_MD* md,
                    const unsigned char* sec,
                    int sec_len,
                    const void* seed1,
                    int seed1_len,
                    const void* seed2,
                    int seed2_len,
                    const void* seed3,
                    int seed3_len,
                    const void* seed4,
                    int seed4_len,
                    const void* seed5,
                    int seed5_len,
                    unsigned char* out,
                    int olen);

    int HmacSha256(const byte* key, size_t key_size, const byte* message, size_t message_size, mmtls::String& mac);

    int Ecdh(int nid,
             const byte* public_material,
             size_t public_material_size,
             const byte* private_material,
             size_t private_material_size,
             mmtls::String& result);

    int EcdsaSign(const byte* private_key,
                  size_t private_key_size,
                  const byte* message,
                  size_t message_size,
                  mmtls::String& signature);

    int EcdsaVerify(const byte* public_key,
                    size_t public_key_size,
                    const byte* signature,
                    size_t signature_size,
                    const byte* message,
                    size_t message_size);

    int Sha256(const byte* message, size_t message_size, mmtls::String& result);

    const EVP_MD* GetHashFunc();

    class OpenSslMd5Digest : public Digest {
     public:
        friend class OpenSslCryptoUtil;
        virtual ~OpenSslMd5Digest(){};
        virtual int Update(const byte* message, size_t message_size);
        virtual int Final(mmtls::String& result);

     protected:
        OpenSslMd5Digest();

     private:
        MD5_CTX ctx_;
        bool has_final_;
    };
    virtual Digest* CreateDigest();

    class OpenSslHash : public Hash {
     public:
        friend class OpenSslCryptoUtil;
        virtual Hash* Clone() const;
        virtual ~OpenSslHash();
        virtual int Update(const byte* message, size_t message_size);
        virtual int Final(mmtls::String& result);

     protected:
        // OpenSslHash(){}
        OpenSslHash(const EVP_MD* md);
        OpenSslHash(const EVP_MD_CTX* ctx);

     protected:
        EVP_MD_CTX* ctx_;
        bool has_final_;
    };

    class OpenSslHash256 : public Hash {
     public:
        friend class OpenSslCryptoUtil;
        virtual Hash* Clone() const;
        virtual ~OpenSslHash256();
        virtual int Update(const byte* message, size_t message_size);
        virtual int Final(mmtls::String& result);

     protected:
        OpenSslHash256();
        OpenSslHash256(const SHA256_CTX& ctx);

     protected:
        SHA256_CTX ctx_;
        bool has_final_;
    };

    virtual Hash* CreateHash();

 private:
    CipherSuite ciphersuite_;
};

}  // namespace mmtls
