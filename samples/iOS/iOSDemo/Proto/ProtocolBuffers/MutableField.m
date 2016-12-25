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

#import "MutableField.h"

#import "Field.h"
#import "PBArray.h"

@implementation PBMutableField


+ (PBMutableField *)field {
	return [[PBMutableField alloc] init];
}

- (PBMutableField *)clear {

    _varintArray = nil;
	_fixed32Array = nil;
    _fixed64Array = nil;
    _lengthDelimitedArray = nil;
    _groupArray = nil;

	return self;
}

- (PBMutableField *)mergeFromField:(PBField *)other {
	if (other.varintArray.count > 0) {
		if (_varintArray == nil) {
			_varintArray = [other.varintArray copy];
		} else {
			[_varintArray appendArray:other.varintArray];
		}
	}

	if (other.fixed32Array.count > 0) {
		if (_fixed32Array == nil) {
			_fixed32Array = [other.fixed32Array copy];
		} else {
			[_fixed32Array appendArray:other.fixed32Array];
		}
	}

	if (other.fixed64Array.count > 0) {
		if (_fixed64Array == nil) {
			_fixed64Array = [other.fixed64Array copy];
		} else {
			[_fixed64Array appendArray:other.fixed64Array];
		}
	}

	if (other.lengthDelimitedArray.count > 0) {
		if (_lengthDelimitedArray == nil) {
			_lengthDelimitedArray = [other.lengthDelimitedArray mutableCopy];
		} else {
			[_lengthDelimitedArray addObjectsFromArray:other.lengthDelimitedArray];
		}
	}

	if (other.groupArray.count > 0) {
		if (_groupArray == nil) {
			_groupArray = [other.groupArray mutableCopy];
		} else {
			[_groupArray addObjectsFromArray:other.groupArray];
		}
	}

	return self;
}

- (PBMutableField *)addVarint:(SInt64)value {
	if (_varintArray == nil) {
		_varintArray = [[PBAppendableArray alloc] initWithValueType:PBArrayValueTypeInt64];
	}
	[_varintArray addInt64:value];

	return self;
}

- (PBMutableField *)addFixed32:(SInt32)value {
	if (_fixed32Array == nil) {
		_fixed32Array = [[PBAppendableArray alloc] initWithValueType:PBArrayValueTypeInt32];
	}
	[_fixed32Array addInt32:value];

	return self;
}

- (PBMutableField *)addFixed64:(SInt64)value {
	if (_fixed64Array == nil) {
		_fixed64Array = [[PBAppendableArray alloc] initWithValueType:PBArrayValueTypeInt64];
	}
	[_fixed64Array addInt64:value];

	return self;
}

- (PBMutableField *)addLengthDelimited:(NSData *)value {
	if (_lengthDelimitedArray == nil) {
		_lengthDelimitedArray = [[NSMutableArray alloc] init];
	}
	[_lengthDelimitedArray addObject:value];

	return self;
}

- (PBMutableField *)addGroup:(PBUnknownFieldSet *)value {
	if (_groupArray == nil) {
		_groupArray = [[NSMutableArray alloc] init];
	}
	[_groupArray addObject:value];

	return self;
}

@end