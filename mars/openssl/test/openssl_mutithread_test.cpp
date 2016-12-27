//
//  test_openssl.cpp
//  network
//
//  Created by elviswu on 7/1/16.
//  Copyright © 2016 Tencent. All rights reserved.
//

#include "thread/bind.h"
#include "openssl/rand.h"
#include "openssl_mutithread_test.h"
#include "xlogger/xlogger.h"
MultiThreadTest::MultiThreadTest() :cancel_(false){

    for (int i=0; i < kThreadNum; ++i) {
    
        threads_[i] = new Thread(ThreadBinder::bind(&MultiThreadTest::__Run, this));
    }
    
}

MultiThreadTest::~MultiThreadTest() {
    for (int i=0; i < kThreadNum; ++i) {
        
       delete (threads_[i]);
    }
}

void MultiThreadTest::__Run() {
    xinfo2(TSF"thread id=%_", pthread_self());
    unsigned char * result_buf = (unsigned char*) malloc(32);
    for (;;) {
        if (cancel_) break;
        int ret = RAND_bytes(result_buf, 32);
        if (1!=ret) {
            xinfo2(TSF"RAND_bytes error.");
        }
    }
    free(result_buf);
    
    
}
void MultiThreadTest::Cancel() {
    cancel_ = true;
}
void MultiThreadTest::Start() {
    for (int i=0; i < kThreadNum; ++i) {
        
        int ret = (threads_[i])->start();
        if (0!=ret) {
            xinfo2(TSF"start error.");
        }
    }
}
