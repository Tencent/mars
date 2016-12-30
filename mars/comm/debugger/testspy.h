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
//  Created by yerungui on 14-5-13.
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
