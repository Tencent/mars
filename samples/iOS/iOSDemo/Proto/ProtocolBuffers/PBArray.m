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

#import "PBArray.h"

NSString * const PBArrayTypeMismatchException = @"PBArrayTypeMismatchException";
NSString * const PBArrayNumberExpectedException = @"PBArrayNumberExpectedException";
NSString * const PBArrayAllocationFailureException = @"PBArrayAllocationFailureException";

#pragma mark NSNumber Setters

typedef void (*PBArrayValueSetter)(NSNumber *number, void *value);

static void PBArraySetBoolValue(NSNumber *number, void *value)
{
	*((BOOL *)value) = [number charValue];
}

static void PBArraySetInt32Value(NSNumber *number, void *value)
{
	*((SInt32 *)value) = (SInt32)[number integerValue];
}

static void PBArraySetUInt32Value(NSNumber *number, void *value)
{
	*((UInt32 *)value) = (UInt32)[number unsignedIntegerValue];
}

static void PBArraySetInt64Value(NSNumber *number, void *value)
{
	*((SInt64 *)value) = (SInt64)[number longLongValue];
}

static void PBArraySetUInt64Value(NSNumber *number, void *value)
{
	*((UInt64 *)value) = (UInt64)[number unsignedLongLongValue];
}

static void PBArraySetFloatValue(NSNumber *number, void *value)
{
	*((Float32 *)value) = [number floatValue];
}

static void PBArraySetDoubleValue(NSNumber *number, void *value)
{
	*((Float64 *)value) = [number doubleValue];
}

#pragma mark Array Value Types

typedef struct _PBArrayValueTypeInfo
{
	const size_t size;
	const PBArrayValueSetter setter;
    
} PBArrayValueTypeInfo;

static PBArrayValueTypeInfo PBValueTypes[] =
{
	{ sizeof(BOOL),		PBArraySetBoolValue		},
	{ sizeof(SInt32),	PBArraySetInt32Value	},
	{ sizeof(UInt32),	PBArraySetUInt32Value	},
	{ sizeof(SInt64),	PBArraySetInt64Value	},
	{ sizeof(UInt64),	PBArraySetUInt64Value	},
	{ sizeof(Float32),	PBArraySetFloatValue	},
	{ sizeof(Float64),	PBArraySetDoubleValue	},
};

#define PBArrayValueTypeSize(type)		PBValueTypes[type].size
#define PBArrayValueTypeSetter(type)	PBValueTypes[type].setter

#pragma mark Helper Macros

#define PBArraySlot(index) (_data + (index * PBArrayValueTypeSize(_valueType)))

#define PBArrayForEachObject(__data, __count, x) \
	if (_valueType == PBArrayValueTypeObject) \
		for (NSUInteger i = 0; i < __count; ++i) { id object = ((id *)__data)[i]; [object x]; }

#define PBArrayValueTypeAssert(type) \
	if (__builtin_expect(_valueType != type, 0)) \
		[NSException raise:PBArrayTypeMismatchException \
					format:@"array value type mismatch (expected '%s')", #type];

#define PBArrayValueRangeAssert(index) \
	if (__builtin_expect(index >= _count, 0)) \
		[NSException raise:NSRangeException format: @"index (%lu) beyond bounds (%lu)", (unsigned long)index, (unsigned long)_count];

#define PBArrayNumberAssert(value) \
	if (__builtin_expect(![value isKindOfClass:[NSNumber class]], 0)) \
		[NSException raise:PBArrayNumberExpectedException format:@"NSNumber expected (got '%@')", [value class]];

#define PBArrayAllocationAssert(p, size) \
	if (__builtin_expect(p == NULL, 0)) \
		[NSException raise:PBArrayAllocationFailureException format:@"failed to allocate %lu bytes", size];

#pragma mark -
#pragma mark PBArray

@implementation PBArray

@synthesize valueType = _valueType;
@dynamic data;

- (instancetype)initWithCount:(NSUInteger)count valueType:(PBArrayValueType)valueType
{
	if ((self = [super init]))
	{
		_valueType = valueType;
		_count = count;
		_capacity = count;
        
		if (_capacity)
		{
            _data = malloc(_capacity * PBArrayValueTypeSize(_valueType));
            if (_data == NULL)
            {
                self = nil;
            }
		}
	}
    
	return self;
}

- (instancetype)copyWithZone:(NSZone *)zone
{
	PBArray *copy = [[[self class] allocWithZone:zone] initWithCount:_count valueType:_valueType];
	if (copy)
	{
		memcpy(copy->_data, _data, _count * PBArrayValueTypeSize(_valueType));
	}

	return copy;
}

- (void)dealloc
{
    if (_data)
    {
        free(_data);
    }
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"<%@ %p>{valueType = %d, count = %ld, capacity = %ld, data = %p}",
			[self class], self, _valueType, (long)_count, (long)_capacity, _data];
}

- (NSUInteger)count
{
	return _count;
}

- (const void *)data
{
	return _data;
}

