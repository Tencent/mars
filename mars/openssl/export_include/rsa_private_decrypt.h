/*
 * rsa_encrypt.h
 *
 *  Created on: 2014-9-9
 *      Author: yerungui
 */

#ifndef RSA_DECRYPT_H_
#define RSA_DECRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef struct rsa_st RSA;

RSA*  		rsa_keypair_gen(int key_length);
int 		ras_keypair_string2key(const char* _keystring, RSA** _key);
char* 		rsa_keypair_key2string(RSA* _pkey);

void 		ras_keypair_freekey(RSA* _key);
void 		ras_keypair_freestring(char* _keystring);

int 		rsa_private_decrypt(unsigned char** _out, unsigned int* _outlen,  const unsigned char* _in, unsigned int _inlen, RSA* _key);

#ifdef __cplusplus
}
#endif

#endif /* RSA_ENCRYPT_H_ */
