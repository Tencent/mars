#include "pay_openssl_crypto_util.h"

#include <string>
#include "openssl/bio.h"
#include "openssl/evp.h"
#include "openssl/pem.h"
#include "openssl/hmac.h"
#include "openssl/ecdsa.h"
#include "openssl/sha.h"
#include "openssl/rand.h"

#include "../../../comm/xlogger/xlogger.h"



namespace mmpaycertlib{

OpenSslCryptoUtil::OpenSslCryptoUtil()
{
}

OpenSslCryptoUtil::~OpenSslCryptoUtil()
{
}

OpenSslCryptoUtil& OpenSslCryptoUtil::GetDefault()
{
    static OpenSslCryptoUtil util;
    return util;
}

#define KDF_SHA256_LENGTH	SHA256_DIGEST_LENGTH

static inline unsigned char *StringPreAlloc(std::string &str, size_t size)
{
	str.clear();
	str.resize(size);

	return (unsigned char *)&str[0];
}

static void *KdfSha256(const void *in, size_t in_len, void *out, size_t *out_len)
{
	if ((!out_len)  || (!in) || (!in_len)  || *out_len < KDF_SHA256_LENGTH)
		return NULL;
	else
		*out_len = KDF_SHA256_LENGTH;

	return SHA256((const unsigned char *)in, in_len, (unsigned char *)out);
}

int OpenSslCryptoUtil::GenEcdhKeyPair(std::string& public_material, std::string& private_material)
{
    int nid = NID_X9_62_prime256v1;
	int ret = -1;
	EC_KEY *ec_key = NULL;
	unsigned char *pub_key_buf = NULL;
	int pub_key_size = 0;
	unsigned char *pri_key_buf = NULL;
	int pri_key_size = 0;

	do {
		// create ec key by nid
		ec_key = EC_KEY_new_by_curve_name(nid);
		if (!ec_key) {
			xerror2(TSF"ERR: EC_KEY_new_by_curve_name failed, nid %_", nid);
			ret = -1;
			break;
		}

		EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);

		// generate ec key pair
		ret = EC_KEY_generate_key(ec_key);
		if (ret != 1) {
			xerror2(TSF"ERR:EC_KEY_generate_key failed, ret %_", ret);
			ret = -1;
			break;
		}

		// get public key from ec key pair
		pub_key_size = i2o_ECPublicKey(ec_key, &pub_key_buf);
		if (pub_key_size == 0 || !pub_key_buf) {
			xerror2(TSF"ERR: i2o_ECPublicKey faild, ret %_", ret);
			ret = -1;
			break;
		}

		// get private key from ec key pair
		pri_key_size = i2d_ECPrivateKey(ec_key, &pri_key_buf);
		if (pri_key_size == 0 || !pri_key_buf) {
			xerror2(TSF"ERR:i2d_ECPrivateKey failed, ret %_", ret);
			ret = -1;
			break;
		}

		// set key_pair
        public_material.assign((const char*)pub_key_buf, pub_key_size);
        private_material.assign((const char*)pri_key_buf, pri_key_size);

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
		return -1;
	}
	
	return 0;
}

