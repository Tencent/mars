/*
 * rsa_pss_sha256.cpp
 *
 *  Created on: 2016年3月17日
 *      Author: wutianqiang
 */
#include "../../export_include/rsa_pss_sha256.h"
#include "../../../comm/xlogger/xlogger.h"
#include "openssl/evp.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"

//_is_private_key == true: _pem_key is rsa private key
//_is_private_key == false: _pem_key is rsa public key
static EVP_PKEY* change_pem_key_to_evp_key(const char* _pem_key,
		size_t _key_len, bool _is_private_key) {

	BIO* bio = BIO_new(BIO_s_mem());
	int len = BIO_write(bio, _pem_key, _key_len);
	if (static_cast<size_t>(len) != _key_len) {
		xerror2(TSF"write pem key to BIO fail.");
		BIO_free(bio);
		return NULL;
	}
	EVP_PKEY* evp_key = NULL;
	if(_is_private_key) {
		evp_key = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
	} else {
		evp_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
	}
	if (evp_key == NULL) {
		xerror2(TSF"try to read structed [%_] key fail.", _is_private_key?"rsa_private": "rsa_public");
		BIO_free(bio);
		return NULL;
	}

	BIO_free(bio);
	return evp_key;
}

bool rsa_pss_sha256_sign(const char* _message, const unsigned long _msg_len,
		const char* _pem_private_key, size_t _private_key_len,
		unsigned char* _signature, size_t& _sig_buf_len) {

	xdebug_function();
	//calculate message hash using sha256.
	unsigned char md_value[EVP_MAX_MD_SIZE] = { 0 };
	unsigned int md_len = 0;

	EVP_MD_CTX* md_ctx = EVP_MD_CTX_create();

	int ret = EVP_DigestInit_ex(md_ctx, EVP_sha256(), NULL);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestInit_ex [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	ret = EVP_DigestUpdate(md_ctx, _message, _msg_len);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestUpdate [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	ret = EVP_DigestFinal_ex(md_ctx, md_value, &md_len);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestFinal_ex [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	EVP_MD_CTX_destroy(md_ctx);

	//for debugging
	xdebug2(TSF"debug: rsassa-pss with sha256 and salt length 20 sign: md_hex [%_]",
			xdump(md_value, md_len));

	// sign  using padding pss with salt length 20.
	EVP_PKEY* evp_private_key = change_pem_key_to_evp_key(_pem_private_key,
			_private_key_len, true); //new evp_private_key
	if (NULL == evp_private_key) {
		xerror2(TSF"change_pem_key_to_evp_key failed.");
		return false;
	}
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_private_key, NULL);
	if (NULL == ctx) {
		xerror2(TSF"err: EVP_PKEY_CTX_new fail");
		EVP_PKEY_free(evp_private_key);
		return false;
	}
	EVP_PKEY_free(evp_private_key); //free evp_private_key

	ret = EVP_PKEY_sign_init(ctx);
	if (0 >= ret) {
		xerror2(TSF"err: sign fail: EVP_PKEY_sign_init [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());
	if (0 >= ret) {
		xerror2(TSF"err: sign fail: EVP_PKEY_CTX_set_signature_md [ret %_]",
				ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING);
	if (0 >= ret) {
		xerror2(TSF"err: sign fail: EVP_PKEY_CTX_set_rsa_padding [ret %_]",
				ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, 20);
	if (0 >= ret) {
		xerror2(TSF"err: sign fail: EVP_PKEY_CTX_set_rsa_pss_saltlen [ret %_]",
				ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	/* Determine buffer length */
	size_t sig_len = _sig_buf_len;
	if (EVP_PKEY_sign(ctx, NULL, &sig_len, md_value, md_len) <= 0) {
		xerror2(TSF"err: get sig_len failed.");
		EVP_PKEY_CTX_free(ctx);
		return false;

	} else {
		xdebug2(TSF"need sig_len=%_, _sig_buf_len=%_", sig_len, _sig_buf_len);
		if (sig_len > _sig_buf_len) {
			xerror2(TSF"err: _sig_buf_len is too small.");
			EVP_PKEY_CTX_free(ctx);
			return false;
		}
	}

	//sign
	ret = EVP_PKEY_sign(ctx, _signature, &sig_len, md_value, md_len);
	if (0 >= ret) {
		xerror2(TSF"err: sign fail: EVP_PKEY_sign [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	_sig_buf_len = sig_len; //return the real signature length
	EVP_PKEY_CTX_free(ctx);

	return true;
}

bool rsa_pss_sha256_verify(const char* _message, const unsigned long _msg_len,
		const char* _pem_public_key, size_t _public_key_len,
		unsigned char* _signature, size_t _sig_len) {

	xdebug_function();
	//step1:calculate message hash using sha256.
	unsigned char md_value[EVP_MAX_MD_SIZE] = { 0 };
	unsigned int md_len = 0;

	EVP_MD_CTX* md_ctx = EVP_MD_CTX_create();

	int ret = EVP_DigestInit_ex(md_ctx, EVP_sha256(), NULL);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestInit_ex [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	ret = EVP_DigestUpdate(md_ctx, _message, _msg_len);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestUpdate [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	ret = EVP_DigestFinal_ex(md_ctx, md_value, &md_len);
	if (0 >= ret) {
		xerror2(TSF"err: sha256 fail: EVP_DigestFinal_ex [ret %_]", ret);
		EVP_MD_CTX_destroy(md_ctx);
		return false;
	}
	EVP_MD_CTX_destroy(md_ctx);

	//for debugging
	xdebug2(TSF"debug: rsassa-pss with sha256 and salt length 20 sign: md_hex [%_]",
			xdump(md_value, md_len));

	// Verify signature using padding pss with salt length 20.
	EVP_PKEY* evp_public_key = change_pem_key_to_evp_key(_pem_public_key,
			_public_key_len, false); //new evp_public_key
	if (NULL == evp_public_key) {
		xerror2(TSF"change_pem_key_to_evp_key failed.");
		return false;
	}
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_public_key, NULL);
	if (NULL == ctx) {
		xerror2(TSF"err: verify signature fail: EVP_PKEY_CTX_new fail");
		EVP_PKEY_free(evp_public_key);
		return false;
	}
	EVP_PKEY_free(evp_public_key); //free evp_public_key

	ret = EVP_PKEY_verify_init(ctx);
	if (0 >= ret) {
		xerror2(TSF"err:EVP_PKEY_verify_init [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());
	if (0 >= ret) {
		xerror2(TSF"err: verify signature fail: EVP_PKEY_CTX_set_signature_md [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PSS_PADDING);
	if (0 >= ret) {
		xerror2(TSF"err: verify signature fail: EVP_PKEY_CTX_set_rsa_padding [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_CTX_set_rsa_pss_saltlen(ctx, 20);
	if (0 >= ret) {
		xerror2(TSF"err: verify signature fail: EVP_PKEY_CTX_set_rsa_pss_saltlen [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ret = EVP_PKEY_verify(ctx, _signature, _sig_len, md_value, md_len);
	if (0 >= ret) {
		xerror2(TSF"err: verify signature fail: EVP_PKEY_verify [ret %_]", ret);
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	EVP_PKEY_CTX_free(ctx);

	return true;
}
