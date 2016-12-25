/*
 * rsa_pss_sha256.h
 *
 *  Created on: 2016年3月18日
 *      Author: wutianqiang
 */

#ifndef EXPORT_CRYPTO_RSA_PSS_SHA256_H_
#define EXPORT_CRYPTO_RSA_PSS_SHA256_H_
#ifdef __cplusplus
extern "C"{
#endif

#include <stddef.h>
bool rsa_pss_sha256_sign(const char* _message, const unsigned long _msg_len,
		const char* _pem_private_key, size_t _private_key_len,
		unsigned char* _signature_buf, size_t& _sig_buf_len);

bool rsa_pss_sha256_verify(const char* _message, const unsigned long _msg_len,
		const char* _pem_public_key, size_t _public_key_len,
		unsigned char* _signature, size_t _sig_len);


#ifdef __cplusplus
}
#endif

#endif /* EXPORT_CRYPTO_RSA_PSS_SHA256_H_ */
