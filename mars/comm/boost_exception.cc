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
//  boost_exception.cpp
//  comm
//
//  Created by yanguoyue on 16/5/20.
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
