/*
 * rsa_encrypt.c
 *
 *  Created on: 2013-7-16
 *      Author: zhouzhijie
        modify: wutianqiang
 */

#include <string.h>
#include <stdlib.h>
#include "rsa_crypt.h"
#include "rsa_private_decrypt.h"
#include <openssl/rsa.h>

int rsa_public_encrypt(const unsigned char* pInput, unsigned int uiInputLen
		, unsigned char** ppOutput, unsigned int* uiOutputLen
		, const char* pPublicKeyN, const char* pPublicKeyE)
{
	RSA* encrypt_rsa;
	int finalLen = 0;
	int i = 0;
	int ret;
	int rsa_len;
    if(pInput == NULL || uiOutputLen == NULL
       || pPublicKeyN == NULL || pPublicKeyE == NULL
       || uiInputLen == 0 || ppOutput == NULL)
        return CRYPT_ERR_INVALID_PARAM;
    
    // assume the byte strings are sent over the network
    encrypt_rsa = RSA_new();
    if(encrypt_rsa == NULL) {
    	return CRYPT_ERR_NO_MEMORY;
    }

    ret = CRYPT_OK;
    if(BN_hex2bn(&encrypt_rsa->n, pPublicKeyN) == 0)
    {
        ret = CRYPT_ERR_INVALID_KEY_N;
        goto END;
    }
    if(BN_hex2bn(&encrypt_rsa->e, pPublicKeyE) == 0)
    {
        ret = CRYPT_ERR_INVALID_KEY_E;
        goto END;
    }

    // alloc encrypt_string
    
    rsa_len = RSA_size(encrypt_rsa);
    if((int)uiInputLen >= rsa_len - 12) {
    	int blockCnt = (uiInputLen / (rsa_len - 12)) + (((uiInputLen % (rsa_len - 12)) == 0) ? 0 : 1);
    	finalLen = blockCnt * rsa_len;
    	*ppOutput = (unsigned char*)calloc(finalLen, sizeof(unsigned char));
    	if(*ppOutput == NULL) {
    		ret = CRYPT_ERR_NO_MEMORY;
    		goto END;
    	}

    	
    	for(; i < blockCnt; ++i) {
    		int blockSize = rsa_len - 12;
    		if(i == blockCnt - 1) blockSize = uiInputLen - i * blockSize;

    		if(RSA_public_encrypt(blockSize, (pInput + i * (rsa_len - 12)), (*ppOutput + i * rsa_len), encrypt_rsa, RSA_PKCS1_PADDING) < 0) {
    			ret = CRYPT_ERR_ENCRYPT_WITH_RSA_PUBKEY;
    			goto END;
    		}
    	}
    	*uiOutputLen = finalLen;
    } else {
        *ppOutput = (unsigned char*)calloc(rsa_len, sizeof(unsigned char));
        if (*ppOutput == NULL) {
            ret = CRYPT_ERR_NO_MEMORY;
            goto END;
        }

        // encrypt (return the size of the encrypted data)
        // note that if RSA_PKCS1_OAEP_PADDING is used,
        // flen must be < RSA_size - 41
        if(RSA_public_encrypt(uiInputLen,
                pInput, *ppOutput, encrypt_rsa, RSA_PKCS1_PADDING) < 0) {
            ret = CRYPT_ERR_ENCRYPT_WITH_RSA_PUBKEY;
            goto END;
        }
        *uiOutputLen = rsa_len;
    }

END:
	if(CRYPT_OK != ret) {
		if(*ppOutput != NULL) {
			free(*ppOutput);
			ppOutput = NULL;
		}
	}
	RSA_free(encrypt_rsa);
	return ret;
}

