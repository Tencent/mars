
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
//  comm_data.h
//  comm
//
//  Created by garry on 2017/2/17.
//

#ifndef comm_data_h
#define comm_data_h

#include <string>
#include <stdint.h>

namespace mars {
    namespace comm {
        
enum ProxyType {
    kProxyNone = 0,
    kProxyHttpTunel,
    kProxySocks5,
    kProxyHttp,
};
        
class ProxyInfo {
public:
    ProxyInfo():ProxyInfo(kProxyNone, "", "", 0, "", ""){}
    ProxyInfo(ProxyType _type, const std::string& _host, const std::string& _ip, uint16_t _port, const std::string& _username, const std::string& _password)
    :type(_type), host(_host), ip(_ip), port(_port), username(_username), password(_password){}
    
    bool IsValid() const {
        return kProxyNone == type || ((!ip.empty() || !host.empty()) && port > 0);
    }
    
public:
    ProxyType type;
    std::string host;
    std::string ip;
    uint16_t port;
    std::string username;
    std::string password;
};
        
    
    }
}

#endif /* comm_data_h */
