#include "mmtls_openssl_crypto_util.h"

#include <string.h>

#include <string>

#include "mmtls_constants.h"
#include "mmtls_log.h"
#include "mmtls_utils.h"
#include "openssl/bio.h"
#include "openssl/ecdsa.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/hmac.h"
#include "openssl/pem.h"
#include "openssl/rand.h"
#include "openssl/sha.h"

/**
 * OpenSslCryptoUtil
 */

namespace mmtls {

#define KDF_SHA256_LENGTH SHA256_DIGEST_LENGTH

static inline unsigned char* StringPreAlloc(mmtls::String& str, size_t size) {
    str.clear();
    str.resize(size);

    return (unsigned char*)&str[0];
}

static void* KdfSha256(const void* in, size_t in_len, void* out, size_t* out_len) {
    if ((!out_len) || (!in) || (!in_len) || *out_len < KDF_SHA256_LENGTH)
        return NULL;
    else
        *out_len = KDF_SHA256_LENGTH;

    return SHA256((const unsigned char*)in, in_len, (unsigned char*)out);
}

OpenSslCryptoUtil::OpenSslCryptoUtil()
: ciphersuite_(CipherSuite::GetByCode(CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256)) {
}

OpenSslCryptoUtil::OpenSslCryptoUtil(const CipherSuite& ciphersuite) : ciphersuite_(ciphersuite) {
}

OpenSslCryptoUtil::~OpenSslCryptoUtil() {
}

int OpenSslCryptoUtil::HkdfExtract(const mmtls::String& salt,
                                   const mmtls::String& key,
                                   mmtls::String& pseudorandom_key) {
    return this->HkdfExtract((const byte*)salt.data(),
                             salt.size(),
                             (const byte*)key.data(),
                             key.size(),
                             pseudorandom_key);
}

int OpenSslCryptoUtil::HkdfExpand(const mmtls::String& pseudorandom_key,
                                  const mmtls::String& info,
                                  const size_t key_material_len,
                                  mmtls::String& key_material) {
    return this->HkdfExpand((const byte*)pseudorandom_key.data(),
                            pseudorandom_key.size(),
                            (const byte*)info.data(),
                            info.size(),
                            key_material_len,
                            key_material);
}

int OpenSslCryptoUtil::HkdfDeriveKey(const mmtls::String& secret,
                                     const mmtls::String& handshake_hash,
                                     const mmtls::String& label,
                                     size_t result_len,
                                     mmtls::String& result) {
    return this->HkdfDeriveKey((const byte*)secret.data(),
                               secret.size(),
                               (const byte*)handshake_hash.data(),
                               handshake_hash.size(),
                               (const byte*)label.data(),
                               label.size(),
                               result_len,
                               result);
}

int OpenSslCryptoUtil::Tls1Prf(const mmtls::String& secret,
                               const mmtls::String& seed,
                               const size_t result_len,
                               mmtls::String& result) {
    return this->Tls1Prf((const byte*)secret.c_str(),
                         secret.size(),
                         (const byte*)seed.c_str(),
                         seed.size(),
                         result_len,
                         result);
}

int OpenSslCryptoUtil::MessageAuthCode(const mmtls::String& key, const mmtls::String& message, mmtls::String& mac) {
    return this->MessageAuthCode((const byte*)key.data(), key.size(), (const byte*)message.data(), message.size(), mac);
}

namespace internal {
const char* get_last_error_file(const ERR_STATE* es) {
    if (!es)
        return "N/A";
    if (es->top < 0 || es->top >= ERR_NUM_ERRORS)
        return "N/A";
    const char* errorfile = es->err_file[es->top];
    if (errorfile == nullptr)
        return "N/A";
    return errorfile;
}
int get_last_error_line(const ERR_STATE* es) {
    if (!es)
        return -1;
    if (es->top < 0 || es->top >= ERR_NUM_ERRORS)
        return -1;
    return es->err_line[es->top];
}
};  // namespace internal

int OpenSslCryptoUtil::GenEcdhKeyPair(int nid, KeyPair& key_pair) {
    int ret = -1;
    EC_KEY* ec_key = NULL;
    unsigned char* pub_key_buf = NULL;
    int pub_key_size = 0;
    unsigned char* pri_key_buf = NULL;
    int pri_key_size = 0;

    do {
        // create ec key by nid
        ec_key = EC_KEY_new_by_curve_name(nid);
        if (!ec_key) {
            const ERR_STATE* es = ERR_get_state();
            MMTLSLOG_IMPT("ERR: %s EC_KEY_new_by_curve_name failed, nid %d, file %s:%d",
                          __func__,
                          nid,
                          internal::get_last_error_file(es),
                          internal::get_last_error_line(es));
            ret = -1;
            break;
        }

        EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);
        // generate ec key pair
        ret = EC_KEY_generate_key(ec_key);
        if (ret != 1) {
            ERR_STATE* es = ERR_get_state();
            MMTLSLOG_IMPT("ERR: %s EC_KEY_generate_key failed, ret %d, file %s:%d",
                          __func__,
                          ret,
                          internal::get_last_error_file(es),
                          internal::get_last_error_line(es));
            ret = -1;
            break;
        }

        // get public key from ec key pair
        pub_key_size = i2o_ECPublicKey(ec_key, &pub_key_buf);
        if (pub_key_size == 0 || !pub_key_buf) {
            const ERR_STATE* es = ERR_get_state();
            MMTLSLOG_IMPT("ERR: %s i2o_ECPublicKey faild, ret %d, file %s:%d",
                          __func__,
                          ret,
                          internal::get_last_error_file(es),
                          internal::get_last_error_line(es));
            ret = -1;
            break;
        }

        // get private key from ec key pair
        pri_key_size = i2d_ECPrivateKey(ec_key, &pri_key_buf);
        if (pri_key_size == 0 || !pri_key_buf) {
            const ERR_STATE* es = ERR_get_state();
            MMTLSLOG_IMPT("ERR: %s i2d_ECPrivateKey failed, ret %d, file %s:%d",
                          __func__,
                          ret,
                          internal::get_last_error_file(es),
                          internal::get_last_error_line(es));
            ret = -1;
            break;
        }

        // set key_pair
        key_pair.set_version(0);
        key_pair.set_nid(nid);
        key_pair.set_public_key((const char*)pub_key_buf, pub_key_size);
        key_pair.set_private_key((const char*)pri_key_buf, pri_key_size);

        ret = 1;

    } while (0);

