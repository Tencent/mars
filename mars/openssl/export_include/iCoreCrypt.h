/*
 *  iCoreCrypt.h
 *  此文件仅给客户端使用，不能含有其它依赖，只能依赖于openssl
 *  测试代码见 http://scm-gy/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/comm2/core/crypto/test/
 *
 *  Created by austindeng on 14-11-5.
 *  Copyright 2014 Tencent. All rights reserved.
 *
 */
#pragma once

#include <string>
#include <string.h>
#include <stdint.h>
#include<vector>

struct evp_pkey_st;
struct ec_key_st;

typedef struct evp_pkey_st EVP_PKEY;
typedef struct ec_key_st EC_KEY;

namespace Comm
{

enum
{
	CRYPT_OK = 0,
	CRYPT_ERR_INVALID_KEY_N = 1,
	CRYPT_ERR_INVALID_KEY_E = 2,
	CRYPT_ERR_ENCRYPT_WITH_RSA_PUBKEY = 3,
	CRYPT_ERR_DECRYPT_WITH_RSA_PRIVKEY = 4,
	CRYPT_ERR_NO_MEMORY = 5,
	CRYPT_ERR_ENCRYPT_WITH_DES_KEY = 6,
	CRYPT_ERR_DECRYPT_WITH_DES_KEY = 7,
	CRYPT_ERR_INVALID_PARAM = 8,
	CRYPT_ERR_LOAD_RSA_PRIVATE_KEY = 9,
	CRYPT_ERR_TRANSFORM_KEY = 10,
	CRYPT_ERR_BIO_FAIL = 11,
	CRYPT_ERR_GEN_KEY = 12,
	CRYPT_ERR_COMPRESS = 13,
	CRYPT_ERR_DECOMPRESS = 14,
	CRYPT_ERR_MATCH_LEN = 15,
};

enum enRSAKeyType
{
	RSA_PRIVATE_KEY = 0,
	RSA_PUBLIC_KEY = 1,
};

enum enECKeyType
{
	eEC_PRIVATE_KEY = 0,
	eEC_PUBLIC_KEY = 1,
};

enum enECDH_KDF_TYPE
{
	eECDH_KDF_MD5 = 0,
	eECDH_KDF_SHA1 = 1,
	//增加定义值需要先对应配置__kdf_list,否则会core
};

// use EVP_PKEY struct of private key as parameter
int RSADecrypt( const unsigned char *pInput,
			   unsigned int uiInputLen, 
			   std::string* rbOutput,
			   EVP_PKEY* Key
			   );

// use file path of private key as parameter
int RSADecrypt( const unsigned char *pInput,
			   unsigned int uiInputLen, 
			   std::string* rbOutput,
			   const char *pPrivKeyFile
			   );

int RSAEncrypt(	const unsigned char *pInput, 
				unsigned int uiInputLen, 
				std::string* sOutput,
				const char *pPublicKeyN,
				const char *pPublicKeyE
			  );

int RSAPrivateEncrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* poResult, 
		     EVP_PKEY* pKey);
int RSAPublicDecrypt( const unsigned char* pInput,
			   unsigned int uiInputLen,
			   std::string* sOutput,
			   EVP_PKEY* Key
			   );
int RSAPublicDecrypt( const unsigned char* pInput,
			   unsigned int uiInputLen,
			   std::string* sOutput,
			   const char* pPublicKeyN, const char *pPublicKeyE);

//aes-cbc 128 pkcs7 加解密
int AESEncrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* rbOutput,
               const char* pKey, unsigned int uiKeyLen);

int AESDecrypt(const unsigned char* pInput, unsigned int uiInputLen, std::string* rbOutput,
               const char* pKey, unsigned int uiKeyLen);

// used to generate openssl key struct from string, type is either "private" or "public"
int LoadRSAKeyFromString(const std::string& KeyString, enRSAKeyType type, EVP_PKEY** key);