int rsa_public_decrypt(const unsigned char* pInput, unsigned int uiInputLen
		, unsigned char** ppOutput, unsigned int* uiOutputLen
		, const char* pPublicKeyN, const char* pPublicKeyE)
{
	RSA* decrypt_rsa;
	int i = 0;
	int ret = 0;
	int ret_size = 0;
	int iKeySize = 0;
	int iPlainSize = 0;
	unsigned char *pcPlainBuf = NULL;
    if(pInput == NULL || uiOutputLen == NULL
       || pPublicKeyN == NULL || pPublicKeyE == NULL
       || uiInputLen == 0 || ppOutput == NULL)
        return CRYPT_ERR_INVALID_PARAM;

    // assume the byte strings are sent over the network
    decrypt_rsa = RSA_new();
    if(decrypt_rsa == NULL) {
    	return CRYPT_ERR_NO_MEMORY;
    }

    ret = CRYPT_OK;
    if(BN_hex2bn(&decrypt_rsa->n, pPublicKeyN) == 0)
    {
        ret = CRYPT_ERR_INVALID_KEY_N;
        goto END;
    }
    if(BN_hex2bn(&decrypt_rsa->e, pPublicKeyE) == 0)
    {
        ret = CRYPT_ERR_INVALID_KEY_E;
        goto END;
    }

    // alloc encrypt_string 
	iKeySize = RSA_size(decrypt_rsa);

	// decrypt
	pcPlainBuf = (unsigned char *)OPENSSL_malloc( uiInputLen );
	
	if (uiInputLen > (unsigned int)iKeySize)
	{
		int iBlockCnt = uiInputLen / iKeySize;
		int iPos = 0;

		for (i = 0; i < iBlockCnt; ++i)
		{
			ret_size = RSA_public_decrypt( iKeySize, pInput + i * iKeySize, pcPlainBuf + iPos, decrypt_rsa, RSA_PKCS1_PADDING );
			if (ret_size < 1)
			{
				ret =  CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;
				goto END;
			}
			iPos += ret_size;
		}

		iPlainSize = iPos;
	}
	else
	{
		ret_size = RSA_public_decrypt( iKeySize, pInput, pcPlainBuf, decrypt_rsa, RSA_PKCS1_PADDING);

		if (ret_size < 1)
		{
			ret = CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;
			goto END;
		}

		iPlainSize = ret_size;
	}

END:
	RSA_free(decrypt_rsa);

	if(CRYPT_OK != ret)
	{
		OPENSSL_free(pcPlainBuf);
	}
	else
	{
		*ppOutput = pcPlainBuf;
		*uiOutputLen = iPlainSize;
	}

	return ret;
}