int OpenSslCryptoUtil::GenEcdsaKeyPair(std::string& public_key, std::string& private_key)
{
    int nid = NID_X9_62_prime256v1;
    int ret = -1;
    EC_KEY* ec_key = NULL;
    BIO* bio = NULL;

	do {
        ec_key = EC_KEY_new_by_curve_name(nid);
        if (!ec_key) {
        	xerror2(TSF"ERR: EC_KEY_new_by_curve_name failed, nid %_", nid);
            ret = -1;
            break;
        }

		EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);

        ret = EC_KEY_generate_key(ec_key);
		if (ret != 1) {
			xerror2(TSF"ERR: EC_KEY_generate_key failed, ret %_", ret);
			ret = -1;
			break;
		}

        ret = EC_KEY_check_key(ec_key);
        if (ret != 1) {
        	xerror2(TSF"ERR: EC_KEY_check_key fail, ret %_", ret);
            ret = -1;
            break;
        }

        bio = BIO_new(BIO_s_mem());
        ret = PEM_write_bio_EC_PUBKEY(bio, ec_key);
        if (ret != 1 || BIO_flush(bio) != 1) {
        	xerror2(TSF"ERR: PEM_write_bio_EC_PUBKEY fail, ret %_", ret);
            ret = -1;
            break;
        }
        char * ptr=NULL;
        long size = BIO_get_mem_data(bio, &ptr);
        public_key.assign(ptr, size);
        BIO_free(bio);

        bio = BIO_new(BIO_s_mem());
        ret = PEM_write_bio_ECPrivateKey(bio, ec_key, NULL, NULL, 0, NULL, NULL);
        if (ret != 1 || BIO_flush(bio) != 1) {
        	xerror2(TSF"ERR: PEM_write_bio_ECPrivateKey fail, ret %_", ret);
            ret = -1;
            break;
        }
        ptr = NULL;
        size = BIO_get_mem_data(bio,&ptr);
        private_key.assign(ptr, size);

        ret = 1;
	} while (0);

    // free memory
    if (NULL != bio) {
        BIO_free(bio);
        bio = NULL;
    }

    if (NULL != ec_key) {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    if (ret != 1) {
        return -1;
    }

    return 0;
}

int OpenSslCryptoUtil::Ecdh(const std::string& public_material,
    	 			        const std::string& private_material,
			 		        std::string& result)
{
    return this->Ecdh(NID_X9_62_prime256v1, (const unsigned char*)public_material.c_str(), public_material.size(),
                                            (const unsigned char*)private_material.c_str(), private_material.size(),
                                            result);
}

