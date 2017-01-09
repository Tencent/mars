/*
 * ecdsa_verify.h
 *
 *  Created on: 2013-5-10
 *      Author: zhouzhijie
 */

#ifndef ECDSA_VERIFY_H_
#define ECDSA_VERIFY_H_

#ifdef __cplusplus
extern "C" {
#endif
int doEcdsaVerify(const unsigned char* _pubkey, int len_key, const char* _content, int len_ctn, const char* _strSig, int len_sig);
int doEcdsaSHAVerify(const unsigned char* _pubkey, int len_key, const char* _content, int len_ctn, const char* _strSig, int len_sig);
#ifdef __cplusplus
}
#endif

#endif /* ECDSA_VERIFY_H_ */
