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

#import <Foundation/Foundation.h>

@class PBArray;
@class PBAppendableArray;
@class PBCodedOutputStream;

@interface PBField : NSObject
{
@protected
	PBAppendableArray *	_varintArray;
	PBAppendableArray *	_fixed32Array;
	PBAppendableArray *	_fixed64Array;
	NSMutableArray *	_lengthDelimitedArray;
	NSMutableArray *	_groupArray;
}

@property (nonatomic,strong,readonly) PBArray *	varintArray;
@property (nonatomic,strong,readonly) PBArray *	fixed32Array;
@property (nonatomic,strong,readonly) PBArray *	fixed64Array;
@property (nonatomic,strong,readonly) NSArray *	lengthDelimitedArray;
@property (nonatomic,strong,readonly) NSArray *	groupArray;

+ (PBField *)defaultInstance;

- (SInt32)getSerializedSize:(SInt32)fieldNumber;
- (SInt32)getSerializedSizeAsMessageSetExtension:(SInt32)fieldNumber;

- (void)writeTo:(SInt32) fieldNumber output:(PBCodedOutputStream *)output;
- (void)writeAsMessageSetExtensionTo:(SInt32)fieldNumber output:(PBCodedOutputStream *)output;
- (void)writeDescriptionFor:(SInt32) fieldNumber
                         to:(NSMutableString*) output
                 withIndent:(NSString*) indent;
@end
