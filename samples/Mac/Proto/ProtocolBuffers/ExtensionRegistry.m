// Protocol Buffers for Objective C
//
// Copyright 2010 Booyah Inc.
// Copyright 2008 Cyrus Najmabadi
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#import "ExtensionRegistry.h"

@interface PBExtensionRegistry()
@property (strong) NSDictionary* classMap;
@end

@implementation PBExtensionRegistry

@synthesize classMap;


static PBExtensionRegistry* emptyRegistry = nil;

+ (void) initialize {
  if (self == [PBExtensionRegistry class]) {
    emptyRegistry = [[PBExtensionRegistry alloc] initWithClassMap:[NSDictionary dictionary]];
  }
}


- (instancetype) initWithClassMap:(NSDictionary*) map_{
  if ((self = [super init])) {
    self.classMap = map_;
  }

  return self;
}


- (id) keyForClass:(Class) clazz {
  return NSStringFromClass(clazz);
}


+ (PBExtensionRegistry*) emptyRegistry {
  return emptyRegistry;
}


- (id<PBExtensionField>) getExtension:(Class) clazz fieldNumber:(SInt32) fieldNumber {
  NSDictionary* extensionMap = [classMap objectForKey:[self keyForClass:clazz]];
  return [extensionMap objectForKey:[NSNumber numberWithInteger:fieldNumber]];
}

@end
