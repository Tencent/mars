#include "openssl/crypto.h"
#include "openssl/bio.h"
//#include <openssl/evp.h>
#include "openssl/x509.h"
#include "openssl/ecdsa.h"
#include "openssl/err.h"
#include "openssl/bn.h"
#include "openssl/sha.h"

#include <stdio.h>
//#include "ecdsa_verify.h"

int doEcdsaVerify(const unsigned char* _pubkey, int len_key, const char* _content, int len_ctn, const char* _strSig, int len_sig)
{
	EC_KEY* key = NULL;
	int ret;
	ECDSA_SIG * sig = NULL;

	const unsigned char* pKey = _pubkey;
	key = d2i_EC_PUBKEY(NULL, (const unsigned char**)&pKey, (long)len_key);
	if(NULL == key)
	{
		return -1;
	}

//	unsigned char * tmp = (unsigned char *)calloc( 128, sizeof(unsigned char));
//	int len = Comm::DecodeBase64((unsigned char*)_strSig, tmp, len_sig);

	sig = d2i_ECDSA_SIG( NULL, (const unsigned char**)&_strSig, len_sig);
	ret = ECDSA_do_verify( (unsigned char*)_content, len_ctn, sig, key );

	EC_KEY_free( key );
	ECDSA_SIG_free( sig );

	return ret;
}

int doEcdsaSHAVerify(const unsigned char* _pubkey, int len_key, const char* _content, int len_ctn, const char* _strSig, int len_sig)
{
    EC_KEY* key = NULL;
    int ret;
    ECDSA_SIG * sig = NULL;
    unsigned char dgst[SHA256_DIGEST_LENGTH] = {0};
    const unsigned char* pKey = _pubkey;

    key = d2i_EC_PUBKEY(NULL, (const unsigned char**)&pKey, (long)len_key);
    if(NULL == key)
    {
        return -1;
    }
    
    SHA256((const unsigned char*)_content, len_ctn, &dgst[0]);
    
    sig = d2i_ECDSA_SIG( NULL, (const unsigned char**)&_strSig, len_sig);
    ret = ECDSA_do_verify( (unsigned char*)&dgst[0], SHA256_DIGEST_LENGTH, sig, key );
    
    EC_KEY_free( key );
    ECDSA_SIG_free( sig );
    
    return ret;
}
