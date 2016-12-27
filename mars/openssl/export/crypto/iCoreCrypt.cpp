/*
 *  iCoreCrypt.cpp
 *  此文件仅给客户端使用，不能含有其它依赖，只能依赖于openssl
 *  测试代码见 http://scm-gy/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/comm2/core/crypto/test/
 *
 *  Created by austindeng on 14-11-5.
 *  Copyright 2014 Tencent. All rights reserved.
 *
 */

#include "iCoreCrypt.h"
#include <openssl/aes.h>

#ifndef WIN32
#include <arpa/inet.h>
#else
#include <windows.h>
#include <WinSock2.h>
#endif

#include "openssl/evp.h"
#include "openssl/ec.h"
#include "openssl/bio.h"
#include "openssl/evp.h"
#include "openssl/pem.h"
#include<openssl/md5.h>
#include <openssl/err.h>


#include "zlib.h"
#include "zconf.h"


#include <limits.h>
#include<sstream>
#ifdef LINUX
#include "iLog.h"
#else
#define MMERR(fmt,...) 
#endif

using namespace std;

namespace Comm
{

#define MD5_DIGEST_LENGTH 16
#define SHA_DIGEST_LENGTH 20
static void *KDF_SHA1(const void *in, size_t inlen, void *out, size_t *outlen)
{
#ifndef OPENSSL_NO_SHA
  if (*outlen < SHA_DIGEST_LENGTH)
		return NULL;
  else
		*outlen = SHA_DIGEST_LENGTH;
  return SHA1((const unsigned char*)in, inlen, (unsigned char*)out);
#else
  return NULL;
#endif
}
static void *KDF_MD5(const void *in, size_t inlen, void *out, size_t *outlen)
{
  if (*outlen < MD5_DIGEST_LENGTH)
  {
	return NULL;
  }
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx,in,inlen);
  MD5_Final((unsigned char*)out,&ctx);
  *outlen = MD5_DIGEST_LENGTH;
  return out;
}
typedef void *(*KDF_FUNC)(const void *in, size_t inlen, void *out, size_t *outlen);
static struct {
	int iLen;
	KDF_FUNC pFunc;
} __kdf_list[] = {
	{MD5_DIGEST_LENGTH,KDF_MD5},
	{SHA_DIGEST_LENGTH,KDF_SHA1},
};

//EVP_PKEY* LoadPrivKey(const char *KeyFile)
//{
//	BIO *BioKey = NULL;
//	EVP_PKEY *Key = NULL;
//
//	BioKey = BIO_new(BIO_s_file());
//	if (BioKey == NULL)
//	{
//		goto Error;
//	}
//	if (BIO_read_filename(BioKey, KeyFile) <= 0)
//	{
//		goto Error;
//	}
//	Key = PEM_read_bio_PrivateKey(BioKey, NULL, NULL, NULL);
//Error:
//	if (BioKey)
//	{
//		BIO_free(BioKey);
//		BioKey = NULL;
//	}
//	return Key;
//}

int RSAEncrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* poResult, 
			   const char* pPublicKeyN, const char *pPublicKeyE)
{
	if(!pInput || !pPublicKeyN || !pPublicKeyE || !poResult)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	// load priv key
	RSA *Rsa = RSA_new();
	if( !Rsa )
	{
		return CRYPT_ERR_NO_MEMORY;
	}
	size_t len = BN_hex2bn(&Rsa->n,pPublicKeyN);
	if( strlen(pPublicKeyN) != len)
	{
		MMERR("Err %s lene %zu :%zu",__func__,strlen(pPublicKeyN),len);
		RSA_free(Rsa);
		return CRYPT_ERR_INVALID_KEY_N;
	}
	len = BN_hex2bn(&Rsa->e,pPublicKeyE);
	if( strlen(pPublicKeyE) != len)
	{
		MMERR("Err %s lene %zu :%zu",__func__,strlen(pPublicKeyE),len);
		RSA_free(Rsa);
		return CRYPT_ERR_INVALID_KEY_E;
	}

	unsigned int keySize  = RSA_size(Rsa);
	poResult->reserve( uiInputLen );

	char tmp[ 8192 ] = { 0 };

	int ret = 0;
	for(unsigned int i = 0; i < uiInputLen; i += ( keySize - 12 ) )
	{
		unsigned int len = ( uiInputLen - i ) > ( keySize - 12 ) ? ( keySize - 12 ) : ( uiInputLen - i );

		/* RSA_private_encrypt */
		ret = RSA_public_encrypt( len, (unsigned char*)pInput+ i,
				(unsigned char*)tmp, Rsa, RSA_PKCS1_PADDING );
		if( ret < 1 )
		{
			MMERR( "RSA_private_decrypt fail on block %d of %d, blocksize %d",
					i / keySize, uiInputLen / keySize, keySize );
			RSA_free(Rsa);
			return CRYPT_ERR_ENCRYPT_WITH_RSA_PUBKEY;
		}
		poResult->append( tmp, ret );
	}
	RSA_free(Rsa);
	return CRYPT_OK;
}

int RSAPrivateEncrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* poResult, 
		     EVP_PKEY* pKey)
{
	if(!pInput || !pKey ||  !pKey->pkey.rsa || !poResult)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	// load priv key
	RSA *Rsa = EVP_PKEY_get1_RSA(pKey);
	unsigned int keySize  = RSA_size(Rsa);
	poResult->reserve( uiInputLen );

	char tmp[ 8192 ] = { 0 };

	int ret = 0;
	for(unsigned int i = 0; i < uiInputLen; i += ( keySize - 12 ) )
	{
		unsigned int len = ( uiInputLen - i ) > ( keySize - 12 ) ? ( keySize - 12 ) : ( uiInputLen - i );

		/* RSA_private_encrypt */
		ret = RSA_private_encrypt( len, (unsigned char*)pInput+ i,
				(unsigned char*)tmp, Rsa, RSA_PKCS1_PADDING );
		if( ret < 1 )
		{
			MMERR( "RSA_private_decrypt fail on block %d of %d, blocksize %d",
					i / keySize, uiInputLen / keySize, keySize );
			return CRYPT_ERR_ENCRYPT_WITH_RSA_PUBKEY;
		}
		poResult->append( tmp, ret );
	}
	return CRYPT_OK;
}
int RSAPublicDecrypt( const unsigned char* pInput,
			   unsigned int uiInputLen,
			   std::string* sOutput,
			   const char* pPublicKeyN, const char *pPublicKeyE)
{
	if(!pInput || !pPublicKeyN || !pPublicKeyE || !sOutput)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	// load priv key
	RSA *Rsa = RSA_new();
	if( !Rsa )
	{
		return CRYPT_ERR_NO_MEMORY;
	}
	size_t len = BN_hex2bn(&Rsa->n,pPublicKeyN);
	if( strlen(pPublicKeyN) != len)
	{
		MMERR("Err %s lene %zu :%zu",__func__,strlen(pPublicKeyN),len);
		RSA_free(Rsa);
		return CRYPT_ERR_INVALID_KEY_N;
	}
	len = BN_hex2bn(&Rsa->e,pPublicKeyE);
	if( strlen(pPublicKeyE) != len)
	{
		MMERR("Err %s lene %zu :%zu",__func__,strlen(pPublicKeyE),len);
		RSA_free(Rsa);
		return CRYPT_ERR_INVALID_KEY_E;
	}
	int ret = 0;

	int iKeySize = RSA_size(Rsa);

	// decrypt
	unsigned char *pcPlainBuf = (unsigned char *)OPENSSL_malloc( uiInputLen );
	int iPlainSize = 0;

	if (uiInputLen > (unsigned int)iKeySize)
	{
		// 鐓﹀儑璩よ簢
		int iBlockCnt = uiInputLen / iKeySize;

		int iPos = 0;

		for (int i = 0; i < iBlockCnt; ++i)
		{
			int iBlockSize = 0;			
			ret = RSA_public_decrypt( iKeySize, 
				pInput + i * iKeySize, 
				pcPlainBuf + iPos, 
				Rsa, RSA_PKCS1_PADDING ); 
			if (ret < 1)
			{
				MMERR( "RSA: MultiBlock RSA_private_decrypt fail on block %d of %d, blocksize %d\n", 
					i, iBlockCnt, iBlockSize
					);

				MMERR( "RSA_private_decrypt return %d\n", ret);
				OPENSSL_free(pcPlainBuf);
				RSA_free(Rsa);

				return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;

			}			
			iPos += ret;
		}

		iPlainSize = iPos;
	}
	else
	{
		// 绛夊儑璩よ簢
		ret = RSA_public_decrypt(
			iKeySize, 
			pInput, 
			pcPlainBuf, 
			Rsa, 
			RSA_PKCS1_PADDING);

		if (ret < 1)
		{
			MMERR( "RSA: SingleBlock RSA_private_decrypt fail\n");
			OPENSSL_free(pcPlainBuf);
			RSA_free(Rsa);
			return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;
		}

		iPlainSize = ret;
	}
	sOutput->assign((const char*)pcPlainBuf,iPlainSize);
	OPENSSL_free(pcPlainBuf);
	RSA_free(Rsa);
	return CRYPT_OK;
}

