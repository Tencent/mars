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
//  log_crypt_ecdh.h
//
//  Created by yanguoyue on 17/3/6.
//

#ifndef LOG_CRYPT_ECDH_H_
#define LOG_CRYPT_ECDH_H_


namespace mars {
    namespace log {
 
bool GenEcdhKeyPair(int nid, unsigned char** _pub_key_x, int& _key_x_len,
                    unsigned char** _pub_key_y, int& _key_y_len,
                    unsigned char** _private_key_buf, int& _private_key_size);
        
bool GetEcdhKey(int nid, const unsigned char* const _own_private_key, int _private_key_len,
                const unsigned char* const _other_pub_key_x, int _key_x_len,
                const unsigned char* const _other_pub_key_y, int _key_y_len, unsigned char ecdh_key_buffer[32]);
        
    }
}


#endif /* LOG_CRYPT_ECDH_H_ */
