//
//  base_netinfo_query.h
//  sdt
//
//  Created by elviswu on 2017/7/6.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#ifndef base_netinfo_query_h
#define base_netinfo_query_h

#include <string>

#include "mars/comm/socket/local_ipstack.h"

class BaseNetInfo {
  public:
    BaseNetInfo();
    ~BaseNetInfo();
    
    const BaseNetInfo& Query();
    
    std::string ToString() const;
    
    int nettype() const {return nettype_;}
    TLocalIPStack ip_stack() const {return ip_stack_;}
    std::string detail_nettype_info() const {return detail_nettype_info_;}
    std::string ipstack_detect_info() const {return ipstack_detect_info_;}
    std::string net_gateway_info() const {return net_gateway_info_;}
    std::string net_dnssvr_info() const {return net_dnssvr_info_;}
    std::string net_interface_info() const {return net_dnssvr_info_;}
    std::string route_info() const {return route_info_;}
    
  private:
    void __QueryNetType();
    void __QueryIPStack();
    void __QueryNetConfig();
  private:
    int nettype_;
    std::string detail_nettype_info_;
    
    TLocalIPStack ip_stack_;
    std::string ipstack_detect_info_;
    
    std::string net_gateway_info_;
    std::string net_dnssvr_info_;
    std::string net_interface_info_;
    std::string route_info_;
};


#endif /* base_netinfo_query_h */