int RSAPublicDecrypt( const unsigned char* pInput,
			   unsigned int uiInputLen,
			   std::string* sOutput,
			   EVP_PKEY* Key
			   )			
{
	if(!pInput || !Key || uiInputLen < 8 || uiInputLen % 8 != 0 || !sOutput)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}

	int ret = 0;

	// load priv key
	RSA *Rsa = NULL;

	Rsa = EVP_PKEY_get1_RSA(Key);
	if (!Rsa)
	{
		MMERR( "Failed, EVP_PKEY_get1_RSA");
		return CRYPT_ERR_LOAD_RSA_PRIVATE_KEY;
	}

	int iKeySize = RSA_size(Rsa);

	// decrypt
	unsigned char *pcPlainBuf = (unsigned char *)OPENSSL_malloc( uiInputLen );
	int iPlainSize = 0;

	if (uiInputLen > (unsigned int)iKeySize)
	{
		// 鐓﹀儑璩よ簢
		int iBlockCnt = uiInputLen / iKeySize;

		int iPos = 0;

		for (int i = 0; i < iBlockCnt; ++i)
		{
			int iBlockSize = 0;			
			ret = RSA_public_decrypt( iKeySize, 
				pInput + i * iKeySize, 
				pcPlainBuf + iPos, 
				Rsa, RSA_PKCS1_PADDING ); 
			if (ret < 1)
			{
				MMERR( "RSA: MultiBlock RSA_private_decrypt fail on block %d of %d, blocksize %d\n", 
					i, iBlockCnt, iBlockSize
					);

				MMERR( "RSA_private_decrypt return %d\n", ret);
				OPENSSL_free(pcPlainBuf);

				return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;

			}			
			iPos += ret;
		}

		iPlainSize = iPos;
	}
	else
	{
		// 绛夊儑璩よ簢
		ret = RSA_public_decrypt(
			iKeySize, 
			pInput, 
			pcPlainBuf, 
			Rsa, 
			RSA_PKCS1_PADDING);

		if (ret < 1)
		{
			MMERR( "RSA: SingleBlock RSA_private_decrypt fail\n");
			OPENSSL_free(pcPlainBuf);
			return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;
		}

		iPlainSize = ret;
	}
	sOutput->assign((const char*)pcPlainBuf,iPlainSize);
	OPENSSL_free(pcPlainBuf);

	return CRYPT_OK;
}

int RSADecrypt( const unsigned char* pInput,
			   unsigned int uiInputLen,
			   std::string* sOutput,
			   EVP_PKEY* Key
			   )			
{
	if(!pInput || !Key || uiInputLen < 8 || uiInputLen % 8 != 0 || !sOutput)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}

	int ret = 0;

	// load priv key
	RSA *Rsa = NULL;

	Rsa = EVP_PKEY_get1_RSA(Key);
	if (!Rsa)
	{
		MMERR( "Failed, EVP_PKEY_get1_RSA");
		return CRYPT_ERR_LOAD_RSA_PRIVATE_KEY;
	}

	int iKeySize = RSA_size(Rsa);

	// decrypt
	unsigned char *pcPlainBuf = (unsigned char *)OPENSSL_malloc( uiInputLen );
	int iPlainSize = 0;

	if (uiInputLen > (unsigned int)iKeySize)
	{
		// 鐓﹀儑璩よ簢
		int iBlockCnt = uiInputLen / iKeySize;

		int iPos = 0;

		for (int i = 0; i < iBlockCnt; ++i)
		{
			int iBlockSize = 0;			
			ret = RSA_private_decrypt( iKeySize, 
				pInput + i * iKeySize, 
				pcPlainBuf + iPos, 
				Rsa, RSA_PKCS1_PADDING ); 
			if (ret < 1)
			{
				MMERR( "RSA: MultiBlock RSA_private_decrypt fail on block %d of %d, blocksize %d\n", 
					i, iBlockCnt, iBlockSize
					);

				MMERR( "RSA_private_decrypt return %d\n", ret);
				OPENSSL_free(pcPlainBuf);

				return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;

			}			
			iPos += ret;
		}

		iPlainSize = iPos;
	}
	else
	{
		// 绛夊儑璩よ簢
		ret = RSA_private_decrypt(
			iKeySize, 
			pInput, 
			pcPlainBuf, 
			Rsa, 
			RSA_PKCS1_PADDING);

		if (ret < 1)
		{
			MMERR( "RSA: SingleBlock RSA_private_decrypt fail\n");
			OPENSSL_free(pcPlainBuf);
			return CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY;
		}

		iPlainSize = ret;
	}
	sOutput->assign((const char*)pcPlainBuf,iPlainSize);
	OPENSSL_free(pcPlainBuf);

	return CRYPT_OK;
}

//int RSADecrypt( const unsigned char* pInput,
//			   unsigned int uiInputLen,
//			   std::string* pOutput,
//			   const char *pPrivKeyFile
//			   )			
//{
//	EVP_PKEY *Key = LoadPrivKey(pPrivKeyFile);
//	if (!Key)
//	{
//		MMERR( "Failed loading RSA private key file \"%s\"", pPrivKeyFile);
//		return CRYPT_ERR_LOAD_RSA_PRIVATE_KEY;
//	}
//	int ret = CRYPT_OK;
//	ret = RSADecrypt(pInput, uiInputLen, pOutput, Key);
//	EVP_PKEY_free(Key);
//	return ret;
//}

static int ZLibCompress(const unsigned char* ptSrcData,unsigned int iSrcLen 
		,unsigned char**ppDestData, unsigned int& iDestLen)
{
	if( !ptSrcData || iSrcLen == 0 || !ppDestData )
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	uLongf ulDestLen = (uLongf)compressBound(iSrcLen);
	*ppDestData = (unsigned char*)OPENSSL_malloc(ulDestLen+1);
	if( !(*ppDestData))
	{
		return CRYPT_ERR_NO_MEMORY;
	}
	int ret = compress((Bytef*)*ppDestData,(uLongf*)&ulDestLen,(const Bytef*)ptSrcData,(uLongf)iSrcLen);
	if( 0 != ret)
	{
		if( *ppDestData) { OPENSSL_free(*ppDestData),*ppDestData= NULL;}
		return CRYPT_ERR_COMPRESS;
	}
	iDestLen=(unsigned int)ulDestLen;
	return CRYPT_OK;
}

