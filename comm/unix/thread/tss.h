/*  Copyright (c) 2013-2015 Tencent. All rights reserved.  */

#ifndef TSS_H_
#define TSS_H_

#include <pthread.h>
typedef void (*cleanup_route)(void*);

class Tss {
  public:
    explicit Tss(cleanup_route cleanup) {
        pthread_key_create(&_key, cleanup);
    }

    ~Tss() {
        pthread_key_delete(_key);
    }

    void* get() const {
        return pthread_getspecific(_key);
    }

    void set(void* value) {
        pthread_setspecific(_key, value);
    }

  private:
    Tss(const Tss&);
    Tss& operator =(const Tss&);

  private:
    pthread_key_t _key;
};

#endif /* TSS_H_ */
