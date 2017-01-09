/*
 * ecdh_crypt.h
 *
 *  Created on: 2013-11-20
 *      Author: renlibin
 *	Description:For ECDH export interfaces
 */

#ifndef ECDH_CRYPT_H_
#define ECDH_CRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ECDH_ECCHANGE_KEY_BUFFER_LEN (133) // For exchanged key buffer size (unsigned char)
#define ECDH_DH_KEY_LEN (20)		  	   // For DH key outBuf size (unsigned char)

struct ec_key_st;
typedef struct ec_key_st EC_KEY;

/*
 * @Return: ecdh key pair including private key and public key.
 */
EC_KEY* ecdh_new_random_keypair();

/*
 * Free ECDH Key
 */
void ecdh_free_random_keypair(EC_KEY* ecKey);

/*
 * @Params:	exchangekeybuffer is the buffer of exchangedkey in char
 * 			outBuf is the common key buffer
 * 			randomKeyPair is the EC Key that provides private key
 * @Return  Return (>1) means success, otherwise failed.
 */
int ecdh_compute_dh_key(const unsigned char *exchangekeybuffer, unsigned char *outBuf, EC_KEY* randomKeyPair);

/*
 * Get public in char form
 * @Parems outBuf store public key in char form
 * @Return Return (>1) means success, otherwise, failed.
 */
int ecdh_get_exchange_keybuffer(EC_KEY *key, unsigned char *exchangekeybuffer);

#ifdef __cplusplus
}
#endif

#endif /* ECDH_CRYPT_H_ */
