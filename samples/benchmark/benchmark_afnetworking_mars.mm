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
//  benchmark_afnetworking_mars.m
//  iOSDemo
//
//  Created by zhouzhijie on 17/1/10.
//  Copyright © 2017年 zhouzhijie. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "benchmark_afnetworking_mars.h"
#import "Main.pb.h"
#import "CgiTask.h"
#import "NetworkService.h"
#import "CommandId.h"
#import "LogUtil.h"
#import "AFNetworking/AFNetworking.h"

@interface AFProtobufRequestSerializer : AFHTTPRequestSerializer
@end

@implementation AFProtobufRequestSerializer

- (NSURLRequest*)requestBySerializingRequest:(NSURLRequest *)request withParameters:(id)parameters error:(NSError *__autoreleasing  _Nullable *)error {
    NSParameterAssert(request);
    if(!parameters) {
        return [super requestBySerializingRequest:request withParameters:parameters error:error];
    }
    else {
        NSMutableURLRequest* mutableReq = [request mutableCopy];
        [self.HTTPRequestHeaders enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, NSString * _Nonnull obj, BOOL * _Nonnull stop) {
            if(![request valueForHTTPHeaderField:key]) {
                [mutableReq setValue:obj forHTTPHeaderField:key];
            }
        }];
        [mutableReq setHTTPBody:parameters];
        return mutableReq;
    }
}

@end

@interface BenchMark()

@property (nonatomic) UInt64 start_time;
@property (nonatomic) UInt64 task_time;
@property (nonatomic) UInt64 task_cnt;
@property (nonatomic) UInt64 task_suc;
@property (nonatomic) UInt64 suc_time;
@property (nonatomic) uint32_t scene;
@property (nonatomic) int32_t trycnt;

@end

@implementation BenchMark

@synthesize start_time;
@synthesize task_time;
@synthesize task_cnt;
@synthesize task_suc;
@synthesize suc_time;
@synthesize scene;
@synthesize trycnt;

typedef NS_ENUM(NSInteger, BenchMarkScene) {
    SceneMars = 0,
    SceneAF,
    SceneSensitivity,
};

-(NSData*) makeDumpData:(uint32_t)_size {
    char* buff = new char[_size];
    NSData* dump = [[NSData alloc] initWithBytes:buff length:_size];
    delete []buff;
    return dump;
}

-(void) StartAfnetworkingTest {
    scene = SceneAF;
    NSLog(@"benchmark afnetworking start");
    
    start_time = [[NSDate date] timeIntervalSince1970] * 1000;
    task_cnt = task_suc = suc_time = 0;
    trycnt = -1;
    
    [self doAFNetworking];
}

-(void) doAFNetworking {
    task_time = [[NSDate date] timeIntervalSince1970] * 1000;
    
    AFHTTPSessionManager* manager = [AFHTTPSessionManager manager];
    manager.requestSerializer = [AFProtobufRequestSerializer serializer];
    manager.responseSerializer = [AFHTTPResponseSerializer serializer];

    [manager.requestSerializer setValue:@"application/octet-stream" forHTTPHeaderField:@"Content-Type"];
    manager.responseSerializer.acceptableContentTypes = [NSSet setWithObjects:@"application/octet-stream", nil];

    //Benchmark scene: 64KB,128KB request test
    //HelloRequest* helloRequest = [[[[[HelloRequest builder] setUser:@"afnetworking"] setText:@"hello afnetworking"] setDumpContent:[self makeDumpData:128*1024]] build];
    HelloRequest* helloRequest = [[[[HelloRequest builder] setUser:@"afnetworking"] setText:@"hello afnetworking"] build];
    [manager POST:@"http://118.89.24.72:8080/mars/hello2" parameters:[helloRequest data] progress:nil success:^(NSURLSessionDataTask * _Nonnull task, id  _Nullable responseObject) {
        NSData* data = (NSData*)responseObject;
        [self onTaskEnd:true data:data];
    } failure:^(NSURLSessionDataTask * _Nullable task, NSError * _Nonnull error) {
        [self onTaskEnd:false data:nil];
    }];
}

-(void) onTaskEnd:(bool)bOK data:(NSData*)data {
    UInt64 curr = [[NSDate date] timeIntervalSince1970] * 1000;
    UInt64 cost = curr - task_time;
    ++task_cnt;
    if(bOK) {
        task_suc++;
        suc_time += cost;
        //HelloResponse* resp = [HelloResponse parseFromData:data];
        //NSLog(@"benchmark afnetworking type:%d suc cost:%llu, cnt:%llu,suc cnt:%llu, ctn:%@, total: %llu", type, (curr - task_time), task_cnt, task_suc, [resp errmsg], (curr - start_time));
    }
    else
        NSLog(@"benchmark afnetworking fail cnt:%llu, cost:%llu", task_cnt, cost);
    
    if((task_suc%50) == 0) {
        UInt64 curr = [[NSDate date] timeIntervalSince1970] * 1000;
        NSLog(@"benchmark afnetworking total:%llu, avg:%llu, suc_rate:%f, suc_cnt:%llu, count:%llu", (curr - start_time), (suc_time)/task_suc, task_suc/(double)task_cnt, task_suc, task_cnt);
    }
    
    if(task_suc >= 500) {
        UInt64 curr = [[NSDate date] timeIntervalSince1970] * 1000;
        NSLog(@"benchmark afnetworking total:%llu, avg:%llu, suc_rate:%f, suc:%llu", (curr - start_time), (suc_time)/task_suc, task_suc/(double)task_cnt, task_suc);
        return;
    }
    
    [self doAFNetworking];
}

