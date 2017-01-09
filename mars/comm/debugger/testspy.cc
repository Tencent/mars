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
//  testspy_spy.cpp
//  PublicComponent
//
//  Created by yerungui on 14-5-14.
//

#include "comm/debugger/testspy.h"

#include "comm/debugger/spy_impl_helper.inl"
#include "comm/debugger/test_spy_sample.h"


bool TSpy::SpyHookLogFunc(XLoggerInfo_t& _info, std::string& _log)
{
    __attribute__((unused)) int i = 0;
    return true;
}

void TSpy::TestFun0()
{
    return reinterpret_cast<Test_Spy_Sample*>(This())->TestFun0();
}

int TSpy::__TestFun1(int i)
{
    return reinterpret_cast<Test_Spy_Sample*>(This())->__TestFun1(i);
}

