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

#import "Field.h"

@class PBUnknownFieldSet;

@interface PBMutableField : PBField

+ (PBMutableField *)field;

- (PBMutableField *)mergeFromField:(PBField *)other;

- (PBMutableField *)clear;
- (PBMutableField *)addVarint:(SInt64)value;
- (PBMutableField *)addFixed32:(SInt32)value;
- (PBMutableField *)addFixed64:(SInt64)value;
- (PBMutableField *)addLengthDelimited:(NSData *)value;
- (PBMutableField *)addGroup:(PBUnknownFieldSet *)value;

@end