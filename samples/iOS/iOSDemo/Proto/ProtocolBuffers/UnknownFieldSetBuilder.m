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

#import "UnknownFieldSetBuilder.h"

#import "CodedInputStream.h"
#import "Field.h"
#import "MutableField.h"
#import "UnknownFieldSet.h"
#import "WireFormat.h"

@interface PBUnknownFieldSetBuilder ()
@property (strong) NSMutableDictionary* fields;
@property SInt32 lastFieldNumber;
@property (strong) PBMutableField* lastField;
@end


@implementation PBUnknownFieldSetBuilder

@synthesize fields;
@synthesize lastFieldNumber;
@synthesize lastField;


- (instancetype) init {
  if ((self = [super init])) {
    self.fields = [NSMutableDictionary dictionary];
  }
  return self;
}


+ (PBUnknownFieldSetBuilder*) createBuilder:(PBUnknownFieldSet*) unknownFields {
    PBUnknownFieldSetBuilder* builder = [[PBUnknownFieldSetBuilder alloc] init];
  [builder mergeUnknownFields:unknownFields];
  return builder;
}


/**
 * Add a field to the {@code PBUnknownFieldSet}.  If a field with the same
 * number already exists, it is removed.
 */
- (PBUnknownFieldSetBuilder*) addField:(PBField*) field forNumber:(SInt32) number {
  if (number == 0) {
    @throw [NSException exceptionWithName:@"IllegalArgument" reason:@"" userInfo:nil];
  }
  if (lastField != nil && lastFieldNumber == number) {
    // Discard this.
    self.lastField = nil;
    lastFieldNumber = 0;
  }
  [fields setObject:field forKey:@(number)];
  return self;
}


/**
 * Get a field builder for the given field number which includes any
 * values that already exist.
 */
- (PBMutableField*) getFieldBuilder:(SInt32) number {
  if (lastField != nil) {
    if (number == lastFieldNumber) {
      return lastField;
    }
    // Note:  addField() will reset lastField and lastFieldNumber.
    [self addField:lastField forNumber:lastFieldNumber];
  }
  if (number == 0) {
    return nil;
  } else {
    PBField* existing = [fields objectForKey:@(number)];
    lastFieldNumber = number;
    self.lastField = [PBMutableField field];
    if (existing != nil) {
      [lastField mergeFromField:existing];
    }
    return lastField;
  }
}


- (PBUnknownFieldSet*) build {
  [self getFieldBuilder:0];  // Force lastField to be built.
  PBUnknownFieldSet* result;
  if (fields.count == 0) {
    result = [PBUnknownFieldSet defaultInstance];
  } else {
    result = [PBUnknownFieldSet setWithFields:fields];
  }
  self.fields = nil;
  return result;
}

