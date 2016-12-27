/*
 * ecdh_crypt.c
 *
 *  Created on: 2013-11-20
 *      Author: renlibin
 *  Description:Implemention of ECDH exporting interfaces
 */

#include "ecdh_crypt.h"
#include <openssl/ecdh.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <openssl/ec.h>

//static const size_t KDF1_SHA1_LEN = 20;
// curve nid used to generate random key pair.
#define CURVE_NID (NID_secp521r1)

static void *KDF1_SHA1(const void *in, size_t inlen, void *out, size_t *outlen) {
#ifndef OPENSSL_NO_SHA
	if (*outlen < SHA_DIGEST_LENGTH)
		return NULL;
	else
		*outlen = SHA_DIGEST_LENGTH;
	return SHA1(in, inlen, out);
#else
	return NULL;
#endif
}

EC_KEY* ecdh_new_random_keypair() {
	// Generate key
	EC_KEY* resKey = EC_KEY_new_by_curve_name(CURVE_NID);
	EC_KEY_generate_key(resKey);
	return resKey;
}

void ecdh_free_random_keypair(EC_KEY* ecKey) {
	if (ecKey) EC_KEY_free(ecKey);
}

int ecdh_compute_key(unsigned char **outBuf, const EC_POINT *pub_key, EC_KEY *ecKey) {
	size_t outLen = ECDH_DH_KEY_LEN;
	int resLen = ECDH_compute_key(*outBuf, outLen, pub_key, ecKey, KDF1_SHA1);
	return (resLen < 4 ? -1 : resLen);
}

int ecdh_compute_dh_key(const unsigned char *exchangekeybuffer, unsigned char *outBuf, EC_KEY* randomKeyPair) {
	if (!exchangekeybuffer || !outBuf || !randomKeyPair)
		return -1;
	EC_KEY *decodedKey = EC_KEY_new_by_curve_name(CURVE_NID);
	if (!decodedKey)
		return -1;

	// decoded public key.
	o2i_ECPublicKey(&decodedKey, &exchangekeybuffer, ECDH_ECCHANGE_KEY_BUFFER_LEN);
	// compute DH key.
	int resLen = ecdh_compute_key(&outBuf, EC_KEY_get0_public_key(decodedKey), randomKeyPair);
	EC_KEY_free(decodedKey);

	return resLen;
}

int ecdh_get_exchange_keybuffer(EC_KEY *key, unsigned char *exchangekeybuffer) {
	// encoded public key.
	return i2o_ECPublicKey(key, &exchangekeybuffer);
}
