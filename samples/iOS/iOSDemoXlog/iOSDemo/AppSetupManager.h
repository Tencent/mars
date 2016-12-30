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
//  AppSetupManager.h
//  mactest
//
//  Created by caoshaokun on 16/5/10.
//  Copyright © 2016年 caoshaokun. All rights reserved.
//

#ifndef AppSetupManager_h
#define AppSetupManager_h

#import <Foundation/Foundation.h>

@interface AppSetupManager : NSObject

+ (AppSetupManager *)shareInstance;
- (void)setupApp;

- (void)setupPublicCompenont;


@end

#endif /* AppSetupManager_h */
