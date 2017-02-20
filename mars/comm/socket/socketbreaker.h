/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#ifdef _WIN32
#include "../windows/SocketSelect/socketselect2.h"
#else
#include "../unix/socket/socketbreaker.h"
#endif
