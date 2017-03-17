// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

//
//  log_crypt_ecdh.cc
//
//  Created by garry on 17/3/6.
//

#include "openssl/ecdsa.h"
#include "openssl/ecdh.h"
#include "openssl/sha.h"
#include "openssl/bn.h"
#include "openssl/err.h"

namespace mars {
    namespace log {
    

bool GenEcdhKeyPair(int nid, unsigned char** _pub_key_x, int& _key_x_len,
                   unsigned char** _pub_key_y, int& _key_y_len,
                   unsigned char** _private_key_buf, int& _private_key_size) {
    int ret = 0;
    EC_KEY* ec_key = NULL;
    BIGNUM* pub_x = NULL;
    BIGNUM* pub_y = NULL;
    
    do {
        ec_key = EC_KEY_new_by_curve_name(nid);
        if (NULL == ec_key) {
            ret = -1;
            break;
        }
        
        ret = EC_KEY_generate_key(ec_key);
        if (ret != 1) {
            ret = -1;
            break;
        }
        
        const EC_GROUP* group = EC_KEY_get0_group(ec_key);
        const EC_POINT* public_point = EC_KEY_get0_public_key(ec_key);
        
        if (NULL == group || NULL == public_point) {
            ret = -1;
            break;
        }
        
        pub_x = BN_new();
        pub_y = BN_new();
        
        if (0 == EC_POINT_get_affine_coordinates_GFp(group, public_point, pub_x, pub_y, NULL)) {
            ret = -1;
            break;
        }
        
        const BIGNUM* private_key = EC_KEY_get0_private_key(ec_key);
        if (NULL == private_key) {
            ret = -1;
            break;
        }
        
        _key_x_len = BN_num_bytes(pub_x);
        _key_y_len = BN_num_bytes(pub_y);
        *_pub_key_x = (unsigned char*)OPENSSL_malloc(_key_x_len);
        *_pub_key_y = (unsigned char*)OPENSSL_malloc(_key_y_len);
        
        _private_key_size = BN_num_bytes(private_key);
        *_private_key_buf = (unsigned char*)OPENSSL_malloc(_private_key_size);
        
        BN_bn2bin(pub_x, *_pub_key_x);
        BN_bn2bin(pub_y, *_pub_key_y);
        BN_bn2bin(private_key, *_private_key_buf);
        
    } while (false);
    
    EC_KEY_free(ec_key);
    BN_free(pub_x);
    BN_free(pub_y);
    
    return ret >= 0;
}

bool GetEcdhKey(int nid, const unsigned char* const _own_private_key, int _private_key_len,
               const unsigned char* const _other_pub_key_x, int _key_x_len,
               const unsigned char* const _other_pub_key_y, int _key_y_len, unsigned char ecdh_key_buffer[32]) {
    
    
    int ret = 0;
    EC_KEY* other_key = NULL;
    BIGNUM* other_pub_x = NULL;
    BIGNUM* other_pub_y = NULL;
    EC_POINT* other_public_key = NULL;
    
    EC_KEY* own_key = NULL;
    BIGNUM* own_private_key = NULL;
    
    do {
        other_key = EC_KEY_new_by_curve_name(nid);
        if (NULL == other_key) {
            ret = -1;
            break;
        }
        
        other_pub_x = BN_bin2bn(_other_pub_key_x, _key_x_len, other_pub_x);
        other_pub_y = BN_bin2bn(_other_pub_key_y, _key_y_len, other_pub_y);
        
        const EC_GROUP* other_group = EC_KEY_get0_group(other_key);
        other_public_key = EC_POINT_new(other_group);
        if (NULL == other_group || NULL == other_public_key) {
            ret = -1;
            break;
        }
        
        if (0 == EC_POINT_set_affine_coordinates_GFp(other_group, other_public_key, other_pub_x, other_pub_y, NULL)) {
            ret = -1;
            break;
        }
        
        if (0 == EC_KEY_set_public_key(other_key, other_public_key) || 0 == EC_KEY_check_key(other_key)) {
            ret = -1;
            break;
        }
        
        own_key = EC_KEY_new_by_curve_name(nid);
        if (NULL == own_key) {
            ret = -1;
            break;
        }
        
        own_private_key = BN_bin2bn(_own_private_key, _private_key_len, own_private_key);
        
        if (0 == EC_KEY_set_private_key(own_key, own_private_key)) {
            ret = -1;
            break;
        }
        
        ECDH_set_method(own_key, ECDH_OpenSSL());
        
        if (32 != ECDH_compute_key(ecdh_key_buffer, 32, other_public_key, own_key, NULL)) {
            ret = -1;
            break;
        }
    } while (false);
    
    EC_KEY_free(other_key);
    BN_free(other_pub_x);
    BN_free(other_pub_y);
    EC_POINT_free(other_public_key);
    EC_KEY_free(own_key);
    BN_free(own_private_key);
    
    return ret >= 0;
}

    }
}