-(void) StartMarsTest {
    scene = SceneMars;
    NSLog(@"benchmark mars start");
    
    start_time = [[NSDate date] timeIntervalSince1970] * 1000;
    task_time = [[NSDate date] timeIntervalSince1970] * 1000;
    task_cnt = task_suc = suc_time = 0;
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kSayHello2 AndCGIUri:@"/mars/hello2" AndHost:@"118.89.24.72"];
    [[NetworkService sharedInstance] startTask:helloCGI ForUI:self];
}

- (int)onTaskEnd:(uint32_t)tid errType:(uint32_t)errtype errCode:(uint32_t)errcode {
    UInt64 curr = [[NSDate date] timeIntervalSince1970] * 1000;
    if(scene == SceneSensitivity) {
        NSLog(@"benchmark mars errtype:%d, errcode:%d, cost:%f", errtype, errcode, ([[NSDate date] timeIntervalSince1970] * 1000 - task_time));
        return 0;
    }
    
    task_cnt++;
    if(errtype == 0 && errcode == 0) {
        task_suc++;
        suc_time += curr - task_time;
        
        //NSLog(@"benchmark mars suc cost:%llu, cnt:%llu, suc cnt:%llu, total:%llu", (curr - task_time), task_cnt, task_suc, (curr - start_time));
    } else {
        NSLog(@"benchmark mars fail cnt:%llu, cost:%llu, errtype:%d, errcode:%d", task_cnt, (curr-task_time), errtype, errcode);
    }
    
    if((task_suc%50) == 0) {
        NSLog(@"benchmark mars total:%llu, avg:%llu, suc_rate:%f, suc:%llu, cnt:%llu", (curr - start_time), (suc_time)/task_suc, task_suc/(double)task_cnt, task_suc, task_cnt);
    }
    if(task_suc >= 500) {
        NSLog(@"benchmark mars final total:%llu, avg:%llu, suc_rate:%f, suc:%llu", (curr - start_time), (suc_time)/task_suc, task_suc/(double)task_cnt, task_suc);
        return 0;
    }
    
    task_time = [[NSDate date] timeIntervalSince1970] * 1000;
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kSayHello2 AndCGIUri:@"/mars/hello2" AndHost:@"118.89.24.72"];
    [[NetworkService sharedInstance] startTask:helloCGI ForUI:self];
    
    return 0;
}

-(NSData*)requestSendData {
    HelloRequest* helloRequest = [[[[HelloRequest builder] setUser:@"anonymous"] setText:@"Hello world!"] build];
    //Benchmark scene: 64KB,128KB request test
    //HelloRequest* helloRequest = [[[[[HelloRequest builder] setUser:@"mars"] setText:@"Hello mars!"] setDumpContent:[self makeDumpData:128*1024]] build];
    NSData* data = [helloRequest data];
    return data;
}

-(int)onPostDecode:(NSData*)responseData {
    //HelloResponse* helloResponse = [HelloResponse parseFromData:responseData];
    return 0;
}

/*
 * benchmark scene: sensitivity test
 * step 1: set network 100% Loss
 * step 2: after 5s, 10s, 15s....40s, set network available
 * step 3: compare the cgi result
 */
-(void) StartSensitivityTest {
    NSLog(@"start sensitivity test");
    scene = SceneSensitivity;
    
    UInt64 time = [[NSDate date] timeIntervalSince1970] * 1000;
    AFHTTPSessionManager* manager = [AFHTTPSessionManager manager];
    manager.requestSerializer = [AFHTTPRequestSerializer serializer];
    manager.responseSerializer = [AFHTTPResponseSerializer serializer];
    [manager.requestSerializer setValue:@"application/octet-stream" forHTTPHeaderField:@"Content-Type"];
    manager.responseSerializer.acceptableContentTypes = [NSSet setWithObjects:@"application/octet-stream", nil];
    [manager POST:@"http://118.89.24.72:8080/mars/hello2" parameters:nil progress:nil success:^(NSURLSessionDataTask * _Nonnull task, id  _Nullable responseObject) {
        NSLog(@"benchmark afnetworking suc:%f", ([[NSDate date] timeIntervalSince1970] * 1000 - time));
    } failure:^(NSURLSessionDataTask * _Nullable task, NSError * _Nonnull error) {
        NSLog(@"benchmark afnetworking fail:%f", ([[NSDate date] timeIntervalSince1970] * 1000 - time));
    }];
    
    task_time = [[NSDate date] timeIntervalSince1970] * 1000;
    CGITask *helloCGI = [[CGITask alloc] initAll:ChannelType_ShortConn AndCmdId:kSayHello2 AndCGIUri:@"/mars/hello2" AndHost:@"118.89.24.72"];
    [[NetworkService sharedInstance] startTask:helloCGI ForUI:self];
}

@end
