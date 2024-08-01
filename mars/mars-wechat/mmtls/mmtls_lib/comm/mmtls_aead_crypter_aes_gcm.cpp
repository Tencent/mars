#include "mmtls_aead_crypter_aes_gcm.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>

namespace mmtls {

static int InitEvpCipherCtx(EVP_CIPHER_CTX* ctx, size_t key_size) {
    int ret = -1;
    MMTLS_CHECK_TRUE(ctx, ret, "ctx null");

    // initialize evp context
    EVP_CIPHER_CTX_reset(ctx);

    // set operation mode with key size
    switch (key_size) {
        case 16:
            ret = EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL);
            break;

        case 24:
            ret = EVP_EncryptInit_ex(ctx, EVP_aes_192_gcm(), NULL, NULL, NULL);
            break;

        case 32:
            ret = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
            break;

        default:
            break;
    }

    return ret;
}

static int AesGcmEncrypt(const byte* nonce,
                         size_t nonce_size,
                         const byte* key,
                         size_t key_size,
                         const byte* aad,
                         size_t aad_size,
                         const byte* input,
                         size_t input_size,
                         mmtls::String& output,
                         byte* tag,
                         size_t tag_size) {
    int ret = ERR_ILLEGAL_PARAM;
    if ((!nonce) || (!key) || (!input) || (!tag) || nonce_size == 0 || 0 == key_size || 0 == input_size
        || tag_size == 0) {
        return ret;
    }
    int out_len = 0;
    int res_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    do {
        // initialize context
        ret = InitEvpCipherCtx(ctx, key_size);
        if (ret != 1)
            break;

        // set iv size
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)nonce_size, NULL);
        if (ret != 1)
            break;

        // set key and iv
        ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce);
        if (ret != 1)
            break;

        // set aad
        if (aad && aad_size > 0) {
            ret = EVP_EncryptUpdate(ctx, NULL, &out_len, aad, (int)aad_size);
            if (ret != 1)
                break;
        }

        // encrypt
        size_t original_len = output.size();
        const EVP_CIPHER* cipher = EVP_CIPHER_CTX_cipher(ctx);
        size_t reserve_len = original_len + input_size + EVP_CIPHER_block_size(cipher);
        output.resize(reserve_len);
        unsigned char* out_buf = (unsigned char*)(output.data() + original_len);
        ret = EVP_EncryptUpdate(ctx, out_buf, &out_len, input, (int)input_size);
        if (ret != 1)
            break;

        res_len = out_len;

        ret = EVP_EncryptFinal_ex(ctx, out_buf + res_len, &out_len);
        if (ret != 1)
            break;

        res_len += out_len;
        output.resize(original_len + res_len);

        // get tag
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)tag_size, tag);
        if (ret != 1)
            break;

    } while (0);

    EVP_CIPHER_CTX_free(ctx);

    if (ret != 1) {
        return ERR_ENCRYPT_FAIL;
    }

    return OK;
}

static int AesGcmDecrypt(const byte* nonce,
                         size_t nonce_size,
                         const byte* key,
                         size_t key_size,
                         const byte* aad,
                         size_t aad_size,
                         const byte* input,
                         size_t input_size,
                         const byte* tag,
                         size_t tag_size,
                         mmtls::String& output) {
    int ret = -1;
    if ((!nonce) || (!key) || (!input) || (!tag) || nonce_size == 0 || 0 == key_size || 0 == input_size
        || tag_size == 0) {
        return ret;
    }
    int out_len = 0;
    int res_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    do {
        // initialize context
        ret = InitEvpCipherCtx(ctx, key_size);
        if (ret != 1)
            break;

        // set iv size
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)nonce_size, NULL);
        if (ret != 1)
            break;

        // set key and iv
        ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce);
        if (ret != 1)
            break;

        // set aad
        if (aad && aad_size > 0) {
            ret = EVP_DecryptUpdate(ctx, NULL, &out_len, aad, (int)aad_size);
            if (ret != 1)
                break;
        }

        // decrypt
        size_t original_len = output.size();
        const EVP_CIPHER* cipher = EVP_CIPHER_CTX_cipher(ctx);
        size_t reserve_len = original_len + input_size + EVP_CIPHER_block_size(cipher);
        output.resize(reserve_len);
        unsigned char* output_buf = (unsigned char*)(output.data() + original_len);
        EVP_DecryptUpdate(ctx, output_buf, &out_len, input, (int)input_size);
        res_len = out_len;

        // verify tag
        ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)tag_size, (void*)tag);
        if (ret == 1) {
            ret = EVP_DecryptFinal_ex(ctx, output_buf + res_len, &out_len);
            if (ret == 1) {
                res_len += out_len;
                output.resize(original_len + res_len);
            }
        }

    } while (0);

    EVP_CIPHER_CTX_free(ctx);

    if (ret != 1) {
        return ERR_DECRYPT_FAIL;
    }

    return OK;
}

