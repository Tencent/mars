//
//  ecdh_util.h
//  openssl
//
//  Created by elviswu on 2017/5/11.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#ifndef ecdh_util_h
#define ecdh_util_h

#include <string>

int GenEcdhKeyPair(int _nid, std::string& _pub_key, std::string& _pri_key);
int ComputeDh(int _nid, const std::string& _pub_key, const std::string& _pri_key, std::string& _result);

#endif /* ecdh_util_h */