/////
int rsa_private_decrypt(unsigned char** _out, unsigned int* _outlen,  const unsigned char* _in, unsigned int _inlen, RSA* _key)
{
	if(!_in || !_key || _inlen < 8 || _inlen % 8 != 0) { return __LINE__; }

	int ret = 0;

	// load priv key
	RSA *Rsa = _key;
	if (!Rsa) { return __LINE__; }
	unsigned int iKeySize = (unsigned int)RSA_size(Rsa);

	// decrypt
    unsigned char *pcPlainBuf = (unsigned char*)calloc(_inlen, sizeof(unsigned char));
    if (!pcPlainBuf) { return __LINE__; }
	int iPlainSize = 0;
	if (_inlen > (unsigned int)iKeySize)
	{
		unsigned int iBlockCnt = _inlen / iKeySize;

		unsigned int iPos = 0;
		unsigned int i = 0;
		for (i = 0; i < iBlockCnt; ++i)
		{
			unsigned int iBlockSize = 0;
			ret = RSA_private_decrypt( iKeySize, (const unsigned char*)_in + i * iKeySize, pcPlainBuf + iPos, Rsa, RSA_PKCS1_PADDING );
			if (ret < 1)
			{
				free(pcPlainBuf);
				return __LINE__;

			}
			iPos += ret;
		}

		iPlainSize = iPos;
	}
	else
	{
        ret = RSA_private_decrypt( iKeySize,  (const unsigned char*)_in, pcPlainBuf, Rsa, RSA_PKCS1_PADDING);

		if (ret < 1)
		{
			free(pcPlainBuf);
			return __LINE__;
		}
       
		iPlainSize = ret;
	}

	*_out =  pcPlainBuf;
    *_outlen = (unsigned int)iPlainSize;
    
    return 0;
}
#include "xlogger/xlogger.h"
#include "openssl/evp.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
//_is_private_key == true: _pem_key is rsa private key
//_is_private_key == false: _pem_key is rsa public key
static EVP_PKEY* change_pem_key_to_evp_key(const char* _pem_key,
                                           unsigned int _key_len, bool _is_private_key) {
    
    BIO* bio = BIO_new(BIO_s_mem());
    int len = BIO_write(bio, _pem_key, _key_len);
    if (static_cast<unsigned int>(len) != _key_len) {
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
static int GetPubKeyParms(EVP_PKEY* pkey, std::string* modulus, std::string* exponent) {
    RSA* rsa = EVP_PKEY_get1_RSA(pkey);
    if (rsa == NULL) {
        return -1;
    }
    char* tmp = NULL;
    tmp = BN_bn2hex(rsa->n);
    if (tmp == NULL) {
        return -1;
    }
    *modulus = std::string(tmp);
    OPENSSL_free(tmp);
    tmp = BN_bn2hex(rsa->e);
    if (tmp == NULL) {
        return -1;
    }
    *exponent = std::string(tmp);
    OPENSSL_free(tmp);
    return 0;
}
int rsa_public_decrypt_pemkey(const unsigned char* pInput, unsigned int uiInputLen
                       , unsigned char** ppOutput, unsigned int* uiOutputLen
                       , const char* pPemPubKey, unsigned int pemPubKeyLen)
{
    EVP_PKEY* evp_pub_key = change_pem_key_to_evp_key(pPemPubKey, pemPubKeyLen, false);
    if (NULL == evp_pub_key) {
        xerror2(TSF"change_pem_key_to_evp_key failed.");
        return -1;
    }
    std::string modulus, exponent;
    
    if (0!=GetPubKeyParms(evp_pub_key, &modulus, &exponent)) {
        xerror2(TSF"GetPubKeyParms failed.");
        EVP_PKEY_free(evp_pub_key); //free evp_pub_key
        return -1;
    }
    EVP_PKEY_free(evp_pub_key); //free evp_pub_key
        
    return rsa_public_decrypt(pInput, uiInputLen, ppOutput, uiOutputLen, modulus.c_str(), exponent.c_str());
        
}

int rsa_public_encrypt_pemkey(const unsigned char* pInput, unsigned int uiInputLen
                       , unsigned char** ppOutput, unsigned int* uiOutputLen
                       , const char* pPemPubKey, unsigned int pemPubKeyLen)
{
    EVP_PKEY* evp_pub_key = change_pem_key_to_evp_key(pPemPubKey, pemPubKeyLen, false);
    if (NULL == evp_pub_key) {
        xerror2(TSF"change_pem_key_to_evp_key failed.");
        return -1;
    }
    std::string modulus, exponent;
    
    if (0!=GetPubKeyParms(evp_pub_key, &modulus, &exponent) ){
        xerror2(TSF"GetPubKeyParms failed.");
        EVP_PKEY_free(evp_pub_key); //free evp_pub_key
        return -1;
    }
        EVP_PKEY_free(evp_pub_key); //free evp_pub_key
        
    return rsa_public_encrypt(pInput, uiInputLen, ppOutput, uiOutputLen, modulus.c_str(), exponent.c_str());
}

int rsa_private_decrypt_pemkey(const unsigned char* pInput, unsigned int uiInputLen
                               , unsigned char** ppOutput, unsigned int* uiOutputLen
                               , const char* pPemPriKey, unsigned int pemPriKeyLen) {
    EVP_PKEY* evp_pri_key = change_pem_key_to_evp_key(pPemPriKey, pemPriKeyLen, true);
    if (NULL == evp_pri_key) {
        xerror2(TSF"change_pem_key_to_evp_key failed.");
        return -1;
    }
    
    RSA* rsa = EVP_PKEY_get1_RSA(evp_pri_key);
    if (rsa == NULL) {
        EVP_PKEY_free(evp_pri_key); //free evp_pri_key
        return -1;
    }

    int ret = rsa_private_decrypt(ppOutput, uiOutputLen, pInput, uiInputLen, rsa);
    
    EVP_PKEY_free(evp_pri_key); //free evp_pri_key
    return ret;
}
