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
//  testspy.cpp
//  PublicComponent
//
//  Created by yerungui on 14-5-13.
//

#include "comm/debugger/test_spy_sample.h"
#include "comm/xlogger/xlogger.h"

#include "comm/debugger/testspy.h"


Test_Spy_Sample::Test_Spy_Sample()
{
    SPY_ATTACH_CLASS(NULL);
}

Test_Spy_Sample::~Test_Spy_Sample()
{
    SPY_DETACH_CLASS();
}

void Test_Spy_Sample::TestFun0()
{
    SPY_HOOK_THIS_API(TestFun0);
    int i = 0;
    SPY_ATTACH_VARIABLE("TestFun0 i", i);
    i++;
    xinfo2(TSF"Test");

    SPY_DETACH_VARIABLE("TestFun0 i");
}

int Test_Spy_Sample::__TestFun1(int i)
{
    SPY_HOOK_THIS_API(__TestFun1, i);
    xinfo2(TSF"Test");
    return i+1;
}
