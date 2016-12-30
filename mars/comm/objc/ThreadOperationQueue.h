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
//  ThreadOperationQueue.h
//  MicroMessenger
//
//  Created by yerungui on 12-12-18.
//

#ifndef __MicroMessenger__ThreadOperationQueue__
#define __MicroMessenger__ThreadOperationQueue__

#import <Foundation/Foundation.h>

@interface ThreadQueue : NSObject
{}

+(BOOL) RunWithTarget:(id)target selector:(SEL)sel object:(id)arg;
@end


extern "C" BOOL RunWithTarget(void (*_funp)(void*), void* _arg);
extern "C" BOOL RunWithTargetNoParam(void (*_fun)());


#endif /* defined(__MicroMessenger__ThreadOperationQueue__) */