static int ZLibUnCompress(const unsigned char* ptSrcData,unsigned int iSrcLen
		,unsigned char**ppDestData, unsigned int& iDestLen)
{
	if( !ptSrcData || iSrcLen == 0 || !ppDestData )
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	uLongf ulDestLen = (uLongf)iSrcLen;
	int ret = CRYPT_ERR_INVALID_PARAM;
	do{
		if( *ppDestData)
		{
			OPENSSL_free(*ppDestData),*ppDestData=NULL;
		}
		ulDestLen*=2;
		*ppDestData = (unsigned char*)OPENSSL_malloc(ulDestLen+1);
		if( !(*ppDestData))
		{
			return CRYPT_ERR_NO_MEMORY;
		}
		ret = uncompress((Bytef*)*ppDestData,(uLongf*)&ulDestLen,(const Bytef*)ptSrcData,(uLongf)iSrcLen);
	}while( ret == Z_BUF_ERROR );
	if( 0  != ret)
	{
		MMERR("ERR %s uncompreses fail.ret %d",__func__,ret);
		if( *ppDestData )
		{
			OPENSSL_free(*ppDestData),*ppDestData=NULL;
		}
		return CRYPT_ERR_DECOMPRESS;
	}
	if(ulDestLen>UINT_MAX)
	{
		MMERR("ERR %s uncompress data len %lu not support MAX_UINT",__func__,ulDestLen);
		return CRYPT_ERR_DECOMPRESS;
	}
	iDestLen=(unsigned int)ulDestLen;
	return CRYPT_OK;
}


#define CHECK_RSA_BUF_LEN( len ) \
	if( (len) > 244 )\
	{\
		MMERR("WARN %s rsa len %u over 244",__func__,(len));\
	}