/**
 * AesGcmCrypter
 */
AesGcmCrypter::AesGcmCrypter(size_t iv_size, size_t key_size, size_t tag_size, bool is_encryptor)
: iv_size_(iv_size), key_size_(key_size), tag_size_(tag_size), is_encryptor_(is_encryptor) {
    if (tag_size > kMaxTagSize) {
        tag_size_ = kMaxTagSize;
    }
}

AesGcmCrypter::~AesGcmCrypter() {
    key_.safe_clear();
    iv_.safe_clear();
}

int AesGcmCrypter::SetKey(const byte* key, size_t key_size) {
    if (key == NULL) {
        return ERR_ILLEGAL_PARAM;
    }

    if (key_size != 16 && key_size != 24 && key_size != 32) {
        return ERR_ILLEGAL_PARAM;
    }

    if (key_size != key_size_) {
        return ERR_ILLEGAL_PARAM;
    }

    key_.assign((const char*)key, key_size);

    return OK;
}

int AesGcmCrypter::SetKeyBySwap(mmtls::String& key) {
    if (key.size() != 16 && key.size() != 24 && key.size() != 32) {
        return ERR_ILLEGAL_PARAM;
    }

    if (key.size() != key_size_) {
        return ERR_ILLEGAL_PARAM;
    }

    key_.swap(key);
    return OK;
}

const mmtls::String& AesGcmCrypter::GetKey() {
    return key_;
}

int AesGcmCrypter::SetIv(const byte* iv, size_t iv_size) {
    if (iv == NULL) {
        return ERR_ILLEGAL_PARAM;
    }

    if (iv_size > kMaxIvSize || iv_size < kMinIvSize) {
        return ERR_ILLEGAL_PARAM;
    }

    if (iv_size != iv_size_) {
        return ERR_ILLEGAL_PARAM;
    }

    iv_.assign((const char*)iv, iv_size);

    return OK;
}

int AesGcmCrypter::SetIvBySwap(mmtls::String& iv) {
    if (iv.size() > kMaxIvSize || iv.size() < kMinIvSize) {
        return ERR_ILLEGAL_PARAM;
    }

    if (iv.size() != iv_size_) {
        return ERR_ILLEGAL_PARAM;
    }

    iv_.swap(iv);

    return OK;
}

const mmtls::String& AesGcmCrypter::GetIv() {
    return iv_;
}

size_t AesGcmCrypter::GetCipherTextSize(size_t input_size) const {
    return input_size + tag_size_;
}

int AesGcmCrypter::Crypt(const byte* nonce,
                         size_t nonce_size,
                         const byte* aad,
                         size_t aad_size,
                         const byte* input,
                         size_t input_size,
                         mmtls::String& output) {
    int ret = 0;

    if (!nonce || !input || nonce_size == 0 || input_size == 0) {
        MMTLSLOG_ERR("ERR: %s invalid param, nonce %p input %p nonce size %zu input size %zu",
                     __func__,
                     nonce,
                     input,
                     nonce_size,
                     input_size);

        return ERR_ILLEGAL_PARAM;
    }

    if (is_encryptor_) {
        byte tag[kMaxTagSize];

        // aes gcm aead encrypt
        ret = AesGcmEncrypt(nonce,
                            nonce_size,
                            (const byte*)key_.data(),
                            key_size_,
                            aad,
                            aad_size,
                            input,
                            input_size,
                            output,
                            tag,
                            tag_size_);
        MMTLS_CHECK_EQ(ret, OK, ret, "encrypt failed");

        // append tag, layout: | ciphertext | tag(fixed size) |
        output.append((const char*)tag, tag_size_);

        return OK;
    } else {
        // check input
        if (input_size <= tag_size_) {
            MMTLSLOG_ERR("ERR: %s invalid input, input size %zu tag size %zu", __func__, input_size, tag_size_);

            return ERR_ILLEGAL_PARAM;
        }

        // aes gcm aead decrypt, layout: | ciphertext | tag(fixed size) |
        ret = AesGcmDecrypt(nonce,
                            nonce_size,
                            (const byte*)key_.data(),
                            key_size_,
                            aad,
                            aad_size,
                            input,
                            input_size - tag_size_,
                            input + input_size - tag_size_,
                            tag_size_,
                            output);
        MMTLS_CHECK_EQ(ret, OK, ret, "decrypt failed");

        return OK;
    }
}

}  // namespace mmtls
