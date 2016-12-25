//
//  testspy.cpp
//  PublicComponent
//
//  Created by Ray on 14-5-13.
//  Copyright (c) 2014年 Tencent. All rights reserved.
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