int DoubleHybridEncrypt(const unsigned char* pRsaInput, unsigned int uiRsaInputLen,
		const unsigned char *pAesInput,unsigned int uiAesInputLen,
		const char* pPublicKeyN, const char *pPublicKeyE,
		const char* pKey, unsigned int uiKeyLen,
		std::string* pOutput)
{
	if( !pOutput || !pRsaInput || !pAesInput || !pPublicKeyN || !pPublicKeyE
			|| !pKey || 0 ==uiRsaInputLen || 0 == uiAesInputLen || 0 == uiKeyLen)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const	size_t iHeadLen = sizeof(DoubleHybridPackBodyHead_t);
	DoubleHybridPackBodyHead_t* ptHead = new DoubleHybridPackBodyHead_t;
	if( !ptHead)
	{
		return CRYPT_ERR_NO_MEMORY;
	}

	ptHead->RsaRawLen = htonl(uiRsaInputLen);
	ptHead->AesRawLen = htonl(uiAesInputLen);

	CHECK_RSA_BUF_LEN(uiRsaInputLen);

	//compress rsa data
	unsigned  iDestLen = 0;
	unsigned char* pDestData = NULL;
	int ret = ZLibCompress(pRsaInput,uiRsaInputLen,&pDestData,iDestLen);
	if( 0 != ret)
	{
		if( pDestData){ OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return CRYPT_ERR_COMPRESS;
	}
	
	//encrypt rsa data
	std::string sOutput ;
	if( 0 != ( ret = RSAEncrypt(pDestData,iDestLen,&sOutput,pPublicKeyN,pPublicKeyE)))
	{
		if( pDestData){ OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return ret;
	}
	ptHead->RsaBufLen = htonl(sOutput.length());
	
	//encrypt rsa data with aes key
	std::string sRsaAesOutput;
	if( 0 != ( ret = AESEncrypt( pDestData,iDestLen,&sRsaAesOutput,pKey,uiKeyLen)))
	{
		if( pDestData) { OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return ret;
	}
	ptHead->RsaBufWithAesLen = htonl(sRsaAesOutput.length());
	
	//compress aes data
	unsigned int iAesDestLen = 0;
	unsigned char* pAesDestData = NULL;
	ret = ZLibCompress(pAesInput,uiAesInputLen,&pAesDestData,iAesDestLen);
	if( 0 != ret)
	{
		if( pAesDestData){ OPENSSL_free(pAesDestData),pAesDestData = NULL;}
		if( pDestData) { OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return CRYPT_ERR_COMPRESS;
	}

	//encrypt aes data
	std::string sAesOutput;
	if( 0 != ( ret = AESEncrypt( pAesDestData,iAesDestLen,&sAesOutput,pKey,uiKeyLen)))
	{
		if( pAesDestData){ OPENSSL_free(pAesDestData),pAesDestData = NULL;}
		if( pDestData) { OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return ret;
	}
	pOutput->reserve(iHeadLen+sAesOutput.length()+sRsaAesOutput.length()+sOutput.length());
	pOutput->assign((const char*)ptHead,iHeadLen);
	pOutput->append((const char*)sOutput.data(),sOutput.length());
	pOutput->append((const char*)sRsaAesOutput.data(),sRsaAesOutput.length());
	pOutput->append((const char*)sAesOutput.data(),sAesOutput.length());
	
	OPENSSL_free(pAesDestData),pAesDestData = NULL;
	OPENSSL_free(pDestData),pDestData = NULL;
	delete ptHead,ptHead=NULL;
	return CRYPT_OK;
}




int HybridEncrypt(const unsigned char* pRsaInput, unsigned int uiRsaInputLen,
		const unsigned char *pAesInput,unsigned int uiAesInputLen,
		const char* pPublicKeyN, const char *pPublicKeyE,
		const char* pKey, unsigned int uiKeyLen,
		std::string* pOutput)
{
	if( !pOutput || !pRsaInput || !pAesInput || !pPublicKeyN || !pPublicKeyE
			|| !pKey || 0 ==uiRsaInputLen || 0 == uiAesInputLen || 0 == uiKeyLen)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const	size_t iHeadLen = sizeof(HybridPackBodyHead_t);
	HybridPackBodyHead_t* ptHead = new HybridPackBodyHead_t;
	if( !ptHead)
	{
		return CRYPT_ERR_NO_MEMORY;
	}
	ptHead->RsaRawLen = htonl(uiRsaInputLen);
	ptHead->AesRawLen = htonl(uiAesInputLen);

	//compress rsa data
	unsigned  int iDestLen = 0;
	unsigned char* pDestData = NULL;
	int ret = ZLibCompress(pRsaInput,uiRsaInputLen,&pDestData,iDestLen);
	if( 0 != ret)
	{
		if( pDestData){ OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return CRYPT_ERR_COMPRESS;
	}

	
	//encrypt rsa data
	std::string sOutput ;
	if( 0 != ( ret = RSAEncrypt(pDestData,iDestLen,&sOutput,pPublicKeyN,pPublicKeyE)))
	{
		if( pDestData){ OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return ret;
	}
	ptHead->RsaBufLen = htonl(sOutput.length());
	

	//compress aes data
	unsigned int iAesDestLen = 0;
	unsigned char* pAesDestData = NULL;
	ret = ZLibCompress(pAesInput,uiAesInputLen,&pAesDestData,iAesDestLen);
	if( 0 != ret)
	{
		if( pAesDestData){ OPENSSL_free(pAesDestData),pAesDestData = NULL;}
		if( pDestData) { OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return CRYPT_ERR_COMPRESS;
	}
	
	//encrypt aes data
	std::string sAesOutput;
	if( 0 != ( ret = AESEncrypt( pAesDestData,iAesDestLen,&sAesOutput,pKey,uiKeyLen)))
	{
		if( pAesDestData){ OPENSSL_free(pAesDestData),pAesDestData = NULL;}
		if( pDestData) { OPENSSL_free(pDestData),pDestData = NULL;}
		delete ptHead,ptHead=NULL;
		return ret;
	}
	CHECK_RSA_BUF_LEN(uiRsaInputLen);

	pOutput->reserve(iHeadLen+sAesOutput.length()+sOutput.length());
	pOutput->assign((const char*)ptHead,iHeadLen);
	pOutput->append((const char*)sOutput.data(),sOutput.length());
	pOutput->append((const char*)sAesOutput.data(),sAesOutput.length());
	
	
	OPENSSL_free(pAesDestData),pAesDestData = NULL;
	OPENSSL_free(pDestData),pDestData = NULL;
	delete ptHead,ptHead=NULL;
	return CRYPT_OK;
}

int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
               const char* pKey, unsigned int uiKeyLen,
			   std::string* pRsaDeOutput,
			   std::string* pAesDeOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput)
{
	const	size_t iHeadLen = sizeof(DoubleHybridPackBodyHead_t);
	if(uiInputLen<=iHeadLen || !pInput || !pRsaDeOutput || !pAesDeOutput )
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const DoubleHybridPackBodyHead_t *ptHead = (DoubleHybridPackBodyHead_t*)pInput;
	pHeadOutput->ntoh(ptHead);

	const unsigned int iRsaBufLen = pHeadOutput->RsaBufLen;
	const unsigned int iRsaBufWithAesLen = pHeadOutput->RsaBufWithAesLen;
	const unsigned int iAesRawLen = pHeadOutput->AesRawLen;
	const unsigned int iRsaRawLen = pHeadOutput->RsaRawLen;

	//decrypt rsa buffer with aes key
	int ret =  HybridAesDecrypt(pInput+iHeadLen+iRsaBufLen, iRsaBufWithAesLen,
			pKey, uiKeyLen,iRsaRawLen,pRsaDeOutput);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s hybrid aes decrypt rsa buff fail.ret %d",__func__,ret);
		return ret;
	}

	//decrypt aes buffer with aes key
	ret = HybridAesDecrypt(pInput+iHeadLen+iRsaBufLen+iRsaBufWithAesLen,uiInputLen-(iHeadLen+iRsaBufLen+iRsaBufWithAesLen),
			pKey, uiKeyLen,iAesRawLen,pAesDeOutput);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s hybrid aes decrypt aes buf fail.ret %d",__func__,ret);
		return ret;
	}
	return CRYPT_OK;
}

int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   RsaDecrypter* pDecrypter,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput)
{
	const	size_t iHeadLen = sizeof(DoubleHybridPackBodyHead_t);
	if(uiInputLen<=iHeadLen || !pInput || !pRsaDeOutput || !pAesEnOutput || !pHeadOutput )
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const DoubleHybridPackBodyHead_t *ptHead = (DoubleHybridPackBodyHead_t*)pInput;
	pHeadOutput->ntoh(ptHead);

	unsigned int iRsaBufLen = pHeadOutput->RsaBufLen;
	if( iRsaBufLen > uiInputLen- iHeadLen)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const unsigned int iRsaBufWithAesLen = pHeadOutput->RsaBufWithAesLen;

	int ret = 0;
	//decrypt with rsa
	if( 0 != ( ret = pDecrypter->Decrypt(pInput+iHeadLen,iRsaBufLen,pRsaDeOutput) ))
	{
		return ret;
	}
	//no aes data
	if( iHeadLen+iRsaBufLen + iRsaBufWithAesLen == uiInputLen)
	{
		pAesEnOutput->clear();
		return CRYPT_OK;
	}

	//decompress rsa data
	unsigned int  iDestLen = 0;
	unsigned char* pDestData =  NULL;
	ret = ZLibUnCompress((const unsigned char*)pRsaDeOutput->data(),pRsaDeOutput->length()
			,&pDestData,iDestLen);
	if( 0 != ret || (!pDestData))
	{
		MMERR("ERR %s uncompreses rsa fail.ret %d",__func__,ret);
		if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
		return CRYPT_ERR_DECOMPRESS;
	}
	//check rsa data len
	unsigned int iRsaRawLen = pHeadOutput->RsaRawLen;
	if( iDestLen != iRsaRawLen)
	{
		MMERR("ERR %s %d rsa uncompress datalen %u not match raw rsa len %u",
				__func__,__LINE__,iDestLen,iRsaRawLen);
		if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
		return CRYPT_ERR_MATCH_LEN;
	}
	CHECK_RSA_BUF_LEN(iRsaRawLen);
	pRsaDeOutput->assign((const char*)pDestData,iDestLen);
	pAesEnOutput->assign((const char*)(pInput+iHeadLen+iRsaBufLen+iRsaBufWithAesLen),uiInputLen-iRsaBufLen-iHeadLen-iRsaBufWithAesLen);
	if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
	return CRYPT_OK;
}

void HybridPackBodyHead_t::ntoh(const HybridPackBodyHead_t* ptSrc)
{
	if(!ptSrc){ return;}
	this->RsaRawLen = ntohl( ptSrc->RsaRawLen);
	this->AesRawLen = ntohl( ptSrc->AesRawLen);
	this->RsaBufLen = ntohl( ptSrc->RsaBufLen);
}

void HybridPackBodyHead_t::hton(const HybridPackBodyHead_t* ptSrc)
{
	if(!ptSrc){ return;}
	this->RsaRawLen = htonl( ptSrc->RsaRawLen);
	this->AesRawLen = htonl( ptSrc->AesRawLen);
	this->RsaBufLen = htonl( ptSrc->RsaBufLen);
}

void DoubleHybridPackBodyHead_t::ntoh(const DoubleHybridPackBodyHead_t* ptSrc)
{
	if(!ptSrc){ return;}
	this->RsaRawLen = ntohl( ptSrc->RsaRawLen);
	this->AesRawLen = ntohl( ptSrc->AesRawLen);
	this->RsaBufLen = ntohl( ptSrc->RsaBufLen);
	this->RsaBufWithAesLen = ntohl(ptSrc->RsaBufWithAesLen);
}

void DoubleHybridPackBodyHead_t::hton(const DoubleHybridPackBodyHead_t* ptSrc)
{
	if(!ptSrc){ return;}
	this->RsaRawLen = htonl( ptSrc->RsaRawLen);
	this->AesRawLen = htonl( ptSrc->AesRawLen);
	this->RsaBufLen = htonl( ptSrc->RsaBufLen);
	this->RsaBufWithAesLen = htonl(ptSrc->RsaBufWithAesLen);
}


int HybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   RsaDecrypter* pDecrypter,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   HybridPackBodyHead_t* pHeadOutput)
{
	const	size_t iHeadLen = sizeof(HybridPackBodyHead_t);
	if(uiInputLen<=iHeadLen || !pInput || !pRsaDeOutput || !pAesEnOutput || !pHeadOutput )
	{
		return CRYPT_ERR_INVALID_PARAM;
	}
	const HybridPackBodyHead_t *ptHead = (HybridPackBodyHead_t*)pInput;

	pHeadOutput->ntoh(ptHead);
	unsigned int iRsaBufLen = pHeadOutput->RsaBufLen;
	if( iRsaBufLen > uiInputLen- iHeadLen)
	{
		return CRYPT_ERR_INVALID_PARAM;
	}

	int ret = 0;
	//decrypt with rsa
	if( 0 != ( ret = pDecrypter->Decrypt(pInput+iHeadLen,iRsaBufLen,pRsaDeOutput) ))
	{
		return ret;
	}
	//no aes data
	if( iHeadLen+iRsaBufLen == uiInputLen)
	{
		pAesEnOutput->clear();
		return CRYPT_OK;
	}

	//decompress rsa data
	unsigned int  iDestLen = 0;
	unsigned char* pDestData =  NULL;
	ret = ZLibUnCompress((const unsigned char*)pRsaDeOutput->data(),pRsaDeOutput->length()
			,&pDestData,iDestLen);
	if( 0 != ret || (!pDestData))
	{
		MMERR("ERR %s uncompreses rsa fail.ret %d",__func__,ret);
		if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
		return CRYPT_ERR_DECOMPRESS;
	}
	//check rsa data len
	unsigned int iRsaRawLen = pHeadOutput->RsaRawLen;
	if( iDestLen != iRsaRawLen)
	{
		MMERR("ERR %s %d rsa uncompress datalen %u not match raw rsa len %u",
				__func__,__LINE__,iDestLen,iRsaRawLen);
		if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
		return CRYPT_ERR_MATCH_LEN;
	}
	CHECK_RSA_BUF_LEN(iRsaRawLen);
	pRsaDeOutput->assign((const char*)pDestData,iDestLen);
	pAesEnOutput->assign((const char*)(pInput+iHeadLen+iRsaBufLen),uiInputLen-iRsaBufLen-iHeadLen);
	if(pDestData) { OPENSSL_free(pDestData),pDestData=NULL;}
	return CRYPT_OK;
}

class RawRsaDecrypter:public RsaDecrypter
{
	public:
		RawRsaDecrypter(EVP_PKEY* Key):m_tKey(Key){}
		int Decrypt(const unsigned char* pInput, unsigned int uiInputLen,std::string* pRsaDeOutput)
		{
			if( !pInput ||  0 == uiInputLen || !pRsaDeOutput)
			{
				return CRYPT_ERR_INVALID_PARAM;
			}
			//decrypt with rsa
			return  RSADecrypt(pInput,uiInputLen,pRsaDeOutput,m_tKey);
		}
	private:
		EVP_PKEY* m_tKey;
};

int HybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   EVP_PKEY* Key,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   HybridPackBodyHead_t* pHead)
{
	RawRsaDecrypter tDecrypter(Key);
	return HybridDecrypt(pInput,uiInputLen,&tDecrypter,pRsaDeOutput,pAesEnOutput,pHead);
}

int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   EVP_PKEY* Key,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput)
{
	RawRsaDecrypter tDecrypter(Key);
	return DoubleHybridDecrypt(pInput,uiInputLen,&tDecrypter,pRsaDeOutput,pAesEnOutput,pHeadOutput);
}


int HybridAesDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
		const char* pKey, unsigned int uiKeyLen,unsigned int iAesRawLen
		,std::string* pOutput)
{
	if( !pOutput || !pInput || uiInputLen%16 != 0 || !pKey || uiKeyLen < 16 || iAesRawLen==0)
	{
		MMERR("ERR %s:%d err param. input len %u keylen %u aesrawlen %u",__func__,__LINE__
				,uiInputLen,uiKeyLen,iAesRawLen);
		return CRYPT_ERR_INVALID_PARAM;
	}
	//decrypt
	std::string sOutput;
	int ret = AESDecrypt(pInput,uiInputLen,&sOutput,pKey,uiKeyLen);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s AESDecrypt fail. ret %d",__func__,ret);
		return ret;
	}
	//decomprese aes data
	unsigned int  iAesDestLen = 0;
	unsigned char* pAesDestData =  NULL;
	ret = ZLibUnCompress((const unsigned char*)sOutput.data(),sOutput.length()
			,&pAesDestData,iAesDestLen);
	if( 0 != ret || (!pAesDestData))
	{
		MMERR("ERR %s uncompreses aes fail.ret %d",__func__,ret);
		if(pAesDestData) { OPENSSL_free( pAesDestData),pAesDestData=NULL;}
		return CRYPT_ERR_DECOMPRESS;
	}
	if( iAesDestLen != iAesRawLen)
	{
		MMERR("ERR %s:%d aes uncompress datalen %u not match raw rsa len %u",
				__func__,__LINE__,iAesDestLen,iAesRawLen);
		if(pAesDestData) { OPENSSL_free(pAesDestData),pAesDestData=NULL;}
		return CRYPT_ERR_MATCH_LEN;
	}
	pOutput->assign((const char*)pAesDestData,iAesDestLen);
	if(pAesDestData) { OPENSSL_free(pAesDestData),pAesDestData=NULL;}
	return CRYPT_OK;
}