- (BOOL)boolAtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeBool);
	return ((BOOL *)_data)[index];
}

- (SInt32)int32AtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert((unsigned long)index);
	PBArrayValueTypeAssert(PBArrayValueTypeInt32);
	return ((SInt32 *)_data)[index];
}

- (UInt32)uint32AtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert((unsigned long)index);
	PBArrayValueTypeAssert(PBArrayValueTypeUInt32);
	return ((UInt32 *)_data)[index];
}

- (SInt32)enumAtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeInt32);
	return ((SInt32 *)_data)[index];
}


- (SInt64)int64AtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeInt64);
	return ((SInt64 *)_data)[index];
}

- (UInt64)uint64AtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeUInt64);
	return ((UInt64 *)_data)[index];
}

- (Float32)floatAtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeFloat);
	return ((Float32 *)_data)[index];
}

- (Float64)doubleAtIndex:(NSUInteger)index
{
	PBArrayValueRangeAssert(index);
	PBArrayValueTypeAssert(PBArrayValueTypeDouble);
	return ((Float64 *)_data)[index];
}

- (BOOL)isEqualToArray:(PBArray *)array
{
	if (self == array)
	{
		return YES;
	}
	else if (array->_count != _count)
	{
		return NO;
	}
	else
	{
		return memcmp(array->_data, _data, _count * PBArrayValueTypeSize(_valueType)) == 0;
	}
}

- (BOOL)isEqual:(id)object
{
	BOOL equal = NO;
	if ([object isKindOfClass:[PBArray class]])
	{
		equal = [self isEqualToArray:object];
	}
	return equal;
}

-(id)firstObject
{
    if (self.count>0) {
        return [self objectAtIndexedSubscript:0];
    }
    return nil;
}

-(id)lastObject
{
    if (self.count>0) {
        return [self objectAtIndexedSubscript:self.count - 1];
    }
    return nil;
}

- (id)objectAtIndexedSubscript:(NSUInteger)idx
{
 
    if (idx < self.count) {
        if (PBArrayValueTypeBool == _valueType)
        {
            return [NSNumber numberWithBool:[self boolAtIndex:idx]];
        }
        else if (PBArrayValueTypeInt32 == _valueType)
        {
            return [NSNumber numberWithLong:[self int32AtIndex:idx]];
        }
        else if (PBArrayValueTypeInt64 == _valueType)
        {
            return [NSNumber numberWithLongLong:[self int64AtIndex:idx]];
        }
        else if (PBArrayValueTypeUInt32 == _valueType)
        {
            return [NSNumber numberWithUnsignedLong:[self uint32AtIndex:idx]];
        }
        else if (PBArrayValueTypeUInt64 == _valueType)
        {
            return [NSNumber numberWithUnsignedLongLong:[self uint64AtIndex:idx]];
        }
        else if (_valueType == PBArrayValueTypeFloat)
        {
            return [NSNumber numberWithFloat:[self floatAtIndex:idx]];
        }
        else if (_valueType == PBArrayValueTypeDouble)
        {
            return [NSNumber numberWithDouble:[self doubleAtIndex:idx]];
        }
    }
    return nil;

}



- (void)enumerateObjectsUsingBlock:(void (^)(id obj, NSUInteger idx, BOOL *stop))block
{
    if (!block) return;
    BOOL stop = NO;
    for (NSInteger i = 0; i < self.count; i++) {
        block([self objectAtIndexedSubscript:i],i,&stop);
        if(stop){
            break;
        }
    }
}

-(NSUInteger)indexOfObjectPassingTest:(BOOL (^)(id obj, NSUInteger, BOOL *stop))predicate
{
    if (!predicate) return NSNotFound;
    __block BOOL bb;
    __block NSUInteger index;
    [self enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
       
        bb = predicate(obj,idx,stop);
        if (bb) {
            index = idx;
        }
    }];
    if (bb) {
        return index;
    }
    return NSNotFound;
}

@end

@implementation PBArray (PBArrayExtended)

-(PBArray *) filteredArrayUsingPredicate:(NSPredicate *)predicate
{
    __block PBAppendableArray *newArray = [[PBAppendableArray alloc] initWithValueType:_valueType];
    [self enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {

        BOOL result = [predicate evaluateWithObject:obj];
        if (result)
        {
            if (PBArrayValueTypeBool == _valueType)
            {
                [newArray addBool:[(NSNumber *)obj boolValue]];
            }
            else if (PBArrayValueTypeInt32 == _valueType)
            {
                [newArray addInt32:(SInt32)[(NSNumber *)obj longValue]];
            }
            else if (PBArrayValueTypeInt64 == _valueType)
            {
                [newArray addInt64:[(NSNumber *)obj longLongValue]];
            }
            else if (PBArrayValueTypeUInt32 == _valueType)
            {
                 [newArray addUint32:(UInt32)[(NSNumber *)obj unsignedLongValue]];
            }
            else if (PBArrayValueTypeUInt64 == _valueType)
            {
                [newArray addUint64:[(NSNumber *)obj unsignedLongLongValue]];
            }
            else if (_valueType == PBArrayValueTypeFloat)
            {
                 [newArray addFloat:[(NSNumber *)obj floatValue]];
            }
            else if (_valueType == PBArrayValueTypeDouble)
            {
                 [newArray addDouble:[(NSNumber *)obj doubleValue]];
            }

            
        }
    }];
    return newArray;
}