- (PBUnknownFieldSet*) buildPartial {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (PBUnknownFieldSet*) clone {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (BOOL) isInitialized {
  return YES;
}

- (PBUnknownFieldSet*) defaultInstance {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (PBUnknownFieldSet*) unknownFields {
  return [self build];
}

- (id<PBMessageBuilder>) setUnknownFields:(PBUnknownFieldSet*) unknownFields {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

/** Check if the given field number is present in the set. */
- (BOOL) hasField:(SInt32) number {
  if (number == 0) {
    @throw [NSException exceptionWithName:@"IllegalArgument" reason:@"" userInfo:nil];
  }

  return number == lastFieldNumber || ([fields objectForKey:@(number)] != nil);
}


/**
 * Add a field to the {@code PBUnknownFieldSet}.  If a field with the same
 * number already exists, the two are merged.
 */
- (PBUnknownFieldSetBuilder*) mergeField:(PBField*) field forNumber:(SInt32) number {
  if (number == 0) {
    @throw [NSException exceptionWithName:@"IllegalArgument" reason:@"" userInfo:nil];
  }
  if ([self hasField:number]) {
    [[self getFieldBuilder:number] mergeFromField:field];
  } else {
    // Optimization:  We could call getFieldBuilder(number).mergeFrom(field)
    // in this case, but that would create a copy of the PBField object.
    // We'd rather reuse the one passed to us, so call addField() instead.
    [self addField:field forNumber:number];
  }

  return self;
}


- (PBUnknownFieldSetBuilder*) mergeUnknownFields:(PBUnknownFieldSet*) other {
  if (other != [PBUnknownFieldSet defaultInstance]) {
    for (NSNumber* number in other.fields) {
      PBField* field = [other.fields objectForKey:number];
      [self mergeField:field forNumber:(SInt32)[number integerValue]];
    }
  }
  return self;
}


- (PBUnknownFieldSetBuilder*) mergeFromData:(NSData*) data {
  PBCodedInputStream* input = [PBCodedInputStream streamWithData:data];
  [self mergeFromCodedInputStream:input];
  [input checkLastTagWas:0];
  return self;
}


- (PBUnknownFieldSetBuilder*) mergeFromData:(NSData*) data extensionRegistry:(PBExtensionRegistry*) extensionRegistry {
  PBCodedInputStream* input = [PBCodedInputStream streamWithData:data];
  [self mergeFromCodedInputStream:input extensionRegistry:extensionRegistry];
  [input checkLastTagWas:0];
  return self;
}


- (PBUnknownFieldSetBuilder*) mergeFromInputStream:(NSInputStream*) input {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (PBUnknownFieldSetBuilder*) mergeFromInputStream:(NSInputStream*) input extensionRegistry:(PBExtensionRegistry*) extensionRegistry {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (PBUnknownFieldSetBuilder*) mergeVarintField:(SInt32) number value:(SInt32) value {
  if (number == 0) {
    @throw [NSException exceptionWithName:@"IllegalArgument" reason:@"Zero is not a valid field number." userInfo:nil];
  }

  [[self getFieldBuilder:number] addVarint:value];
  return self;
}


/**
 * Parse a single field from {@code input} and merge it into this set.
 * @param tag The field's tag number, which was already parsed.
 * @return {@code NO} if the tag is an engroup tag.
 */
- (BOOL) mergeFieldFrom:(SInt32) tag input:(PBCodedInputStream*) input {
  SInt32 number = PBWireFormatGetTagFieldNumber(tag);
  switch (PBWireFormatGetTagWireType(tag)) {
    case PBWireFormatVarint:
      [[self getFieldBuilder:number] addVarint:[input readInt64]];
      return YES;
    case PBWireFormatFixed64:
      [[self getFieldBuilder:number] addFixed64:[input readFixed64]];
      return YES;
    case PBWireFormatLengthDelimited:
      [[self getFieldBuilder:number] addLengthDelimited:[input readData]];
      return YES;
    case PBWireFormatStartGroup: {
      PBUnknownFieldSetBuilder* subBuilder = [PBUnknownFieldSet builder];
      [input readUnknownGroup:number builder:subBuilder];
      [[self getFieldBuilder:number] addGroup:[subBuilder build]];
      return YES;
    }
    case PBWireFormatEndGroup:
      return NO;
    case PBWireFormatFixed32:
      [[self getFieldBuilder:number] addFixed32:[input readFixed32]];
      return YES;
    default:
      @throw [NSException exceptionWithName:@"InvalidProtocolBuffer" reason:@"" userInfo:nil];
  }
}


/**
 * Parse an entire message from {@code input} and merge its fields into
 * this set.
 */
- (PBUnknownFieldSetBuilder*) mergeFromCodedInputStream:(PBCodedInputStream*) input {
  while (YES) {
    SInt32 tag = [input readTag];
    if (tag == 0 || ![self mergeFieldFrom:tag input:input]) {
      break;
    }
  }
  return self;
}

- (PBUnknownFieldSetBuilder*) mergeFromCodedInputStream:(PBCodedInputStream*) input extensionRegistry:(PBExtensionRegistry*) extensionRegistry {
  @throw [NSException exceptionWithName:@"UnsupportedMethod" reason:@"" userInfo:nil];
}

- (PBUnknownFieldSetBuilder*) clear {
  self.fields = [NSMutableDictionary dictionary];
  self.lastFieldNumber = 0;
  self.lastField = nil;
  return self;
}

@end