#ifdef LINUX
//客户端裁剪掉了evp接口，所以只在服务器编译使用evp接口的版本

int AESEncrypt(const unsigned char* pInput, unsigned int uiInputLen,std::string* rbOutput,
               const char* pKey, unsigned int uiKeyLen) {
    if (!pInput || !pKey || !rbOutput)
    {
        return CRYPT_ERR_INVALID_PARAM;
    }

    //EVP 接口会保证使用平台特有的密码学加速，例如AES-NI
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_CIPHER_CTX_set_padding(&ctx,1);//使用pkcs padding

    unsigned char key[16], iv[16]; //aes_128的 key size=128bit , iv size=128bit
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    memcpy(key, pKey, uiKeyLen > 16U ? 16U : uiKeyLen);
    memcpy(iv, key, sizeof(key));
    bool ret=CRYPT_ERR_NO_MEMORY;
    do{
        if(1 != EVP_EncryptInit_ex(&ctx, cipher, NULL, key, iv)){
            break;
        }

        rbOutput->assign(uiInputLen + EVP_CIPHER_block_size(cipher),0);
        int len=rbOutput->size();
        int ciphertext_len=0;

        if(1 != EVP_EncryptUpdate(&ctx, (unsigned char *) &(*rbOutput)[0], &len, pInput, uiInputLen)){
            break;
        }
        ciphertext_len = len;

        if(1 != EVP_EncryptFinal_ex(&ctx, (unsigned char *) &(*rbOutput)[0] + len, &len)){
            break;
        }
        ciphertext_len += len;
        rbOutput->resize(ciphertext_len);
        ret=CRYPT_OK;
    }while(0);

    /* Clean up */
    OPENSSL_cleanse(key,sizeof(key));
    OPENSSL_cleanse(iv,sizeof(iv));
    EVP_CIPHER_CTX_cleanup(&ctx);
    return ret;
}

int AESDecrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* sOutput,
               const char* pKey, unsigned int uiKeyLen) {
    if (!pInput || !pKey || !sOutput || uiInputLen < 16 || uiInputLen % 16 != 0)
    {
        return CRYPT_ERR_INVALID_PARAM;
    }

    //EVP 接口会保证使用平台特有的密码学加速，例如AES-NI
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_CIPHER_CTX_set_padding(&ctx,1);//使用pkcs padding

    unsigned char key[16], iv[16]; //aes_128的 key size=128bit , iv size=128bit
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));
    memcpy(key, pKey, uiKeyLen > 16U ? 16U : uiKeyLen);
    memcpy(iv, key, sizeof(key));
    bool ret=CRYPT_ERR_NO_MEMORY;
    do{
        if(1 != EVP_DecryptInit_ex(&ctx, cipher, NULL, key, iv)){
            break;
        }

        sOutput->assign(uiInputLen + EVP_CIPHER_block_size(cipher),0);
        int len=sOutput->size();
        int plaintext_len=0;

        if(1 != EVP_DecryptUpdate(&ctx, (unsigned char *) &(*sOutput)[0], &len, pInput, uiInputLen)){
            break;
        }
        plaintext_len = len;

        if(1 != EVP_DecryptFinal_ex(&ctx, (unsigned char *) &(*sOutput)[0] + len, &len)){
            break;
        }
        plaintext_len += len;
        sOutput->resize(plaintext_len);
        ret=CRYPT_OK;
    }while(0);

    /* Clean up */
    OPENSSL_cleanse(key,sizeof(key));
    OPENSSL_cleanse(iv,sizeof(iv));
    EVP_CIPHER_CTX_cleanup(&ctx);
    return ret;
}

#else

int AESEncrypt(const unsigned char* pInput, unsigned int uiInputLen,std::string* rbOutput,
               const char* pKey, unsigned int uiKeyLen) {
    if (!pInput || !pKey || !rbOutput)
    {
        return CRYPT_ERR_INVALID_PARAM;
    }

    //padding pkcs7
	std::string rbPaddedInput;
    int nPaddingLen = 16 - (uiInputLen % 16);
	rbPaddedInput.assign((const char*)pInput, uiInputLen);
	rbPaddedInput.append(nPaddingLen,(char)nPaddingLen);

	unsigned char *pcData = (unsigned char*)OPENSSL_malloc(uiInputLen + nPaddingLen);
	if( !pcData)
	{
		return CRYPT_ERR_NO_MEMORY;
	}

    unsigned char key[16], iv[16];
    memset(key, 0, sizeof(key));
    memcpy(key, pKey, uiKeyLen > 16U ? 16U : uiKeyLen);
    memcpy(iv, key, sizeof(key));

    AES_KEY aesKey;
    AES_set_encrypt_key(key, 128, &aesKey);
    AES_cbc_encrypt((unsigned char*)rbPaddedInput.data(),
            (unsigned char*)pcData,
            uiInputLen + nPaddingLen, &aesKey, iv, AES_ENCRYPT);
	rbOutput->append((const char*)pcData,uiInputLen + nPaddingLen);
	if( pcData) { OPENSSL_free(pcData); pcData = NULL;}
    return CRYPT_OK;
}

int AESDecrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* sOutput,
               const char* pKey, unsigned int uiKeyLen) {
    if (!pInput || !pKey || uiInputLen < 16 || uiInputLen % 16 != 0)
    {
        return CRYPT_ERR_INVALID_PARAM;
    }
	unsigned char* pcData = (unsigned char*)OPENSSL_malloc(uiInputLen);
	if( !pcData)
	{
		return CRYPT_ERR_NO_MEMORY;
	}

    unsigned char key[16], iv[16];
    memset(key, 0, sizeof(key));
    memcpy(key, pKey, uiKeyLen > 16U ? 16U : uiKeyLen);
    memcpy(iv, key, sizeof(key));

    AES_KEY decKey;
    AES_set_decrypt_key(key, 128, &decKey);

    AES_cbc_encrypt((unsigned char*)pInput,
            (unsigned char*)pcData,
            uiInputLen, &decKey, iv, AES_DECRYPT);
    int nPaddingLen = *(pcData+ uiInputLen-1);
    if (nPaddingLen <= 0 || nPaddingLen > 16)
    {
		if(pcData){ OPENSSL_free(pcData); pcData=NULL;}
        return CRYPT_ERR_INVALID_PARAM;
    }
	sOutput->append((const char*)pcData,uiInputLen- nPaddingLen);
	if(pcData){ OPENSSL_free(pcData); pcData=NULL;}
    return CRYPT_OK;
}
#endif
//int LoadRSAKeyFromString( const string& strKeyString, enRSAKeyType type, EVP_PKEY** Key )
//{
//	BIO* io = BIO_new(BIO_s_mem());
//	int len = BIO_write(io, strKeyString.c_str(), strKeyString.length());
//	if (static_cast<size_t>(len) != strKeyString.length()) {
//		MMERR( "Failed to write key string into BIO struct." );
//		BIO_free(io);
//		return CRYPT_ERR_TRANSFORM_KEY;
//	}
//	RSA* rsa = RSA_new();
//	if( !rsa)
//	{
//		BIO_free(io);
//		return CRYPT_ERR_NO_MEMORY;
//	}
//	if (type == RSA_PRIVATE_KEY) {
//		rsa = PEM_read_bio_RSAPrivateKey(io, &rsa, NULL, NULL);
//	} else {
//		rsa = PEM_read_bio_RSA_PUBKEY(io, &rsa, NULL, NULL);
//	}
//	if (rsa == NULL) {
//		MMERR( "Error reading RSA struct from BIO struct." );
//		BIO_free(io);
//		RSA_free(rsa);
//		return CRYPT_ERR_TRANSFORM_KEY;
//	}
//	*Key = EVP_PKEY_new();
//	int ret = EVP_PKEY_assign_RSA(*Key, rsa);
//	if (ret != 1) {
//		MMERR( "Failed to assign rsa to EVP_PKEY." );
//		EVP_PKEY_free(*Key);
//		*Key = NULL;
//		BIO_free(io);
//		return CRYPT_ERR_TRANSFORM_KEY;
//	}
//	BIO_free(io);
//	return CRYPT_OK;
//}

EVP_PKEY* GenRSAKeyPair( int key_length )
{
	bool status = true;
	BIGNUM* bne = NULL;
	RSA* rkey = NULL;
	EVP_PKEY* pkey = NULL;

	bne = BN_new();
	if (bne == NULL) {
		MMERR( "Error creating BIGNUM" );
		status = false;
		goto cleanup;
	}
	// use 0x10001 as exponent, as client did before
	if ((BN_set_word(bne, RSA_F4)) != 1) {
		MMERR( "Error setting BIGNUM bne" );
		status = false;
		goto cleanup;
	}
	rkey = RSA_new();
	if (rkey == NULL) {
		MMERR( "Error creating RSA key" );
		status = false;
		goto cleanup;
	}
	if ((RSA_generate_key_ex(rkey, key_length, bne, NULL)) != 1) {
		MMERR( "Error creating RSA key pair" );
		status = false;
		RSA_free(rkey),rkey=NULL;
		goto cleanup;
	}
	pkey = EVP_PKEY_new();
	if (pkey == NULL) {
		MMERR( "Error creating EVP_PKEY" );
		status = false;
		RSA_free(rkey),rkey=NULL;
		goto cleanup;
	}
	if ((EVP_PKEY_assign_RSA(pkey, rkey)) != 1) {
		MMERR( "Error setting EVP_PKEY" );
		status = false;
		goto cleanup;
	}

cleanup:
	// rkey is pointed by pkey's struct member, so should not free.
	// EVP_PKEY_free(pkey) will also release rkey
	if (bne != NULL)
		BN_free(bne);
	if (pkey != NULL && status == false)
		EVP_PKEY_free(pkey);
	return pkey;
}

