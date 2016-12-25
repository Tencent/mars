/*
 * GenRsaKeyPair.c
 *
 *  Created on: 2016年3月21日
 *      Author: wutianqiang
 */
#include <string.h>
#include "../../export_include/gen_rsa_key_pair.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "../../../comm/xlogger/xlogger.h"
GenRsaKeyResult generate_rsa_key_pair_2048(char* _pem_public_key_buf, const size_t _public_key_buf_len,
										   char* _pem_private_key_buf, const size_t _private_key_buf_len) {
    
    GenRsaKeyResult ret = kOK;
	/* 产生RSA密钥 */
	RSA *rsa = RSA_generate_key(2048, 65537, NULL, NULL); //TODO

	xdebug2(TSF"BIGNUM: %_\n", BN_bn2hex(rsa->n));

	/* 提取私钥 */
	BIO* bio_private = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bio_private, rsa, NULL, NULL, 0, NULL, NULL);
    int private_key_len = BIO_pending(bio_private);
    xdebug2(TSF"private_key_len=%_", private_key_len);
    char* pem_private_key = (char*)calloc(private_key_len+1, 1);
	BIO_read(bio_private, pem_private_key, private_key_len);
	//xdebug2(TSF"pem_private_key=%_, pem key len=%_",pem_private_key, strlen(pem_private_key));

	/* 提取公钥 */
	unsigned char *n_b = (unsigned char *) calloc(RSA_size(rsa),
			sizeof(unsigned char));
	unsigned char *e_b = (unsigned char *) calloc(RSA_size(rsa),
			sizeof(unsigned char));

	int n_size = BN_bn2bin(rsa->n, n_b);
	int b_size = BN_bn2bin(rsa->e, e_b);

	RSA *pubrsa = RSA_new();
	pubrsa->n = BN_bin2bn(n_b, n_size, NULL);
	pubrsa->e = BN_bin2bn(e_b, b_size, NULL);


	BIO* bio_public = BIO_new(BIO_s_mem());
	PEM_write_bio_RSA_PUBKEY(bio_public, pubrsa);
    int public_key_len = BIO_pending(bio_public);
    xdebug2(TSF"public_key_len=%_", public_key_len);
    char *pem_public_key = (char*)calloc(public_key_len+1, 1);
	BIO_read(bio_public, pem_public_key, public_key_len);
	xdebug2(TSF"pem_public_key=%_, pem key len=%_", pem_public_key, strlen(pem_public_key));

    if (_public_key_buf_len<strlen(pem_public_key)
        || _private_key_buf_len<strlen((const char*)&pem_private_key)) {
        xerror2(TSF"buffer length is not enough. _public_key_buf_len=%_, pem_pub_key_len=%_; \
                _private_key_buf_len=%_, pem_pri_key_len=%_", _public_key_buf_len, strlen(pem_public_key),_private_key_buf_len, strlen(pem_private_key));
        ret = kBufNotEnough;
        goto err;
    }
    memcpy(_pem_private_key_buf, pem_private_key, strlen(pem_private_key));
    memcpy(_pem_public_key_buf, pem_public_key, strlen(pem_public_key));
	
err:
    if (n_b!=NULL) {
        free(n_b);
    }
    if (e_b!=NULL) {
        free(e_b);
    }
    if (pem_public_key!=NULL) {
        free(pem_public_key);
    }
    if (pem_private_key!=NULL) {
        free(pem_private_key);
    }
    BIO_free(bio_private);
	BIO_free(bio_public);
	RSA_free(rsa);
	RSA_free(pubrsa);
    return ret;

}

