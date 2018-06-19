/*
 * GenRsaKeyPair.h
 *
 *  Created on: 2016年3月21日
 *      Author: wutianqiang
 */

#ifndef EXPORT_CRYPTO_GENRSAKEYPAIR_H_
#define EXPORT_CRYPTO_GENRSAKEYPAIR_H_

#ifdef __cplusplus
extern "C"{
#endif
#include <stddef.h>
typedef enum {
	kBufNotEnough,
    kOK,

} GenRsaKeyResult;

typedef enum{
    kKey512Bits = 512,
    kKey1024Bits= 1024,
    kKey2048Bits= 2048,
}RSAKeyBits;

GenRsaKeyResult generate_rsa_key_pair(char* _pem_public_key_buf, const size_t _public_key_buf_len,
								char* _pem_private_key_buf, const size_t _private_key_buf_len, RSAKeyBits _key_bits=kKey1024Bits);


//Deprecated, use generate_rsa_key_pair instead
GenRsaKeyResult generate_rsa_key_pair_2048(char* _pem_public_key_buf, const size_t _public_key_buf_len,
                                               char* _pem_private_key_buf, const size_t _private_key_buf_len);

#ifdef __cplusplus
}
#endif



#endif /* EXPORT_CRYPTO_GENRSAKEYPAIR_H_ */
