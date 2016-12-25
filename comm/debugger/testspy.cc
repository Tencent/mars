//
//  testspy_spy.cpp
//  PublicComponent
//
//  Created by Ray on 14-5-14.
//  Copyright (c) 2014年 Tencent. All rights reserved.
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

