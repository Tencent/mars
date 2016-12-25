//
//  testspy_spy.cpp
//  PublicComponent
//
//  Created by Ray on 14-5-13.
//  Copyright (c) 2014年 Tencent. All rights reserved.
//


#define SPY_DEF_CLASS_NAME TSpy
#define SPY_DEF_XLOGGER_HOOK TSpy::SpyHookLogFunc

#include "comm/debugger/spy.inl"

class TSpy :  public Spy {
  public:
    TSpy(void* _this): Spy(_this) {}


    static bool SpyHookLogFunc(struct XLoggerInfo_t& _info, std::string& _log);
    void TestFun0();
    int __TestFun1(int i);

  private:
    virtual void __OnAttach(const char* _key) {}
    virtual void __OnDetach(const char* _key) {}
};
