#if 0
#include <stdlib.h>
#include <string.h>

#include <openssl/rsa.h>

#define ARRARY_SIZE(arrary) (sizeof(arrary)/sizeof(arrary[0]))
int ras_keypair_string2key(const char* _keystring, RSA** _key)
{
    BIGNUM* rsa_bn[8] = {0};
    size_t i = 0;
    char* keystring = strdup(_keystring);
    char* start = keystring;

    for (i=0; i<ARRARY_SIZE(rsa_bn); ++i)
    {
        char* tok = (char*)strchr(start, ';');
        if (NULL==tok) break;
        *tok = '\0';
        BN_hex2bn(&rsa_bn[i], start);
        start = tok+1;
    }
    free(keystring);

    *_key = RSA_new();
   	(*_key)->n = rsa_bn[0];
   	(*_key)->e = rsa_bn[1];
   	(*_key)->d = rsa_bn[2];
   	(*_key)->p = rsa_bn[3];
   	(*_key)->q = rsa_bn[4];
   	(*_key)->dmp1 = rsa_bn[5];
   	(*_key)->dmq1 = rsa_bn[6];
   	(*_key)->iqmp = rsa_bn[7];

	return 0;
}

char* rsa_keypair_key2string(RSA* _pkey)
{
    char* rsa_bn[] =
    {
    	BN_bn2hex(_pkey->n),
    	BN_bn2hex(_pkey->e),
    	BN_bn2hex(_pkey->d),
    	BN_bn2hex(_pkey->p),
    	BN_bn2hex(_pkey->q),
    	BN_bn2hex(_pkey->dmp1),
    	BN_bn2hex(_pkey->dmq1),
    	BN_bn2hex(_pkey->iqmp),
    };

   size_t len = 1;
   size_t i = 0;
   for (i=0; i<ARRARY_SIZE(rsa_bn); ++i)
   {
	   len += strlen(rsa_bn[i]);
	   len += 1;
   }

   char* out = OPENSSL_malloc(len);
   memset(out, 0, len);
   for (i=0; i<ARRARY_SIZE(rsa_bn); ++i)
   {
       strcat(out, rsa_bn[i]);
       strcat(out, ";");
       OPENSSL_free(rsa_bn[i]);
   }

   out[len-1] = '\0';
   return out;
}

RSA* rsa_keypair_gen(int key_length)
{
	int status = 0;
	BIGNUM* bne = NULL;
	RSA* rkey = NULL;

	bne = BN_new();
	if (bne == NULL) {
		status = -1;
		goto cleanup;
	}
	// use 0x10001 as exponent, as client did before
	if ((BN_set_word(bne, RSA_F4)) != 1) {
		status = -1;
		goto cleanup;
	}
	rkey = RSA_new();
	if (rkey == NULL) {
		status = -1;
		goto cleanup;
	}
	if ((RSA_generate_key_ex(rkey, key_length, bne, NULL)) != 1) {
		status = -1;
		goto cleanup;
	}

cleanup:
	// rkey is pointed by pkey's struct member, so should not free.
	if (bne != NULL) BN_free(bne);
	if (rkey != NULL && status != 0) {
		RSA_free(rkey);
		rkey = NULL;
	}
	return rkey;
}

void ras_keypair_freestring(char* _keystring)
{
	OPENSSL_free(_keystring);
}

void ras_keypair_freekey(RSA* _key)
{
	RSA_free(_key);
}

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
#endif