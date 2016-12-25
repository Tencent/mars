//
//  boost_exception.cpp
//  comm
//
//  Created by yanguoyue on 16/5/20.
//  Copyright © 2016年 Tencent. All rights reserved.
//

#include <exception>
#include "comm/xlogger/xlogger.h"

#ifdef ANDROID
#include "comm/android/callstack.h"
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost {

    void throw_exception( std::exception const & e ) {
        xfatal2(TSF"boost exception:%_", e.what());
        
#ifdef ANDROID
        char stack[4096] = {0};
        android_callstack(stack, sizeof(stack));
        xfatal2(TSF"%_", stack);
#endif
    }
}
