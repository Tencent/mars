/*
 * aes_crypt.c
 *
 *  Created on: 2013-7-17
 *      Author: zhouzhijie
 */

//#include "aes_crypt.h"
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>

#define AES_KEY_LEN 16
#define AES_KEY_BITSET_LEN 128

int aes_cbc_encrypt(const unsigned char* pKey, unsigned int uiKeyLen
		, const unsigned char* pInput, unsigned int uiInputLen
		, unsigned char** ppOutput, unsigned int* pOutputLen)
{
	unsigned char keyBuf[AES_KEY_LEN] = {0};
	unsigned char iv[AES_KEY_LEN];

    AES_KEY aesKey;
    int ret;

    unsigned int uiPaddingLen;
    unsigned int uiTotalLen;
    unsigned char* pData;

	if(pKey == NULL || uiKeyLen == 0 || pInput == NULL || uiInputLen == 0
	   || pOutputLen == NULL || ppOutput == NULL)
		return -1;

	memcpy(keyBuf, pKey, (uiKeyLen > AES_KEY_LEN) ? AES_KEY_LEN : uiKeyLen);
	memcpy(iv, keyBuf, AES_KEY_LEN);

    ret = AES_set_encrypt_key(keyBuf, AES_KEY_BITSET_LEN, &aesKey);
    if(ret != 0) return -2;

	//padding
	uiPaddingLen = AES_KEY_LEN - (uiInputLen % AES_KEY_LEN);
	uiTotalLen = uiInputLen + uiPaddingLen;
	pData = malloc(uiTotalLen);
	memcpy(pData, pInput, uiInputLen);

	if(uiPaddingLen > 0) memset(pData+uiInputLen, uiPaddingLen, uiPaddingLen);

    *pOutputLen = uiTotalLen;
	*ppOutput = malloc(uiTotalLen);
	memset(*ppOutput, 0, uiTotalLen);

    AES_cbc_encrypt(pData, *ppOutput, uiTotalLen, &aesKey, iv, AES_ENCRYPT);

	free(pData);
	return 0;
}

int aes_cbc_decrypt(const unsigned char* pKey, unsigned int uiKeyLen
		, const unsigned char* pInput, unsigned int uiInputLen
		, unsigned char** ppOutput, unsigned int* pOutputLen)
{

	unsigned char keyBuf[AES_KEY_LEN] = {0};
	unsigned char iv[AES_KEY_LEN];

    AES_KEY aesKey;
    int ret;
    int uiPaddingLen;

	if(pKey == NULL || uiKeyLen == 0 || pInput == NULL || uiInputLen == 0 || pOutputLen == NULL
			|| (uiInputLen%AES_KEY_LEN) != 0 || ppOutput == NULL)
		return -1;

	memcpy(keyBuf, pKey, (uiKeyLen > AES_KEY_LEN) ? AES_KEY_LEN : uiKeyLen);
	memcpy(iv, keyBuf, AES_KEY_LEN);

    ret = AES_set_decrypt_key(keyBuf, AES_KEY_BITSET_LEN, &aesKey);
    if(ret != 0) { return -2; }

	*ppOutput = malloc(uiInputLen);
	memset(*ppOutput, 0, uiInputLen);

    AES_cbc_encrypt(pInput, *ppOutput, uiInputLen, &aesKey, iv, AES_DECRYPT);

    uiPaddingLen = (*ppOutput)[uiInputLen - 1];
    if(uiPaddingLen > AES_KEY_LEN || uiPaddingLen <= 0) {
    	free(*ppOutput);
    	ppOutput = NULL;
    	return -3;
    }

    *pOutputLen = uiInputLen - uiPaddingLen;

	return 0;
}

