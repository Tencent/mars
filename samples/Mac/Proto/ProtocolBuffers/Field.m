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

#import "CodedOutputStream.h"
#import "PBArray.h"
#import "UnknownFieldSet.h"
#import "Utilities.h"

@implementation PBField

@synthesize varintArray = _varintArray;
@synthesize fixed32Array = _fixed32Array;
@synthesize fixed64Array = _fixed64Array;
@synthesize lengthDelimitedArray = _lengthDelimitedArray;
@synthesize groupArray = _groupArray;

static PBField *sDefaultInstance = nil;

+ (void)initialize {
	if (self == [PBField class]) {
		sDefaultInstance = [[PBField alloc] init];
	}
}


+ (PBField *)defaultInstance {
	return sDefaultInstance;
}

- (SInt32)getSerializedSize:(SInt32)fieldNumber {
	SInt32 result = 0;

	const SInt64 *varintValues = (const SInt64 *)_varintArray.data;
	if (varintValues) {
		const NSUInteger count = _varintArray.count;
		for (UInt32 i = 0; i < count; ++i) {
			result += computeInt64Size(fieldNumber, varintValues[i]);
		}
	}

	const SInt32 *fixed32Values = (const SInt32 *)_fixed32Array.data;
	if (fixed32Values) {
		const NSUInteger count = _fixed32Array.count;
		for (UInt32 i = 0; i < count; ++i) {
			result += computeFixed32Size(fieldNumber, fixed32Values[i]);
		}
	}

	const SInt64 *fixed64Values = (const SInt64 *)_fixed64Array.data;
	if (fixed64Values) {
		const NSUInteger count = _fixed64Array.count;
		for (NSUInteger i = 0; i < count; ++i) {
			result += computeFixed64Size(fieldNumber, fixed64Values[i]);
		}
	}

	for (NSData *value in _lengthDelimitedArray) {
		result += computeDataSize(fieldNumber, value);
	}

	for (PBUnknownFieldSet *value in _groupArray) {
		result += computeUnknownGroupSize(fieldNumber, value);
	}

	return result;
}

- (SInt32)getSerializedSizeAsMessageSetExtension:(SInt32)fieldNumber {
	SInt32 result = 0;

	for (NSData *value in _lengthDelimitedArray) {
		result += computeRawMessageSetExtensionSize(fieldNumber, value);
	}

	return result;
}

- (void)writeTo:(SInt32)fieldNumber output:(PBCodedOutputStream *) output {
	const SInt64 *varintValues = (const SInt64 *)_varintArray.data;
	if (varintValues) {
		const NSUInteger count = _varintArray.count;
		for (NSUInteger i = 0; i < count; ++i) {
			[output writeInt64:fieldNumber value:varintValues[i]];
		}
	}

	const SInt32 *fixed32Values = (const SInt32 *)_fixed32Array.data;
	if (fixed32Values) {
		const NSUInteger count = _fixed32Array.count;
		for (NSUInteger i = 0; i < count; ++i) {
			[output writeFixed32:fieldNumber value:fixed32Values[i]];
		}
	}

	const SInt64 *fixed64Values = (const SInt64 *)_fixed64Array.data;
	if (fixed64Values) {
		const NSUInteger count = _fixed64Array.count;
		for (NSUInteger i = 0; i < count; ++i) {
			[output writeFixed64:fieldNumber value:fixed64Values[i]];
		}
	}

	for (NSData *value in _lengthDelimitedArray) {
		[output writeData:fieldNumber value:value];
	}

	for (PBUnknownFieldSet *value in _groupArray) {
		[output writeUnknownGroup:fieldNumber value:value];
	}
}

- (void)writeDescriptionFor:(SInt32) fieldNumber
                         to:(NSMutableString*) output
                 withIndent:(NSString*) indent {
    [self.varintArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        
    }];
    [self.varintArray enumerateObjectsUsingBlock:^(NSNumber* value, NSUInteger idx, BOOL *stop) {
        [output appendFormat:@"%@%ld: %qi\n", indent, (long)fieldNumber, value.longLongValue];
    }];
    [self.fixed32Array enumerateObjectsUsingBlock:^(NSNumber* value, NSUInteger idx, BOOL *stop) {
        [output appendFormat:@"%@%ld: %ld\n", indent, (long)fieldNumber, (long)value.integerValue];
    }];
    [self.fixed64Array enumerateObjectsUsingBlock:^(NSNumber* value, NSUInteger idx, BOOL *stop) {
         [output appendFormat:@"%@%ld: %lld\n", indent, (long)fieldNumber, value.longLongValue];
    }];
      for (NSData* value in self.lengthDelimitedArray) {
        [output appendFormat:@"%@%ld: %@\n", indent, (long)fieldNumber, value];
      }
      for (PBUnknownFieldSet* value in self.groupArray) {
        [output appendFormat:@"%@%ld: [\n", indent, (long)fieldNumber];
        [value writeDescriptionTo:output withIndent:[NSString stringWithFormat:@"%@  ", indent]];
        [output appendFormat:@"%@]", indent];
      }
}

- (void)writeAsMessageSetExtensionTo:(SInt32)fieldNumber output:(PBCodedOutputStream *) output {
	for (NSData *value in _lengthDelimitedArray) {
		[output writeRawMessageSetExtension:fieldNumber value:value];
	}
}

@end