// return: a EVP_PKEY structure containing the RSA private key
// (in openssl: private key struct contain both private key and pubkey info, public key can be gotten from private key)  
EVP_PKEY* GenRSAKeyPair( int key_length );

// get public key string from either EVP_PKEY struct of private key or public key
int GetKeyString(EVP_PKEY* pub_key, enRSAKeyType type, std::string* pub_key_str);

// get exponent and modulus of the public key in decimal string format
int GetPubKeyParms(EVP_PKEY* pub_key, std::string* modulus, std::string* exponent);


/**
 * @note
 *   Creates a new EC_KEY object using a named curve as underlying EC_GROUP object
 * @param
 *   nid nid	NID of the named curve.
 * @Returns:
 *    succ: EC_KEY object 
 *    NULL if an error occurred.
 */
EC_KEY* GenerateECKey(int nid);

/*
 * @note 
 * 		Encodes a private key object and stores the result in a keystring 
 * @praram
 *    prKey	the EC_KEY object to encode
 *    pKeyString encode string
 * @returns
 *    0 : succ
 *    else : false
 */
int ECPriKeyToString(EC_KEY* pPriKey, std::string* pKeyString);

/*
 * @note 
 * 		Encodes a public key object and stores the result in a keystring 
 * @praram
 *    prKey	the EC_KEY object to encode
 *    pKeyString encode string
 * @returns
 *    0 : succ
 *    else : false
 */
int ECPubKeyToString(EC_KEY* pPubKey, std::string* pKeyString);

/*
 * @note
 *     Decodes a ec public key from a string.
 * @param
 *    sKeyString : encode buffer
 *    nid  ec curve name
 *    key	a pointer to a EC_KEY object which should be used
 * @returns
 *    0 : succ
 *    else : false
 *
 */
int LoadECPubKeyFromString(const std::string& sKeyString,int nid, EC_KEY** ppPubKey);

/*
 * @note
 *     Decodes a ec private key from a string.
 * @param
 *    sKeyString : encode buffer
 *    nid  ec curve name
 *    key	a pointer to a EC_KEY object which should be used
 * @returns
 *    0 : succ
 *    else : false
 *
 */
int LoadECPriKeyFromString(const std::string& sKeyString,EC_KEY** ppPriKey);


/*
*  @note 
*     computer ecdh key
* @param 
*     pPubKey : public ec key from peer
*     pPriKey : private ec key
*     pEcdhKey: output ecdh key
*     eKdfType: edf method, default by md5
 * @returns
 *    0 : succ
 *    else : false
*/
int ComputeECDHKey(const EC_KEY* pPubKey, EC_KEY* pPriKey, std::string* pEcdhKey, const enECDH_KDF_TYPE eKdfType   = eECDH_KDF_MD5);

/*
*  @note 
*     computer ecdh key
* @param 
*     nid     : ec curve name
*     sPubKey : public ec key string from peer
*     pPriKey : private ec key
*     pEcdhKey: output ecdh key
*     eKdfType: edf method, default by md5
* @returns
*    0 : succ
*    else : false
*/
int ComputeECDHKey(int nid,const std::string& sPubKey, EC_KEY* pPriKey, std::string* pEcdhKey, const enECDH_KDF_TYPE eKdfType =eECDH_KDF_MD5 );

/*
 * @note
 *   compress and  encrypt with RSA and AES. AES key store in RSA plain data
 * @param
 *    pRsaInput:         rsa plain data 
 *    uiRsaInputLen: 	 rsa plain data length
 *    pAesInput      :   aes plain data. aes key need by random key buffer
 *    uiAesInputLen  :   aes plain data length
 *    pPublicKeyN    :   rsa public key n
 *    pPublicKeyE    :   rsa public key e
 *    pKey		     :   aes key
 *    uiKeyLen       :   aes key len
 *    pOutput       :   encrypt data buffer
 */
