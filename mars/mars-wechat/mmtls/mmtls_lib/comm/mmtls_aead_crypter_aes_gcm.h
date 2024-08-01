#pragma once
#include "mmtls_aead_crypter.h"
#include "mmtls_string.h"

namespace mmtls {

/**
 * AesGcmCrypter
 */
class AesGcmCrypter : public AeadCrypter {
 public:
    AesGcmCrypter(size_t iv_size, size_t key_size, size_t tag_size, bool is_encryptor);

    virtual ~AesGcmCrypter();

    // set key
    virtual int SetKey(const byte* key, size_t key_size);
    virtual int SetKeyBySwap(mmtls::String& key);

    // get key
    virtual const mmtls::String& GetKey();

    // set iv
    virtual int SetIv(const byte* iv, size_t iv_size);
    virtual int SetIvBySwap(mmtls::String& iv);

    // get iv
    virtual const mmtls::String& GetIv();

    virtual size_t GetCipherTextSize(size_t input_size) const;

    virtual int Crypt(const byte* nonce,
                      size_t nonce_size,
                      const byte* aad,
                      size_t aad_size,
                      const byte* input,
                      size_t input_data,
                      mmtls::String& output);

 private:
    const size_t iv_size_;
    const size_t key_size_;

    size_t tag_size_;

    bool is_encryptor_;

    mmtls::String iv_;
    mmtls::String key_;

 private:
    // Disallow copy and assign.
    AesGcmCrypter(const AesGcmCrypter& crypter);
    AesGcmCrypter& operator=(const AesGcmCrypter& crypter);
};

};  // namespace mmtls
