//
//  scope_autoreleasepool.h
//  MicroMessenger
//
//  Created by yerungui on 12-11-30.
//  Copyright (c) 2012年 Tencent. All rights reserved.
//

#ifndef __MicroMessenger__scope_autoreleasepool__
#define __MicroMessenger__scope_autoreleasepool__

class Scope_AutoReleasePool {
  public:
    Scope_AutoReleasePool();
    ~Scope_AutoReleasePool();

  private:
    Scope_AutoReleasePool(const Scope_AutoReleasePool&);
    Scope_AutoReleasePool& operator=(const Scope_AutoReleasePool&);

  private:
    id m_pool;
};

#define SCOPE_POOL() Scope_AutoReleasePool __pool__##__LINE__

#endif /* defined(__MicroMessenger__scope_autoreleasepool__) */
