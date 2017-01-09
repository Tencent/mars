// Protocol Buffers for Objective C
//
// Copyright 2010 Booyah Inc.
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
//
// Author: Jon Parise <jon@booyah.com>

#import <Foundation/Foundation.h>

extern NSString * const PBArrayTypeMismatchException;
extern NSString * const PBArrayNumberExpectedException;
extern NSString * const PBArrayAllocationFailureException;

typedef enum _PBArrayValueType
{
	PBArrayValueTypeBool,
	PBArrayValueTypeInt32,
	PBArrayValueTypeUInt32,
	PBArrayValueTypeInt64,
	PBArrayValueTypeUInt64,
	PBArrayValueTypeFloat,
	PBArrayValueTypeDouble,
} PBArrayValueType;

// PBArray is an immutable array class that's optimized for storing primitive
// values.  All values stored in an PBArray instance must have the same type
// (PBArrayValueType).  Object values (PBArrayValueTypeObject) are retained.
@interface PBArray : NSObject <NSCopying>
{
@protected
	PBArrayValueType	_valueType;
	NSUInteger			_capacity;
	NSUInteger			_count;
	void *				_data;

}

- (NSUInteger)count;
- (BOOL)boolAtIndex:(NSUInteger)index;
- (SInt32)int32AtIndex:(NSUInteger)index;
- (SInt32)enumAtIndex:(NSUInteger)index;
- (UInt32)uint32AtIndex:(NSUInteger)index;
- (SInt64)int64AtIndex:(NSUInteger)index;
- (UInt64)uint64AtIndex:(NSUInteger)index;
- (Float32)floatAtIndex:(NSUInteger)index;
- (Float64)doubleAtIndex:(NSUInteger)index;
- (BOOL)isEqualToArray:(PBArray *)array;
- (void)enumerateObjectsUsingBlock:(void (^)(id obj, NSUInteger idx, BOOL *stop))block;
- (NSUInteger)indexOfObjectPassingTest:(BOOL (^)(id obj, NSUInteger idx, BOOL *stop))predicate;

//This Methods automaticaly pack/unpack in NSNumber primitive values
- (id)firstObject;
- (id)lastObject;
- (id)objectAtIndexedSubscript:(NSUInteger)idx;

@property (nonatomic,assign,readonly) PBArrayValueType valueType;
@property (nonatomic,assign,readonly) const void * data;
@property (nonatomic,assign,readonly,getter=count) NSUInteger count;

@end

@interface PBArray (PBArrayExtended)

- (instancetype)arrayByAppendingArray:(PBArray *)array;
- (PBArray *)filteredArrayUsingPredicate:(NSPredicate *)predicate;
@end

@interface PBArray (PBArrayCreation)

+ (instancetype)arrayWithValueType:(PBArrayValueType)valueType;
+ (instancetype)arrayWithValues:(const void *)values count:(NSUInteger)count valueType:(PBArrayValueType)valueType;
+ (instancetype)arrayWithArray:(NSArray *)array valueType:(PBArrayValueType)valueType;
- (instancetype)initWithValueType:(PBArrayValueType)valueType;
- (instancetype)initWithValues:(const void *)values count:(NSUInteger)count valueType:(PBArrayValueType)valueType;
- (instancetype)initWithArray:(NSArray *)array valueType:(PBArrayValueType)valueType;

@end

// PBAppendableArray extends PBArray with the ability to append new values to
// the end of the array.
@interface PBAppendableArray : PBArray

- (void)addBool:(BOOL)value;
- (void)addInt32:(SInt32)value;
- (void)addUint32:(UInt32)value;
- (void)addInt64:(SInt64)value;
- (void)addUint64:(UInt64)value;
- (void)addFloat:(Float32)value;
- (void)addDouble:(Float64)value;
- (void)addEnum:(SInt32)value;

- (void)appendArray:(PBArray *)array;
- (void)appendValues:(const void *)values count:(UInt32)count;

@end