- (id)arrayByAppendingArray:(PBArray *)array
{
	PBArrayValueTypeAssert(array.valueType);

	PBArray *result = [[[self class] alloc] initWithCount:_count + array.count valueType:_valueType];
	if (result)
	{
		const size_t elementSize = PBArrayValueTypeSize(_valueType);
		const size_t originalSize = _count * elementSize;

		memcpy(result->_data, _data, originalSize);
		memcpy(result->_data + originalSize, array.data, array.count * elementSize);
	}

	return result;
}

@end

@implementation PBArray (PBArrayCreation)

+ (instancetype)arrayWithValueType:(PBArrayValueType)valueType
{
	return [[self alloc] initWithValueType:valueType];
}

+ (instancetype)arrayWithValues:(const void *)values count:(NSUInteger)count valueType:(PBArrayValueType)valueType
{
	return [[self alloc] initWithValues:values count:count valueType:valueType];
}

+ (instancetype)arrayWithArray:(NSArray *)array valueType:(PBArrayValueType)valueType
{
	return [[self alloc] initWithArray:array valueType:valueType];
}

- (instancetype)initWithValueType:(PBArrayValueType)valueType
{
	return [self initWithCount:0 valueType:valueType];
}

- (instancetype)initWithValues:(const void *)values count:(NSUInteger)count valueType:(PBArrayValueType)valueType
{
	if ((self = [self initWithCount:count valueType:valueType]))
	{
		memcpy(_data, values, count * PBArrayValueTypeSize(_valueType));
	}

	return self;
}

- (instancetype)initWithArray:(NSArray *)array valueType:(PBArrayValueType)valueType
{
	if ((self = [self initWithCount:[array count] valueType:valueType]))
	{
		const size_t elementSize = PBArrayValueTypeSize(valueType);
		size_t offset = 0;

        PBArrayValueSetter setter = PBArrayValueTypeSetter(valueType);
        for (id object in array)
        {
            PBArrayNumberAssert(object);
            setter((NSNumber *)object, _data + offset);
            offset += elementSize;
        }

	}

	return self;
}

@end

#pragma mark -
#pragma mark PBAppendableArray

@implementation PBAppendableArray

- (void)ensureAdditionalCapacity:(NSUInteger)additionalSlots
{
	const NSUInteger requiredSlots = _count + additionalSlots;

	if (requiredSlots > _capacity)
	{
		// If we haven't allocated any capacity yet, simply reserve
		// enough capacity to cover the required number of slots.
		if (_capacity == 0)
		{
			_capacity = requiredSlots;
		}
		else
		{
			// Otherwise, continue to double our capacity until we
			// can accomodate the required number of slots.
			while (_capacity < requiredSlots)
			{
				_capacity *= 2;
			}
		}

		const size_t size = _capacity * PBArrayValueTypeSize(_valueType);
		_data = reallocf(_data, size);
		PBArrayAllocationAssert(_data, size);
	}
}



- (void)addBool:(BOOL)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeBool);
	[self ensureAdditionalCapacity:1];
	*(BOOL *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addInt32:(SInt32)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeInt32);
	[self ensureAdditionalCapacity:1];
	*(SInt32 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addUint32:(UInt32)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeUInt32);
	[self ensureAdditionalCapacity:1];
	*(UInt32 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addEnum:(SInt32)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeInt32);
	[self ensureAdditionalCapacity:1];
	*(SInt32 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addInt64:(int64_t)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeInt64);
	[self ensureAdditionalCapacity:1];
	*(SInt64 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addUint64:(uint64_t)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeUInt64);
	[self ensureAdditionalCapacity:1];
	*(UInt64 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addFloat:(Float32)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeFloat);
	[self ensureAdditionalCapacity:1];
	*(Float32 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)addDouble:(Float64)value
{
	PBArrayValueTypeAssert(PBArrayValueTypeDouble);
	[self ensureAdditionalCapacity:1];
	*(Float64 *)PBArraySlot(_count) = value;
	_count++;
}

- (void)appendArray:(PBArray *)array
{
	PBArrayValueTypeAssert(array.valueType);
	[self ensureAdditionalCapacity:array.count];

	const size_t elementSize = PBArrayValueTypeSize(_valueType);
	memcpy(_data + (_count * elementSize), array.data, array.count * elementSize);
	_count += array.count;
}

- (void)appendValues:(const void *)values count:(UInt32)count
{
	[self ensureAdditionalCapacity:count];

	const size_t elementSize = PBArrayValueTypeSize(_valueType);
	memcpy(_data + (_count * elementSize), values, count * elementSize);
	_count += count;
}

@end