//int GetKeyString( EVP_PKEY* pkey, enRSAKeyType type, string* pkey_str )
//{
//	BIO* io = BIO_new(BIO_s_mem());
//	int ret = 1;
//	if ( type == RSA_PRIVATE_KEY ) {
//		ret = PEM_write_bio_RSAPrivateKey(io, pkey->pkey.rsa, NULL, NULL, 0, NULL, NULL);
//	} else {
//		ret = PEM_write_bio_RSA_PUBKEY(io, pkey->pkey.rsa);
//	}
//	if (ret != 1) {
//		MMERR( "Failed to write key int BIO struct." );
//		BIO_free(io);
//		return -1;
//	}
//	char* out = NULL;
//	int len = BIO_ctrl_pending(io);
//	out = (char*)OPENSSL_malloc(len + 1);
//    out[len] = '\0';
//	int r_len = BIO_read(io, out, len);
//	if (r_len != len) {
//		MMERR( "Error reading string from BIO struct." );
//		OPENSSL_free(out);
//		BIO_free(io);
//		return -1;
//	}
//	*pkey_str = string(out);
//	OPENSSL_free(out);
//	BIO_free(io);
//	return 0;
//}

int GetPubKeyParms(EVP_PKEY* pkey, std::string* modulus, std::string* exponent) {
    RSA* rsa = EVP_PKEY_get1_RSA(pkey);
    if (rsa == NULL) {
        return -1;
    }
    char* tmp = NULL;
    tmp = BN_bn2hex(rsa->n);
    if (tmp == NULL) {
        return -1;
    }
    *modulus = string(tmp);
    OPENSSL_free(tmp);
    tmp = BN_bn2hex(rsa->e);
    if (tmp == NULL) {
        return -1;
    }
    *exponent = string(tmp);
    OPENSSL_free(tmp);
    return 0;
}

EC_KEY* GenerateECKey(int nid)
{
	EC_KEY *ptKey = EC_KEY_new_by_curve_name(nid); 
	if( !ptKey )
	{
		MMERR("ERR %s new key by curve name fail",__func__);
		return NULL;
	}
	int ret = EC_KEY_generate_key(ptKey);
	if( !ret)
	{
		//ERR_print_errors_fp(stderr);
		MMERR("ERR %s generate key fail",__func__);
		EC_KEY_free(ptKey);
		return  NULL;
	}
	return  ptKey;
}

int ECPubKeyToString(EC_KEY* pPriKey, std::string* pKeyString)
{
	if(!pKeyString)
	{
		MMERR("ERR %s invalid",__func__);
		return CRYPT_ERR_INVALID_PARAM;
	}
	int ret = 0;
	int r_len = 0;
    unsigned char* out =NULL;
	r_len = i2o_ECPublicKey(pPriKey,&out);
	if( r_len == 0 || ( !out))
	{
	   MMERR("ERR %s seralize key fail.ret %d len %d",__func__,ret,r_len);
	   if(out) { OPENSSL_free(out),out=NULL;}
	   return CRYPT_ERR_INVALID_PARAM;
	}
	pKeyString->assign((const char*)out,r_len);
	if(out) { OPENSSL_free(out),out=NULL;}
	return CRYPT_OK;
}

int ECPriKeyToString(EC_KEY* pPriKey, std::string* pKeyString)
{
	if(!pKeyString)
	{
		MMERR("ERR %s invalid",__func__);
		return CRYPT_ERR_INVALID_PARAM;
	}
	int ret = 0;
	int r_len = 0;
    unsigned char* out =NULL;
    r_len = i2d_ECPrivateKey(pPriKey,&out);
	if( r_len == 0 || (!out))
	{
	   MMERR("ERR %s seralize key fail.ret %d len %d",__func__,ret,r_len);
	   if(out) { OPENSSL_free(out),out=NULL;}
	   return CRYPT_ERR_INVALID_PARAM;
	}
	pKeyString->assign((const char*)out,r_len);
	if(out) { OPENSSL_free(out),out=NULL;}
	return CRYPT_OK;
}

//int ECNewKeyToString(EC_KEY* pPriKey, enECKeyType eType,  std::string* pKeyString)
//{
//	if(!pKeyString)
//	{
//		MMERR("ERR %s invalid",__func__);
//		return CRYPT_ERR_INVALID_PARAM;
//	}
//	BIO *io = BIO_new(BIO_s_mem());
//	if( !io)
//	{
//		MMERR("ERR %s bio new fail",__func__);
//		return CRYPT_ERR_NO_MEMORY;
//	}
//	int ret = 0;
//	if( eEC_PRIVATE_KEY ==  eType)
//	{
//	   ret = i2d_ECPrivateKey_bio(io,pPriKey);
//	}else if( eEC_PUBLIC_KEY == eType)
//	{
//	   ret = i2d_EC_PUBKEY_bio(io,pPriKey);
//	}else
//	{
//		MMERR("ERR %s unexpect type %d",__func__,eType);
//		BIO_free(io);
//		return CRYPT_ERR_INVALID_PARAM;
//	}
//	if( !ret)
//	{
//		MMERR("ERR %s key to type %u fail.",__func__,eType);
//		BIO_free(io);
//		return CRYPT_ERR_TRANSFORM_KEY;
//	}
//
//	char *out = NULL;
//	int len = BIO_ctrl_pending(io);
//	if( len <= 0 )
//	{
//		MMERR("ERR %s read fail.len %d",__func__,len);
//		BIO_free(io);
//		return CRYPT_ERR_BIO_FAIL;
//	}
//	out = (char*)OPENSSL_malloc(len);
//	if( !out)
//	{
//		MMERR("ERR %s malloc fail",__func__);
//		BIO_free(io);
//		return CRYPT_ERR_NO_MEMORY;
//	}
//	int r_len = BIO_read(io,out,len);
//	if( r_len != len)
//	{
//		MMERR("ERR %s read from bio fail.len %d rlen %d",__func__,len,r_len);
//		OPENSSL_free(out);
//		BIO_free(io);
//		return CRYPT_ERR_BIO_FAIL;
//	}
//	pKeyString->assign(out,len);
//	OPENSSL_free(out);
//	BIO_free(io);
//	return CRYPT_OK;
//}

int ECKeyToString(EC_KEY* pPriKey, enECKeyType eType,  std::string* pKeyString)
{
	if( eEC_PRIVATE_KEY == eType)
	{
		 return ECPriKeyToString(pPriKey, pKeyString);
	}else
	{
		 return ECPubKeyToString(pPriKey,  pKeyString);
		//return ECNewKeyToString(pPriKey, eType,  pKeyString);
	}
}


int ComputeECDHKey(int nid,const std::string& sPubKey, EC_KEY* pPriKey, std::string* pEcdhKey, const enECDH_KDF_TYPE eKdfType /*=eECDH_KDF_MD5*/ )
{
	EC_KEY *pPubKey = NULL;
	int ret = LoadECPubKeyFromString(sPubKey,nid,&pPubKey);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s load pubkey fail.ret %d",__func__,ret);
		return ret;
	}
	ret = ComputeECDHKey(pPubKey,pPriKey,pEcdhKey,eKdfType);
	if( pPubKey ) { EC_KEY_free( pPubKey);pPubKey= NULL;}
	return ret;
}

