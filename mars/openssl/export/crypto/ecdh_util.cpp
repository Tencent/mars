//
//  ecdh_util.cpp
//  openssl
//
//  Created by elviswu on 2017/5/11.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#include "openssl/ecdh.h"
#include "openssl/sha.h"

#include "mars/openssl/export_include/ecdh_util.h"
#include "mars/comm/xlogger/xlogger.h"

# define KDF_SHA256_LENGTH    32

int GenEcdhKeyPair(int _nid, std::string& _pub_key, std::string& _pri_key)
{
    int ret = -1;
    EC_KEY *ec_key = NULL;
    unsigned char *pub_key_buf = NULL;
    int pub_key_size = 0;
    unsigned char *pri_key_buf = NULL;
    int pri_key_size = 0;
    
    do {
        // create ec key by nid
        ec_key = EC_KEY_new_by_curve_name(_nid);
        if (!ec_key) {
            xerror2(TSF"ERR: EC_KEY_new_by_curve_name failed, nid %_", _nid);
            ret = -1;
            break;
        }
        
        EC_KEY_set_asn1_flag(ec_key, OPENSSL_EC_NAMED_CURVE);
        
        // generate ec key pair
        ret = EC_KEY_generate_key(ec_key);
        if (ret != 1) {
            xerror2(TSF"ERR: EC_KEY_generate_key failed, ret %_", ret);
            ret = -1;
            break;
        }
        
        // get public key from ec key pair
        pub_key_size = i2o_ECPublicKey(ec_key, &pub_key_buf);
        if (pub_key_size == 0 || !pub_key_buf) {
            xerror2(TSF"ERR: i2o_ECPublicKey faild, ret %_", ret);
            ret = -1;
            break;
        }
        
        // get private key from ec key pair
        pri_key_size = i2d_ECPrivateKey(ec_key, &pri_key_buf);
        if (pri_key_size == 0 || !pri_key_buf) {
            xerror2(TSF"ERR: i2d_ECPrivateKey failed, ret %_", ret);
            ret = -1;
            break;
        }
        
        // set key_pair
        _pub_key = std::string((const char *)pub_key_buf, pub_key_size);
        _pri_key = std::string((const char *)pri_key_buf, pri_key_size);
        
        ret = 1;
        
    } while (0);
    
    // free memory
    if (ec_key) {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }
    
    if (pub_key_buf) {
        OPENSSL_free(pub_key_buf);
        pub_key_buf = NULL;
    }
    
    if (pri_key_buf) {
        OPENSSL_free(pri_key_buf);
        pri_key_buf = NULL;
    }
    
    return ret;
}





static void *KdfSha256(const void *in, size_t in_len, void *out, size_t *out_len)
{
    if ((!out_len)  || (!in) || (!in_len)  || *out_len < KDF_SHA256_LENGTH)
        return NULL;
    else
        *out_len = KDF_SHA256_LENGTH;
    
    return SHA256((const unsigned char *)in, in_len, (unsigned char *)out);
}
int Ecdh(int nid,
         const unsigned char *public_material, size_t public_material_size,
         const unsigned char *private_material, size_t private_material_size,
         std::string& result)
{
    int ret = -1;
    EC_KEY *pub_ec_key = NULL;
    EC_KEY *pri_ec_key = NULL;
    
    do {
        // load public key
        pub_ec_key = EC_KEY_new_by_curve_name(nid);
        if (!pub_ec_key) {
            xerror2(TSF"ERR: public key EC_KEY_new_by_curve_name failed, nid %_",  nid);
            ret = -1;
            break;
        }
        
        pub_ec_key = o2i_ECPublicKey(&pub_ec_key, &public_material, public_material_size);
        if (!pub_ec_key) {
            xerror2(TSF"ERR: public key o2i_ECPublicKey failed, nid %_", nid);
            ret = -1;
            break;
        }
        
        // load private key
        pri_ec_key = EC_KEY_new_by_curve_name(nid);
        if (!pri_ec_key) {
            xerror2(TSF"ERR: private key EC_KEY_new_by_curve_name failed, nid %_", nid);
            ret = -1;
            break;
        }
        
        pri_ec_key = d2i_ECPrivateKey(&pri_ec_key, &private_material, private_material_size);
        if (!pri_ec_key) {
            xerror2(TSF"ERR: private key d2i_ECPrivateKey failed, nid %_", nid);
            ret = -1;
            break;
        }
        
        // compute ecdh key
        result.resize(KDF_SHA256_LENGTH);
        unsigned char *result_buf = (unsigned char *)result.data();
        
        int res = ECDH_compute_key(result_buf, KDF_SHA256_LENGTH, EC_KEY_get0_public_key(pub_ec_key), pri_ec_key, KdfSha256);
        if (res != KDF_SHA256_LENGTH) {
            xerror2(TSF"ERR: ECDH_compute_key failed, nid %_ res %_ kdf len %_", nid, res, KDF_SHA256_LENGTH);
            ret = -1;
            break;
        }
        
        ret = 1;
        
    } while (0);
    
    // free memory
    if (pub_ec_key) {
        EC_KEY_free(pub_ec_key);
        pub_ec_key = NULL;
    }
    
    if (pri_ec_key) {
        EC_KEY_free(pri_ec_key);
        pri_ec_key = NULL;
    }
    
    return ret;
}

int ComputeDh(int _nid, const std::string& _pub_key, const std::string& _pri_key, std::string& _result)
{
    if (_pub_key.empty() || _pri_key.empty() ) {
        xerror2(TSF"ERR: invalid param.");
        return -1;
    }
    
    return Ecdh(_nid, (const unsigned char*)_pub_key.data(), _pub_key.size(), (const unsigned char*)_pri_key.data(), _pri_key.size(), _result);
}