int HybridEncrypt(const unsigned char* pRsaInput, unsigned int uiRsaInputLen,
		const unsigned char *pAesInput,unsigned int uiAesInputLen,
		const char* pPublicKeyN, const char *pPublicKeyE,
		const char* pKey, unsigned int uiKeyLen,
		std::string* pOutput);



class RsaDecrypter
{
	public:
		RsaDecrypter(){};
		virtual ~RsaDecrypter(){}
		virtual int Decrypt(const unsigned char* pInput, unsigned int uiInputLen,std::string* pRsaDeOutput) = 0;
};


#if !WIN32
struct HybridPackBodyHead_t
{
	uint32_t RsaRawLen;
	uint32_t AesRawLen;
	uint32_t RsaBufLen;
	void ntoh(const HybridPackBodyHead_t* pSrc);
	void hton(const HybridPackBodyHead_t* pSrc);
}__attribute__((packed));
#else
#pragma pack(push, 1)
struct HybridPackBodyHead_t
{
	uint32_t RsaRawLen;
	uint32_t AesRawLen;
	uint32_t RsaBufLen;
	void ntoh(const HybridPackBodyHead_t* pSrc);
	void hton(const HybridPackBodyHead_t* pSrc);
};
#pragma pack(pop)
#endif

#if !WIN32
struct DoubleHybridPackBodyHead_t
{
	uint32_t RsaRawLen;
	uint32_t AesRawLen;
	uint32_t RsaBufLen;
	uint32_t RsaBufWithAesLen;
	void ntoh(const DoubleHybridPackBodyHead_t* pSrc);
	void hton(const DoubleHybridPackBodyHead_t* pSrc);
}__attribute__((packed));
#else
#pragma pack(push, 1)
struct DoubleHybridPackBodyHead_t
{
	uint32_t RsaRawLen;
	uint32_t AesRawLen;
	uint32_t RsaBufLen;
	uint32_t RsaBufWithAesLen;
	void ntoh(const DoubleHybridPackBodyHead_t* pSrc);
	void hton(const DoubleHybridPackBodyHead_t* pSrc);
};
#pragma pack(pop)
#endif




/*
 * @note
 *   uncompress and decrypt with RSA and AES. AES key store in RSA plain data
 * @param
 *    pInput         :   hybrid encrypt data
 *    uiInputLen     : 	 hybrid encrypt data len
 *    pDecrypter :   rsa decrypter interface 
 *    pRsaDeOutput  :   rsa plain data decrypted from pInput
 *	  pAesEnOutput  :   aes encrypt data, which need to decrypt by aes key stored in rsa plain data
 *    pHeadOutput    :   hybrid data head 
 */
int HybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   RsaDecrypter* pDecrypter,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   HybridPackBodyHead_t* pHeadOutput);


/*
 * @note
 *   uncompress and decrypt with RSA and AES. AES key store in RSA plain data
 *   only used for decrypt buffer generated by HybridEncrypt
 * @param
 *    pInput         :   hybrid encrypt data
 *    uiInputLen     : 	 hybrid encrypt data len
 *    Key            :   rsa private key
 *    pRsaDeOutput  :   rsa plain data decrypted from pInput
 *	  pAesEnOutput  :   aes encrypt data, which need to decrypt by aes key stored in rsa plain data
 *    pHeadOutput    :   hybrid data head 
 */
int HybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   EVP_PKEY* Key,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   HybridPackBodyHead_t* pHeadOutput);
/*
 *@note
 *	decrypt and uncompress hybridaseencrypt data which generator from HybridDecrypt
 *
 * @param
 *    pInput         :   hybrid encrypt data
 *    uiInputLen     : 	 hybrid encrypt data len
 *    pKey		     :   aes key
 *    uiKeyLen       :   aes key len
 *	  iAesRawLen     :   aes raw len, used for check data
 * 	  pOutput       :    plain data
 */
int HybridAesDecrypt(const unsigned char* pInput, unsigned int uiInputLen
		,const char* pKey, unsigned int uiKeyLen,unsigned int iAesRawLen
		,std::string* pOutput);

