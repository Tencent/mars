//
//  getdnssvraddress.hpp
//  comm
//
//  Created by yerungui on 16/1/19.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#ifndef getdnssvraddress_hpp
#define getdnssvraddress_hpp

#include <vector>
#include "comm/socket/socket_address.h"

void getdnssvraddrs(std::vector<socket_address>& _dnssvraddrs);

#endif /* getdnssvraddress_hpp */