int OpenSslCryptoUtil::Ecdh(int nid,
							const unsigned char* public_material, size_t public_material_size,
							const unsigned char* private_material, size_t private_material_size,
							std::string& result)
{
	int ret = -1;
	EC_KEY *pub_ec_key = NULL;
	EC_KEY *pri_ec_key = NULL;

	do {
		// load public key
		pub_ec_key = EC_KEY_new_by_curve_name(nid);
		if (!pub_ec_key) {
			xerror2(TSF"ERR: public key EC_KEY_new_by_curve_name failed, nid %_", nid);
			ret = -1;
			break;
		}

		pub_ec_key = o2i_ECPublicKey(&pub_ec_key, &public_material, public_material_size);
		if (!pub_ec_key) {
			xerror2(TSF"ERR:public key o2i_ECPublicKey failed, nid %_", nid);
			ret = -1;
			break;
		}

		// load private key
		pri_ec_key = EC_KEY_new_by_curve_name(nid);
		if (!pri_ec_key) {
			xerror2(TSF"ERR: private key EC_KEY_new_by_curve_name failed, nid %_", nid);
			ret = -1;
			break;
		}

		pri_ec_key = d2i_ECPrivateKey(&pri_ec_key, &private_material, private_material_size);
		if (!pri_ec_key) {
			xerror2(TSF"ERR: private key d2i_ECPrivateKey failed, nid %_", nid);
			ret = -1;
			break;
		}

		// compute ecdh key
		unsigned char *result_buf = StringPreAlloc(result, KDF_SHA256_LENGTH);

		int res = ECDH_compute_key(result_buf, KDF_SHA256_LENGTH, EC_KEY_get0_public_key(pub_ec_key), pri_ec_key, KdfSha256);
		if (res != KDF_SHA256_LENGTH) {
			xerror2(TSF"ERR:ECDH_compute_key failed, nid %_ res %_ kdf len %_", nid, res, KDF_SHA256_LENGTH);
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
		return -1;
	}

	return 0;
}

int OpenSslCryptoUtil::EcdsaSign(const std::string& private_key,
                                 const std::string& message,
                                 std::string& signature)
{
    return this->EcdsaSign((const unsigned char*)private_key.c_str(), private_key.size(), 
                           (const unsigned char*)message.c_str(), message.size(),
                           signature);
}

int OpenSslCryptoUtil::EcdsaSign(const unsigned char* private_key, size_t private_key_size,
								 const unsigned char* message, size_t message_size,
								 std::string& signature)
{
	int ret = -1;
	BIO *bio = NULL;
	EC_KEY *ec_key = NULL;
	std::string tmp((const char *)private_key, private_key_size);
	
	do {
		// load ecdsa key
		bio = BIO_new_mem_buf(&tmp[0], tmp.size());
		if (!bio) {
			xerror2(TSF"ERR:BIO_new_mem_buf failed, private key size %_", private_key_size);
			ret = -1;
			break;
		}

		ec_key = PEM_read_bio_ECPrivateKey(bio, NULL, NULL, NULL);
		if (!ec_key) {
			xerror2(TSF"ERR: PEM_read_bio_ECPrivateKey failed");
			ret = -1;
			break;
		}

		// digest
		unsigned char digest[SHA256_DIGEST_LENGTH];
		unsigned char *hash = SHA256(message, message_size, digest);
		if (!hash) {
			xerror2(TSF"ERR: SHA256 failed, message size %_", message_size);
			ret = -1;
			break;
		}

		// sign
		unsigned int sign_len = ECDSA_size(ec_key);
		unsigned char *sign_buf = StringPreAlloc(signature, sign_len);

		int res = ECDSA_sign(0, digest, SHA256_DIGEST_LENGTH, sign_buf, &sign_len, ec_key);
		if (res != 1) {
			xerror2(TSF"ERR: ECDSA_sign failed, res %_", res);
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
		return -1;
	}

	return 0;
}

int OpenSslCryptoUtil::EcdsaVerify(const std::string& public_key,
                                   const std::string& signature,
                                   const std::string& message)

{
    return this->EcdsaVerify((const unsigned char*)public_key.c_str(), public_key.size(),
                             (const unsigned char*)signature.c_str(), signature.size(),
                             (const unsigned char*)message.c_str(), message.size());
}

int OpenSslCryptoUtil::EcdsaVerify(const unsigned char* public_key, size_t public_key_size,
								   const unsigned char* signature, size_t signature_size,
								   const unsigned char* message, size_t message_size)
{
	int ret = -1;
	BIO *bio = NULL;
	EC_KEY *ec_key = NULL;
	std::string tmp((const char *)public_key, public_key_size);
	
	do {
		// load ecdsa key
		bio = BIO_new_mem_buf(&tmp[0], tmp.size());
		if (!bio) {
			xerror2(TSF"ERR: BIO_new_mem_buf failed, public key size %_", public_key_size);
			ret = -1;
			break;
		}

		ec_key = PEM_read_bio_EC_PUBKEY(bio, NULL, NULL, NULL);
		if (!ec_key) {
			xerror2(TSF"ERR: PEM_read_bio_EC_PUBKEY failed");
			ret = -1;
			break;
		}

		// check signature size
		if (signature_size > (size_t)ECDSA_size(ec_key)) {
			xerror2(TSF"ERR:  invalid signature size, signature size %_ ecdsa size %_", signature_size, (size_t)ECDSA_size(ec_key));
			ret = -1;
			break;
		}

		// digest
		unsigned char digest[SHA256_DIGEST_LENGTH];
		unsigned char *hash = SHA256(message, message_size, digest);
		if (!hash) {
			xerror2(TSF"ERR: SHA256 failed, message size %_", message_size);
			ret = -1;
			break;
		}

		// verify
		int res = ECDSA_verify(0, digest, SHA256_DIGEST_LENGTH, signature, signature_size, ec_key);
		if (res != 1) {
			xerror2(TSF"ERR: ECDSA_verify failed, res %_", res);
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
		return -1;
	}

	return 0;
}

}


