/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */
#include <string>

bool getProxyInfoImpl(int& port, std::string& strProxy, const std::string& _host);

bool isNetworkConnectedImpl();

DWORD getNetworkStatus();