    // free memory
    if (ec_key) {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    if (pub_key_buf) {
        OPENSSL_free(pub_key_buf);
        pub_key_buf = NULL;
    }

    if (pri_key_buf) {
        OPENSSL_free(pri_key_buf);
        pri_key_buf = NULL;
    }

    if (ret != 1) {
        return ERR_GEN_ECDH_KEY_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::ComputeDh(int nid,
                                 const mmtls::String& public_material,
                                 const mmtls::String& private_material,
                                 mmtls::String& result) {
    return this->ComputeDh(nid,
                           (const byte*)public_material.data(),
                           public_material.size(),
                           (const byte*)private_material.data(),
                           private_material.size(),
                           result);
}

int OpenSslCryptoUtil::SignMessage(const mmtls::String& private_key,
                                   const mmtls::String& message,
                                   mmtls::String& signature) {
    return this->SignMessage((const byte*)private_key.data(),
                             private_key.size(),
                             (const byte*)message.data(),
                             message.size(),
                             signature);
}

int OpenSslCryptoUtil::VerifyMessage(const mmtls::String& public_key,
                                     const mmtls::String& signature,
                                     const mmtls::String& message) {
    return this->VerifyMessage((const byte*)public_key.data(),
                               public_key.size(),
                               (const byte*)signature.data(),
                               signature.size(),
                               (const byte*)message.data(),
                               message.size());
}

int OpenSslCryptoUtil::MessageHash(const mmtls::String& message, mmtls::String& result) {
    return this->MessageHash((const byte*)message.data(), message.size(), result);
}

int OpenSslCryptoUtil::GenerateRandom(size_t result_len, mmtls::String& result) {
    unsigned char* result_buf = StringPreAlloc(result, result_len);

    int ret = RAND_bytes(result_buf, (int)result_len);
    if (ret != 1) {
        return ERR_GEN_RANDOM_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::HkdfExtract(const byte* salt,
                                   size_t salt_size,
                                   const byte* key,
                                   size_t key_size,
                                   mmtls::String& pseudorandom_key) {
    if (!salt || !key || 0 == salt_size || 0 == key_size) {
        return ERR_ILLEGAL_PARAM;
    }

    unsigned char buffer[EVP_MAX_MD_SIZE];
    size_t len = 0;

    const EVP_MD* evp_md = GetHashFunc();
    MMTLS_CHECK_TRUE(evp_md, ERR_UNEXPECT_CHECK_FAIL, "GetHashFunc failed");

    unsigned char* result = this->HKDF_Extract(evp_md, salt, salt_size, key, key_size, buffer, &len);
    MMTLS_CHECK_TRUE(result, ERR_HKDF_FAIL, "HKDF_Extract failed");

    pseudorandom_key.assign((const char*)buffer, len);

    return OK;
}

int OpenSslCryptoUtil::HkdfExpand(const byte* pseudorandom_key,
                                  size_t pseudorandom_key_size,
                                  const byte* info,
                                  size_t info_size,
                                  const size_t key_material_len,
                                  mmtls::String& key_material) {
    if (!pseudorandom_key || !info || 0 == pseudorandom_key_size || 0 == info_size || 0 == key_material_len) {
        return ERR_ILLEGAL_PARAM;
    }

    const EVP_MD* evp_md = GetHashFunc();
    MMTLS_CHECK_TRUE(evp_md, ERR_UNEXPECT_CHECK_FAIL, "GetHashFunc failed");

    unsigned char* key_material_buf = StringPreAlloc(key_material, key_material_len);

    unsigned char* result = this->HKDF_Expand(evp_md,
                                              pseudorandom_key,
                                              pseudorandom_key_size,
                                              info,
                                              info_size,
                                              key_material_buf,
                                              key_material_len);
    MMTLS_CHECK_TRUE(result, ERR_HKDF_FAIL, "HKDF_Expand failed");

    return OK;
}

int OpenSslCryptoUtil::Tls1Prf(const byte* secret,
                               size_t secret_size,
                               const byte* seed,
                               size_t seed_size,
                               const size_t result_len,
                               mmtls::String& result) {
    if (!secret || !seed || 0 == secret_size || 0 == seed_size || 0 == result_len) {
        return ERR_ILLEGAL_PARAM;
    }

    const EVP_MD* evp_md = GetHashFunc();
    MMTLS_CHECK_TRUE(evp_md, ERR_UNEXPECT_CHECK_FAIL, "GetHashFunc failed");

    unsigned char* result_buf = StringPreAlloc(result, result_len);

    int ret = Tls1_P_Hash(evp_md,
                          secret,
                          (int)secret_size,
                          seed,
                          (int)seed_size,
                          NULL,
                          0,
                          NULL,
                          0,
                          NULL,
                          0,
                          NULL,
                          0,
                          result_buf,
                          (int)result_len);

    MMTLS_CHECK_EQ(1, ret, ERR_UNEXPECT_CHECK_FAIL, "Tls1_P_Hash fail");

    return OK;
}

int OpenSslCryptoUtil::HkdfDeriveKey(const byte* secret,
                                     size_t secret_size,
                                     const byte* handshake_hash,
                                     size_t handshake_hash_size,
                                     const byte* label,
                                     size_t label_size,
                                     size_t result_len,
                                     mmtls::String& result) {
    if (!secret || !handshake_hash || !label || 0 == secret_size || 0 == handshake_hash_size || 0 == label_size
        || 0 == result_len) {
        return ERR_ILLEGAL_PARAM;
    }

    // this call must be successful
    const EVP_MD* evp_md = GetHashFunc();
    MMTLS_CHECK_TRUE(evp_md, ERR_UNEXPECT_CHECK_FAIL, "GetHashFunc failed");

    // see section 2.3 at https://tools.ietf.org/html/rfc5869
    size_t dig_len = EVP_MD_size(evp_md);
    if (result_len > 255 * dig_len) {
        return ERR_ILLEGAL_PARAM;
    }

    unsigned char* result_buf = StringPreAlloc(result, result_len);

    unsigned char* res = this->HKDF(evp_md,
                                    handshake_hash,
                                    handshake_hash_size,
                                    secret,
                                    secret_size,
                                    label,
                                    label_size,
                                    result_buf,
                                    result_len);

    if (!res) {
        return ERR_DERIVE_KEY_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::MessageAuthCode(const byte* key,
                                       size_t key_size,
                                       const byte* message,
                                       size_t message_size,
                                       mmtls::String& mac) {
    if (!key || !message || 0 == key_size || 0 == message_size) {
        return ERR_ILLEGAL_PARAM;
    }

    if (ciphersuite_.prf_algo() == "SHA256") {
        return this->HmacSha256(key, key_size, message, message_size, mac);
    }

    return ERR_ILLEGAL_PARAM;
}

int OpenSslCryptoUtil::ComputeDh(int nid,
                                 const byte* public_material,
                                 size_t public_material_size,
                                 const byte* private_material,
                                 size_t private_material_size,
                                 mmtls::String& result) {
    if (!public_material || !private_material || 0 == public_material_size || 0 == private_material_size) {
        return ERR_ILLEGAL_PARAM;
    }

    if (ciphersuite_.key_exchange_algo() == "ECDHE") {
        return this->Ecdh(nid, public_material, public_material_size, private_material, private_material_size, result);
    }

    return ERR_ILLEGAL_PARAM;
}

int OpenSslCryptoUtil::SignMessage(const byte* private_key,
                                   size_t private_key_size,
                                   const byte* message,
                                   size_t message_size,
                                   mmtls::String& signature) {
    if (!private_key || !message || 0 == private_key_size || 0 == message_size) {
        return ERR_ILLEGAL_PARAM;
    }

    if (ciphersuite_.sig_algo() == "ECDSA") {
        return this->EcdsaSign(private_key, private_key_size, message, message_size, signature);
    }

    return ERR_ILLEGAL_PARAM;
}

int OpenSslCryptoUtil::VerifyMessage(const byte* public_key,
                                     size_t public_key_size,
                                     const byte* signature,
                                     size_t signature_size,
                                     const byte* message,
                                     size_t message_size) {
    if (!signature || !message || !public_key || 0 == public_key_size || 0 == signature_size || 0 == message_size) {
        return ERR_ILLEGAL_PARAM;
    }

    if (ciphersuite_.sig_algo() == "ECDSA") {
        return this->EcdsaVerify(public_key, public_key_size, signature, signature_size, message, message_size);
    }

    return ERR_ILLEGAL_PARAM;
}

int OpenSslCryptoUtil::MessageHash(const byte* message, size_t message_size, mmtls::String& result) {
    if (!message || 0 == message_size) {
        return ERR_ILLEGAL_PARAM;
    }

    return this->Sha256(message, message_size, result);
}

/**
 * HKDF(), HKDF_Extract() and HKDF_Expand() arc copied from
 * https://github.com/ghedo/openssl/commit/7378cb6e029205c17340c8ceffb9f964e3ef064d
 */
unsigned char* OpenSslCryptoUtil::HKDF(const EVP_MD* evp_md,
                                       const unsigned char* salt,
                                       size_t salt_len,
                                       const unsigned char* key,
                                       size_t key_len,
                                       const unsigned char* info,
                                       size_t info_len,
                                       unsigned char* okm,
                                       size_t okm_len) {
    unsigned char prk[EVP_MAX_MD_SIZE];
    size_t prk_len;

    if (!HKDF_Extract(evp_md, salt, salt_len, key, key_len, prk, &prk_len))
        return NULL;

    return HKDF_Expand(evp_md, prk, prk_len, info, info_len, okm, okm_len);
}

unsigned char* OpenSslCryptoUtil::HKDF_Extract(const EVP_MD* evp_md,
                                               const unsigned char* salt,
                                               size_t salt_len,
                                               const unsigned char* key,
                                               size_t key_len,
                                               unsigned char* prk,
                                               size_t* prk_len) {
    unsigned int tmp_len = 0;

    if (!HMAC(evp_md, salt, (int)salt_len, key, (int)key_len, prk, (unsigned int*)&tmp_len))
        return NULL;

    *prk_len = tmp_len;
    return prk;
}

unsigned char* OpenSslCryptoUtil::HKDF_Expand(const EVP_MD* evp_md,
                                              const unsigned char* prk,
                                              size_t prk_len,
                                              const unsigned char* info,
                                              size_t info_len,
                                              unsigned char* okm,
                                              size_t okm_len) {
    unsigned int i;

    unsigned char prev[EVP_MAX_MD_SIZE];

    size_t done_len = 0, dig_len = EVP_MD_size(evp_md);

    if (dig_len == 0) {
        return NULL;
    }

    size_t n = okm_len / dig_len;
    if (okm_len % dig_len)
        n++;

    if (n > 255)
        return NULL;

    HMAC_CTX* hmac = HMAC_CTX_new();
    HMAC_CTX_reset(hmac);

    if (!HMAC_Init_ex(hmac, prk, (int)prk_len, evp_md, NULL))
        goto err;

    for (i = 1; i <= n; i++) {
        size_t copy_len;
        const unsigned char ctr = i;

        if (i > 1) {
            if (!HMAC_Init_ex(hmac, NULL, 0, NULL, NULL))
                goto err;

            if (!HMAC_Update(hmac, prev, dig_len))
                goto err;
        }

        if (!HMAC_Update(hmac, info, info_len))
            goto err;

        if (!HMAC_Update(hmac, &ctr, 1))
            goto err;

        if (!HMAC_Final(hmac, prev, NULL))
            goto err;

        copy_len = (done_len + dig_len > okm_len) ? okm_len - done_len : dig_len;

        ::memcpy(okm + done_len, prev, copy_len);

        done_len += copy_len;
    }

    HMAC_CTX_free(hmac);
    return okm;

err:
    HMAC_CTX_free(hmac);
    return NULL;
}

// Copy from tls1.2 P_Hash
// https://github.com/openssl/openssl/blob/28ba2541f9f5e61ddef548d3bead494ff6946db2/ssl/t1_enc.c#L149
int OpenSslCryptoUtil::Tls1_P_Hash(const EVP_MD* md,
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
                                   int olen)

{
    int chunk;
    size_t j;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_MD_CTX* ctx_tmp = EVP_MD_CTX_new();
    EVP_MD_CTX* ctx_init = EVP_MD_CTX_new();
    EVP_PKEY* mac_key;
    unsigned char A1[EVP_MAX_MD_SIZE];
    size_t A1_len;
    int ret = 0;

    chunk = EVP_MD_size(md);
    MMTLS_CHECK_GE(chunk, 0, 1, "chun size 0");
    // OPENSSL_assert(chunk >= 0);

    EVP_MD_CTX_reset(ctx);
    EVP_MD_CTX_reset(ctx_tmp);
    EVP_MD_CTX_reset(ctx_init);
    EVP_MD_CTX_set_flags(ctx_init, EVP_MD_CTX_FLAG_NON_FIPS_ALLOW);
    mac_key = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, sec, sec_len);
    if (!mac_key)
        goto err;
    if (!EVP_DigestSignInit(ctx_init, NULL, md, NULL, mac_key))
        goto err;
    if (!EVP_MD_CTX_copy_ex(ctx, ctx_init))
        goto err;
    if (seed1 && !EVP_DigestSignUpdate(ctx, seed1, seed1_len))
        goto err;
    if (seed2 && !EVP_DigestSignUpdate(ctx, seed2, seed2_len))
        goto err;
    if (seed3 && !EVP_DigestSignUpdate(ctx, seed3, seed3_len))
        goto err;
    if (seed4 && !EVP_DigestSignUpdate(ctx, seed4, seed4_len))
        goto err;
    if (seed5 && !EVP_DigestSignUpdate(ctx, seed5, seed5_len))
        goto err;
    if (!EVP_DigestSignFinal(ctx, A1, &A1_len))
        goto err;

    for (;;) {
        /* Reinit mac contexts */
        if (!EVP_MD_CTX_copy_ex(ctx, ctx_init))
            goto err;
        if (!EVP_DigestSignUpdate(ctx, A1, A1_len))
            goto err;
        if (olen > chunk && !EVP_MD_CTX_copy_ex(ctx_tmp, ctx))
            goto err;
        if (seed1 && !EVP_DigestSignUpdate(ctx, seed1, seed1_len))
            goto err;
        if (seed2 && !EVP_DigestSignUpdate(ctx, seed2, seed2_len))
            goto err;
        if (seed3 && !EVP_DigestSignUpdate(ctx, seed3, seed3_len))
            goto err;
        if (seed4 && !EVP_DigestSignUpdate(ctx, seed4, seed4_len))
            goto err;
        if (seed5 && !EVP_DigestSignUpdate(ctx, seed5, seed5_len))
            goto err;

        if (olen > chunk) {
            if (!EVP_DigestSignFinal(ctx, out, &j))
                goto err;
            out += j;
            olen -= j;
            /* calc the next A1 value */
            if (!EVP_DigestSignFinal(ctx_tmp, A1, &A1_len))
                goto err;
        } else { /* last one */

            if (!EVP_DigestSignFinal(ctx, A1, &A1_len))
                goto err;
            memcpy(out, A1, olen);
            break;
        }
    }
    ret = 1;
err:
    EVP_PKEY_free(mac_key);
    EVP_MD_CTX_free(ctx);
    EVP_MD_CTX_free(ctx_tmp);
    EVP_MD_CTX_free(ctx_init);
    OPENSSL_cleanse(A1, sizeof(A1));
    return ret;
}

int OpenSslCryptoUtil::HmacSha256(const byte* key,
                                  size_t key_size,
                                  const byte* message,
                                  size_t message_size,
                                  mmtls::String& mac) {
    unsigned char* mac_buf = StringPreAlloc(mac, EVP_MAX_MD_SIZE);
    unsigned int mac_size = EVP_MAX_MD_SIZE;

    unsigned char* res = HMAC(EVP_sha256(), key, (int)key_size, message, message_size, mac_buf, &mac_size);

    if (!res) {
        return ERR_GEN_MAC_FAIL;
    }

    mac.resize(mac_size);

    return OK;
}

int OpenSslCryptoUtil::Ecdh(int nid,
                            const byte* public_material,
                            size_t public_material_size,
                            const byte* private_material,
                            size_t private_material_size,
                            mmtls::String& result) {
    int ret = -1;
    EC_KEY* pub_ec_key = NULL;
    EC_KEY* pri_ec_key = NULL;

    do {
        // load public key
        pub_ec_key = EC_KEY_new_by_curve_name(nid);
        if (!pub_ec_key) {
            MMTLSLOG_IMPT("ERR: %s public key EC_KEY_new_by_curve_name failed, nid %d", __func__, nid);
            ret = -1;
            break;
        }

        pub_ec_key = o2i_ECPublicKey(&pub_ec_key, &public_material, public_material_size);
        if (!pub_ec_key) {
            MMTLSLOG_IMPT("ERR: %s public key o2i_ECPublicKey failed, nid %d", __func__, nid);
            ret = -1;
            break;
        }

        // load private key
        pri_ec_key = EC_KEY_new_by_curve_name(nid);
        if (!pri_ec_key) {
            MMTLSLOG_IMPT("ERR: %s private key EC_KEY_new_by_curve_name failed, nid %d", __func__, nid);
            ret = -1;
            break;
        }

        pri_ec_key = d2i_ECPrivateKey(&pri_ec_key, &private_material, private_material_size);
        if (!pri_ec_key) {
            MMTLSLOG_IMPT("ERR: %s private key d2i_ECPrivateKey failed, nid %d", __func__, nid);
            ret = -1;
            break;
        }

        // compute ecdh key
        unsigned char* result_buf = StringPreAlloc(result, KDF_SHA256_LENGTH);

        int res =
            ECDH_compute_key(result_buf, KDF_SHA256_LENGTH, EC_KEY_get0_public_key(pub_ec_key), pri_ec_key, KdfSha256);
        if (res != KDF_SHA256_LENGTH) {
            MMTLSLOG_IMPT("ERR: %s ECDH_compute_key failed, nid %d res %d kdf len %d",
                          __func__,
                          nid,
                          res,
                          KDF_SHA256_LENGTH);
            ret = -1;
            break;
        }

        ret = 1;

    } while (0);

    // free memory
    if (pub_ec_key) {
        EC_KEY_free(pub_ec_key);
        pub_ec_key = NULL;
    }

    if (pri_ec_key) {
        EC_KEY_free(pri_ec_key);
        pri_ec_key = NULL;
    }

    if (ret != 1) {
        return ERR_COMPUTE_ECDH_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::EcdsaSign(const byte* private_key,
                                 size_t private_key_size,
                                 const byte* message,
                                 size_t message_size,
                                 mmtls::String& signature) {
    int ret = -1;
    BIO* bio = NULL;
    EC_KEY* ec_key = NULL;
    mmtls::String tmp((const char*)private_key, private_key_size);

    do {
        // load ecdsa key
        bio = BIO_new_mem_buf(&tmp[0], (int)tmp.size());
        if (!bio) {
            MMTLSLOG_IMPT("ERR: %s BIO_new_mem_buf failed, private key size %zu", __func__, private_key_size);
            ret = -1;
            break;
        }

        ec_key = PEM_read_bio_ECPrivateKey(bio, NULL, NULL, NULL);
        if (!ec_key) {
            MMTLSLOG_IMPT("ERR: %s PEM_read_bio_ECPrivateKey failed", __func__);
            ret = -1;
            break;
        }

        // digest
        unsigned char digest[SHA256_DIGEST_LENGTH];
        unsigned char* hash = SHA256(message, message_size, digest);
        if (!hash) {
            MMTLSLOG_IMPT("ERR: %s SHA256 failed, message size %zu", __func__, message_size);
            ret = -1;
            break;
        }

        // sign
        unsigned int sign_len = ECDSA_size(ec_key);
        unsigned char* sign_buf = StringPreAlloc(signature, sign_len);

        int res = ECDSA_sign(0, digest, SHA256_DIGEST_LENGTH, sign_buf, &sign_len, ec_key);
        if (res != 1) {
            MMTLSLOG_IMPT("ERR: %s ECDSA_sign failed, res %d", __func__, res);
            ret = -1;
            break;
        }

        signature.resize(sign_len);

        ret = 1;

    } while (0);

    if (bio) {
        BIO_free(bio);
        bio = NULL;
    }

    if (ec_key) {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    OPENSSL_cleanse(&tmp[0], tmp.size());

    if (ret != 1) {
        return ERR_ECDSA_SIGN_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::EcdsaVerify(const byte* public_key,
                                   size_t public_key_size,
                                   const byte* signature,
                                   size_t signature_size,
                                   const byte* message,
                                   size_t message_size) {
    int ret = -1;
    BIO* bio = NULL;
    EC_KEY* ec_key = NULL;
    mmtls::String tmp((const char*)public_key, public_key_size);

    do {
        // load ecdsa key
        bio = BIO_new_mem_buf(&tmp[0], (int)tmp.size());
        if (!bio) {
            MMTLSLOG_IMPT("ERR: %s BIO_new_mem_buf failed, public key size %zu", __func__, public_key_size);
            ret = -1;
            break;
        }

        ec_key = PEM_read_bio_EC_PUBKEY(bio, NULL, NULL, NULL);
        if (!ec_key) {
            MMTLSLOG_IMPT("ERR: %s PEM_read_bio_EC_PUBKEY failed", __func__);
            ret = -1;
            break;
        }

        // check signature size
        if (signature_size > (size_t)ECDSA_size(ec_key)) {
            MMTLSLOG_IMPT("ERR: %s invalid signature size, signature size %zu ecdsa size %zu",
                          __func__,
                          signature_size,
                          (size_t)ECDSA_size(ec_key));
            ret = -1;
            break;
        }

        // digest
        unsigned char digest[SHA256_DIGEST_LENGTH];
        unsigned char* hash = SHA256(message, message_size, digest);
        if (!hash) {
            MMTLSLOG_IMPT("ERR: %s SHA256 failed, message size %zu", __func__, message_size);
            ret = -1;
            break;
        }

        // verify
        int res = ECDSA_verify(0, digest, SHA256_DIGEST_LENGTH, signature, (int)signature_size, ec_key);
        if (res != 1) {
            MMTLSLOG_IMPT("ERR: %s ECDSA_verify failed, res %d", __func__, res);
            ret = -1;
            break;
        }

        ret = 1;

    } while (0);

    if (bio) {
        BIO_free(bio);
        bio = NULL;
    }

    if (ec_key) {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    OPENSSL_cleanse(&tmp[0], tmp.size());

    if (ret != 1) {
        return ERR_ECDSA_VEIRFY_FAIL;
    }

    return OK;
}

int OpenSslCryptoUtil::Sha256(const byte* message, size_t message_size, mmtls::String& result) {
    unsigned char* result_buf = StringPreAlloc(result, SHA256_DIGEST_LENGTH);
    unsigned char* res = SHA256(message, message_size, result_buf);
    if (!res) {
        return ERR_HASH_FAILD;
    }

    return OK;
}

const EVP_MD* OpenSslCryptoUtil::GetHashFunc() {
    const EVP_MD* evp_md = NULL;
    const mmtls::String& prf_algo = ciphersuite_.prf_algo();

    if (prf_algo == "SHA256") {
        evp_md = EVP_sha256();
    } else if (prf_algo == "SHA384") {
        evp_md = EVP_sha384();
    } else if (prf_algo == "SHA224") {
        evp_md = EVP_sha224();
    } else if (prf_algo == "SHA512") {
        evp_md = EVP_sha512();
    } else {
        evp_md = EVP_sha256();
    }

    return evp_md;
}

// Get default OpenSslCryptoUtil instance, just for being easy to invoke.
OpenSslCryptoUtil& OpenSslCryptoUtil::GetDefault() {
    static OpenSslCryptoUtil crypto_util;

    return crypto_util;
}

CryptoUtil::Hash* OpenSslCryptoUtil::CreateHash() {
    // OpenSslHash256 has better performance than OpenSslHash
    if (ciphersuite_.prf_algo() == "SHA256") {
        return new OpenSslHash256();
    } else {
        return new OpenSslHash(GetHashFunc());
    }
}

OpenSslCryptoUtil::OpenSslHash::OpenSslHash(const EVP_MD* md) : has_final_(false) {
    EVP_DigestInit(ctx_, md);
}

OpenSslCryptoUtil::OpenSslHash::OpenSslHash(const EVP_MD_CTX* ctx) : has_final_(false) {
    EVP_MD_CTX_copy(ctx_, ctx);
}

OpenSslCryptoUtil::OpenSslHash::~OpenSslHash() {
    if (!has_final_) {
        EVP_MD_CTX_reset(ctx_);
    }
}

CryptoUtil::Hash* OpenSslCryptoUtil::OpenSslHash::Clone() const {
    if (has_final_) {
        return NULL;
    }
    return new OpenSslHash(ctx_);
}

int OpenSslCryptoUtil::OpenSslHash::Update(const byte* message, size_t message_size) {
    if (message_size == 0) {
        return OK;
    }
    MMTLS_CHECK_TRUE(message, ERR_ILLEGAL_PARAM, "msg null");

    MMTLS_CHECK_NE(0,
                   EVP_DigestUpdate(ctx_, message, message_size),
                   ERR_HASH_FAILD,
                   "update fail.size %zu",
                   message_size);
    return OK;
}

int OpenSslCryptoUtil::OpenSslHash::Final(mmtls::String& result) {
    has_final_ = true;
    unsigned char res[EVP_MAX_MD_SIZE];
    unsigned int res_len = 0;
    int ret = EVP_DigestFinal(ctx_, res, &res_len);
    MMTLS_CHECK_NE(0, ret, ERR_HASH_FAILD, "digist fail.ret %d", ret);

    result.assign((const char*)res, res_len);
    return OK;
}

OpenSslCryptoUtil::OpenSslHash256::OpenSslHash256() : has_final_(false) {
    SHA256_Init(&ctx_);
}

OpenSslCryptoUtil::OpenSslHash256::OpenSslHash256(const SHA256_CTX& ctx) : has_final_(false) {
    memcpy(&ctx_, &ctx, sizeof(ctx));
}

OpenSslCryptoUtil::OpenSslHash256::~OpenSslHash256() {
}

CryptoUtil::Hash* OpenSslCryptoUtil::OpenSslHash256::Clone() const {
    if (has_final_) {
        return NULL;
    }
    return new OpenSslHash256(ctx_);
}

int OpenSslCryptoUtil::OpenSslHash256::Update(const byte* message, size_t message_size) {
    if (message_size == 0) {
        return OK;
    }
    MMTLS_CHECK_TRUE(message, ERR_ILLEGAL_PARAM, "msg null");

    MMTLS_CHECK_NE(0,
                   SHA256_Update(&ctx_, message, message_size),
                   ERR_HASH_FAILD,
                   "update fail.size %zu",
                   message_size);
    return OK;
}

int OpenSslCryptoUtil::OpenSslHash256::Final(mmtls::String& result) {
    has_final_ = true;
    unsigned char res[SHA256_DIGEST_LENGTH];
    int ret = SHA256_Final(res, &ctx_);
    MMTLS_CHECK_NE(0, ret, ERR_HASH_FAILD, "digist sha256 fail.ret %d", ret);

    result.assign((const char*)res, SHA256_DIGEST_LENGTH);
    return OK;
}

CryptoUtil::Digest* OpenSslCryptoUtil::CreateDigest() {
    return new OpenSslCryptoUtil::OpenSslMd5Digest();
}

OpenSslCryptoUtil::OpenSslMd5Digest::OpenSslMd5Digest() : has_final_(false) {
    MD5_Init(&ctx_);
}

int OpenSslCryptoUtil::OpenSslMd5Digest::Update(const byte* message, size_t message_size) {
    MMTLS_CHECK_FALSE(has_final_, ERR_UNEXPECT_CHECK_FAIL, "update should not finalize");

    int ret = MD5_Update(&ctx_, message, (unsigned long)message_size);

    MMTLS_CHECK_NE(0, ret, ERR_DIGEST_FAILD, "digest udpate fail.ret %d", ret);
    return OK;
}

int OpenSslCryptoUtil::OpenSslMd5Digest::Final(mmtls::String& result) {
    MMTLS_CHECK_FALSE(has_final_, ERR_UNEXPECT_CHECK_FAIL, "update should not finalize");

    unsigned char md[MD5_DIGEST_LENGTH];
    int ret = MD5_Final(md, &ctx_);
    has_final_ = true;
    MMTLS_CHECK_NE(0, ret, ERR_DIGEST_FAILD, "digest final fail.ret %d", ret);
    return OK;
}

}  // namespace mmtls