int aes_ecb_encrypt(const unsigned char* pKey, unsigned int uiKeyLen
		, const unsigned char* pInput, unsigned int uiInputLen, int bFinal
		, unsigned char** ppOutput, unsigned int* pOutputLen)
{

	unsigned char keyBuf[AES_KEY_LEN] = {0};

    AES_KEY aesKey;
    int ret;

	unsigned int uiPaddingLen;
	unsigned int uiTotalLen;
	unsigned char* pData;

    unsigned int uiDone;
    unsigned char* pcInput;
    unsigned char* pcOutput;

	if(pKey == NULL || uiKeyLen == 0 || pInput == NULL || uiInputLen == 0 || pOutputLen == NULL
	   || (bFinal==0 && (uiInputLen%AES_KEY_LEN) != 0) || ppOutput == NULL)
		return -1;

	memcpy(keyBuf, pKey, (uiKeyLen > AES_KEY_LEN) ? AES_KEY_LEN : uiKeyLen);
    ret = AES_set_encrypt_key(keyBuf, AES_KEY_BITSET_LEN, &aesKey);

    if(ret != 0) { return -2;}

	//padding
	uiPaddingLen = (bFinal!=0) ? (AES_KEY_LEN - (uiInputLen % AES_KEY_LEN)) : 0;
	uiTotalLen = uiInputLen + uiPaddingLen;
	pData = malloc(uiTotalLen);
	memcpy(pData, pInput, uiInputLen);
	if(uiPaddingLen > 0) memset(pData+uiInputLen, uiPaddingLen, uiPaddingLen);

	*ppOutput = malloc(uiTotalLen);
	memset(*ppOutput, 0, uiTotalLen);

    uiDone = 0;
    pcInput = pData;
    pcOutput = *ppOutput;

    while(uiDone < uiTotalLen) {
    	AES_ecb_encrypt(pcInput, pcOutput, &aesKey, AES_ENCRYPT);
    	pcInput += AES_KEY_LEN;
    	pcOutput += AES_KEY_LEN;
    	uiDone += AES_KEY_LEN;
    }

    *pOutputLen = uiTotalLen;
	free(pData);
	return 0;
}


int aes_ecb_decrypt(const unsigned char* pKey, unsigned int uiKeyLen
		, const unsigned char* pInput, unsigned int uiInputLen, int bFinal
		, unsigned char** ppOutput, unsigned int* pOutputLen)
{
	unsigned char keyBuf[AES_KEY_LEN] = {0};

    AES_KEY aesKey;
    unsigned int uiDone;
    const unsigned char* pcInput;
    unsigned char* pcOutput;
    unsigned int uiPaddingLen;
	int ret;

	if(pKey == NULL || uiKeyLen == 0 || pInput == NULL || uiInputLen == 0 || pOutputLen == NULL
			|| (uiInputLen%AES_KEY_LEN) != 0 || ppOutput == NULL)
		return -1;

	memcpy(keyBuf, pKey, (uiKeyLen > AES_KEY_LEN) ? AES_KEY_LEN : uiKeyLen);

    ret = AES_set_decrypt_key(keyBuf, AES_KEY_BITSET_LEN, &aesKey);
    if(ret != 0) { return -2;}

	*ppOutput = malloc(uiInputLen);
	memset(*ppOutput, 0, uiInputLen);

    uiDone = 0;
    pcInput = pInput;
    pcOutput = *ppOutput;

    while(uiDone < uiInputLen) {
    	AES_ecb_encrypt(pcInput, pcOutput, &aesKey, AES_DECRYPT);
    	pcInput += AES_KEY_LEN;
    	pcOutput += AES_KEY_LEN;
    	uiDone += AES_KEY_LEN;
    }

    if(bFinal != 0)
    {
		uiPaddingLen = (*ppOutput)[uiInputLen - 1];
		if(uiPaddingLen > AES_KEY_LEN || uiPaddingLen <= 0) {
			free(*ppOutput);
			ppOutput = NULL;
			return -3;
		}

		*pOutputLen = uiInputLen - uiPaddingLen;
    }
    else
    	*pOutputLen = uiInputLen;

	return 0;
}