/*
 * @note
 *   compress and  encrypt with RSA and AES. AES key store in RSA plain data
 *   rsa buffer will encrypt twice with rsa and aes key.
 * @param
 *    pRsaInput:         rsa plain data 
 *    uiRsaInputLen: 	 rsa plain data length
 *    pAesInput      :   aes plain data.
 *    uiAesInputLen  :   aes plain data length
 *    pPublicKeyN    :   rsa public key n
 *    pPublicKeyE    :   rsa public key e
 *    pKey		     :   aes key. aes key is generated from aat
 *    uiKeyLen       :   aes key len
 *    pOutput       :   encrypt data buffer
 */
int DoubleHybridEncrypt(const unsigned char* pRsaInput, unsigned int uiRsaInputLen,
		const unsigned char *pAesInput,unsigned int uiAesInputLen,
		const char* pPublicKeyN, const char *pPublicKeyE,
		const char* pKey, unsigned int uiKeyLen,
		std::string* pOutput);

/*
 * @note
 *   uncompress and decrypt with RSA and AES. AES key store in RSA plain data
 *   only used for decrypt buffer generated by DoubleHybridEncrypt
 * @param
 *    pInput        :   hybrid encrypt data
 *    uiInputLen    : 	 hybrid encrypt data len
 *    pKey          :  aes key
 *    pRsaDeOutput  :   rsa plain data decrypted from Rsa Buffer Segment
 *	  pAesDeOutput  :   aes plain data decrypted from Aes Buffer Segment
 *    pHeadOutput   :   double hybrid data head. the element aes raw data len,which is used to valid aes data
 */
int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
               const char* pKey, unsigned int uiKeyLen,
			   std::string* pRsaDeOutput,
			   std::string* pAesDeOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput);
/*
 * @note
 *   uncompress and decrypt with RSA and AES. AES key store in RSA plain data
 *   only used for decrypt buffer generated by DoubleHybridEncrypt
 * @param
 *    pInput         :   hybrid encrypt data
 *    uiInputLen     : 	 hybrid encrypt data len
 *    Key            :   rsa private key
 *    pRsaDeOutput  :   rsa plain data decrypted from pInput
 *	  pAesEnOutput  :   aes encrypt data, which need to decrypt by aes key stored in rsa plain data with function HybridAesDecrypt
 *    pHeadOutput   :   double hybrid data head. the element aes raw data len,which is used to valid aes data
 */
int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   RsaDecrypter* pDecrypter,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput);

/*
 * @note
 *   uncompress and decrypt with RSA and AES. AES key store in RSA plain data
 *   only used for decrypt buffer generated by DoubleHybridEncrypt
 * @param
 *    pInput         :   hybrid encrypt data
 *    uiInputLen     : 	 hybrid encrypt data len
 *    Key            :   rsa private key
 *    pRsaDeOutput  :   rsa plain data decrypted from pInput
 *	  pAesEnOutput  :   aes encrypt data, which need to decrypt by aes key stored in rsa plain data with function HybridAesDecrypt
 *    pHeadOutput   :   double hybrid data head. the element aes raw data len,which is used to valid aes data
 */
int DoubleHybridDecrypt(const unsigned char* pInput, unsigned int uiInputLen,
			   EVP_PKEY* Key,
			   std::string* pRsaDeOutput,
			   std::string* pAesEnOutput,
			   DoubleHybridPackBodyHead_t* pHeadOutput);

void Free_EC_KEY(EC_KEY **ppKey);

int ComputerEcdhKeyWrapper(int nid,const std::string& sPubKey1, std::string* sPubKey2, std::string* sPriKey2, std::string* sEcdhKey);

unsigned int GenSignature(unsigned int iUin, const std::string &sEcdhKey, const unsigned char *pcBuff, unsigned int iBuffLen);

void GenKVRes(unsigned int iUin,const char* pcKey, const std::vector<std::string>& vValue, std::string& sRes);

};
