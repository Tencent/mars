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
//  scope_autoreleasepool.h
//  MicroMessenger
//
//  Created by yerungui on 12-11-30.
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