int ComputeECDHKey(const EC_KEY* pPubKey, EC_KEY* pPriKey, std::string* pEcdhKey, const enECDH_KDF_TYPE eKdfType /*=eECDH_KDF_MD5*/ )
{
	if( pEcdhKey == NULL)
	{
		MMERR("ERR %s invalid",__func__);
		return CRYPT_ERR_INVALID_PARAM;
	}

    unsigned char *aout =  (unsigned char*)OPENSSL_malloc( __kdf_list[eKdfType].iLen);
	if( !aout)
	{
		MMERR("ERR %s malloc fail",__func__);
		return CRYPT_ERR_NO_MEMORY;
	}
    int keylen =ECDH_compute_key(aout,__kdf_list[eKdfType].iLen,EC_KEY_get0_public_key(pPubKey),pPriKey,__kdf_list[eKdfType].pFunc);
	if( keylen != __kdf_list[eKdfType].iLen)
	{
		MMERR("ERR %s computekey fail.keylen %d expectlen %d",__func__,keylen,__kdf_list[eKdfType].iLen);
		OPENSSL_free(aout);
		return CRYPT_ERR_GEN_KEY;
	}
	pEcdhKey->assign((const char*)aout,keylen);
	OPENSSL_free(aout);
	return CRYPT_OK;
}

int LoadECPriKeyFromString(const std::string& sKeyString,EC_KEY** ppPriKey)
{
	if( !ppPriKey)
	{
		MMERR("ERR %s prikey point null",__func__);
		return CRYPT_ERR_INVALID_PARAM;
	}
	EC_KEY *pPriKey = *ppPriKey;
	if( pPriKey )
	{
		EC_KEY_free(pPriKey);
		pPriKey = NULL;
	}
	const unsigned char* pcKey = (const unsigned char*)sKeyString.c_str();
	*ppPriKey = d2i_ECPrivateKey(ppPriKey,&pcKey,sKeyString.length());
	if( !(*ppPriKey))
	{
		MMERR("ERR %s fail fail",__func__);
		return CRYPT_ERR_TRANSFORM_KEY;
	}
	return CRYPT_OK;
}

int LoadECPubKeyFromString(const std::string& sKeyString,int nid, EC_KEY** ppPriKey)
{
	if( !ppPriKey)
	{
		MMERR("ERR %s prikey point null",__func__);
		return CRYPT_ERR_INVALID_PARAM;
	}
	EC_KEY *pPriKey = *ppPriKey;
	if( pPriKey )
	{
		EC_KEY_free(pPriKey);
		pPriKey = NULL;
	}
	*ppPriKey = EC_KEY_new_by_curve_name(nid);
	const unsigned char* pcKey = (const unsigned char*)sKeyString.data();
	*ppPriKey = o2i_ECPublicKey(ppPriKey,&pcKey,sKeyString.length());
	if( !(*ppPriKey))
	{
		MMERR("ERR %s fail fail",__func__);
		if( *ppPriKey) { EC_KEY_free( *ppPriKey);*ppPriKey = NULL;}
		return CRYPT_ERR_TRANSFORM_KEY;
	}
	return CRYPT_OK;
}

void Free_EC_KEY(EC_KEY**ppKey)
{
	if(ppKey && *ppKey )
	{
		EC_KEY_free(*ppKey);
		*ppKey = NULL;
	}
}

int ComputerEcdhKeyWrapper(int nid,const std::string& sPubKey1, std::string* sPubKey2, std::string* sPriKey2, std::string* sEcdhKey)
{
	EC_KEY* pCliPubKey= NULL;
	int ret = Comm::LoadECPubKeyFromString(sPubKey1,nid,&pCliPubKey);
	if( CRYPT_OK != ret || !pCliPubKey )
	{
		MMERR("ERR %s load ec pubkey fail.nid %u ret %d",__func__,nid,ret);
		Free_EC_KEY(&pCliPubKey);
		return ret;
	}

	EC_KEY* pPriKey2 = GenerateECKey(nid);
	if( !pPriKey2 )
	{
		MMERR("ERR %s generater ec key fail. nid %d",__func__,nid);
		Free_EC_KEY(&pCliPubKey);
		return CRYPT_ERR_INVALID_PARAM;
	}

	ret = ComputeECDHKey(pCliPubKey, pPriKey2, sEcdhKey, eECDH_KDF_MD5);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s computer ecdh key fail.ret %d",__func__,ret);
		Free_EC_KEY(&pCliPubKey);
		Free_EC_KEY(&pPriKey2);
		return ret;
	}
	ret = ECPriKeyToString(pPriKey2,sPriKey2);
	if( CRYPT_OK != ret)
	{
		MMERR("ERR %s pri key to string fail.ret %d",__func__,ret);
		Free_EC_KEY(&pCliPubKey);
		Free_EC_KEY(&pPriKey2);
		return ret;
	}
	ret = ECPubKeyToString(pPriKey2,sPubKey2);
	if( CRYPT_OK != ret)
	{
		Free_EC_KEY(&pCliPubKey);
		Free_EC_KEY(&pPriKey2);
		MMERR("ERR %s pub key to string fail.ret %d",__func__,ret);
		return ret;
	}
	Free_EC_KEY(&pCliPubKey);
	Free_EC_KEY(&pPriKey2);
	return CRYPT_OK;
}

unsigned int GenSignature(unsigned int iUin, const std::string &sEcdhKey, const unsigned char *pcBuff, unsigned int iBuffLen)
{
	unsigned int iSeed;
	unsigned char pcOut[MD5_DIGEST_LENGTH + 1];

	// the first md5
	MD5_CTX oCtx1;
	MD5_Init(&oCtx1);

	iSeed = htonl(iUin);
	MD5_Update(&oCtx1, (unsigned char *)&iSeed, sizeof(iSeed));
	MD5_Update(&oCtx1, (unsigned char *)sEcdhKey.c_str(), sEcdhKey.size());
	MD5_Final(pcOut, &oCtx1);

	// the second md5
	MD5_CTX oCtx2;
	MD5_Init(&oCtx2);

	iSeed = htonl(iBuffLen);
	MD5_Update(&oCtx2, (unsigned char *)&iSeed, sizeof(iSeed));
	MD5_Update(&oCtx2, (unsigned char *)sEcdhKey.c_str(), sEcdhKey.size());
	MD5_Update(&oCtx2, pcOut, MD5_DIGEST_LENGTH);
	MD5_Final(pcOut, &oCtx2);

	// adler32
	unsigned long adler = adler32(0L, Z_NULL, 0);
	adler = adler32(adler, pcOut, MD5_DIGEST_LENGTH);
	adler = adler32(adler, pcBuff, iBuffLen);

	return (unsigned int)adler;
}

void GenKVRes(unsigned int iUin,const char* pcKey, const std::vector<std::string>& vValue, std::string& sRes)
{
	if( !pcKey || iUin == 0 )
	{
		sRes.clear();
		return;
	}
	std::stringstream ss;
	ss<<"<R><M0>1</M0><M1>"<<GenSignature(iUin,pcKey,(const unsigned char*)pcKey,strlen(pcKey))<<"</M1>";
	for(size_t i =0;i< vValue.size();i++)
	{
		unsigned int iV1 = GenSignature(iUin,vValue[i],(const unsigned char*)pcKey,strlen(pcKey));
		ss<<"<M"<<i+2<<">"<<iV1<<"</M"<<i+2<<">";
	}
	ss<<"</R>";
	sRes.assign(ss.str());
}


}


//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif



#ifndef WIN32
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used)) = "$HeadURL: http://scm-gy/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/comm2/core/crypto/iCoreCrypt.cpp $ $Id: iCoreCrypt.cpp 1111211 2015-04-09 07:19:18Z byronhe $ " GZRD_SVN_ATTR "__file__";

#else
static char gzrd_Lib_CPP_Version_ID[] = "$HeadURL: http://scm-gy/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/comm2/core/crypto/iCoreCrypt.cpp $ $Id: iCoreCrypt.cpp 1111211 2015-04-09 07:19:18Z byronhe $ " GZRD_SVN_ATTR "__file__";

#endif

// gzrd_Lib_CPP_Version_ID--end

