//
//  test_openssl.h
//  network
//
//  Created by wutianqiang on 7/1/16.
//  Copyright © 2016 Tencent. All rights reserved.
//

#ifndef test_openssl_hpp
#define test_openssl_hpp

#include <stdio.h>
#include "thread/thread.h"


class MultiThreadTest {
	static const int kThreadNum = 20;
  public:
    MultiThreadTest();
    ~MultiThreadTest();
    
    void Start();
    void Cancel();
  private:
    void __Run();
  private:
    Thread* threads_[kThreadNum];
    bool cancel_;
};


#endif /* test_openssl_hpp */
