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

#import "UnknownFieldSet.h"

#import "CodedInputStream.h"
#import "CodedOutputStream.h"
#import "Field.h"
#import "UnknownFieldSetBuilder.h"

@interface PBUnknownFieldSet()
@property (strong) NSDictionary* fields;
@end


@implementation PBUnknownFieldSet

static PBUnknownFieldSet* defaultInstance = nil;

+ (void) initialize {
  if (self == [PBUnknownFieldSet class]) {
      defaultInstance = [PBUnknownFieldSet setWithFields:[NSMutableDictionary dictionary]];
  }
}


@synthesize fields;




+ (PBUnknownFieldSet*) defaultInstance {
  return defaultInstance;
}


- (instancetype) initWithFields:(NSMutableDictionary*) fields_ {
  if ((self = [super init])) {
    self.fields = fields_;
  }

  return self;
}


+ (PBUnknownFieldSet*) setWithFields:(NSMutableDictionary*) fields {
    return [[PBUnknownFieldSet alloc] initWithFields:fields];
}


- (BOOL) hasField:(SInt32) number {
  return [fields objectForKey:@(number)] != nil;
}


- (PBField*) getField:(SInt32) number {
  PBField* result = [fields objectForKey:@(number)];
  return (result == nil) ? [PBField defaultInstance] : result;
}


- (void) writeToCodedOutputStream:(PBCodedOutputStream*) output {
  NSArray* sortedKeys = [fields.allKeys sortedArrayUsingSelector:@selector(compare:)];
  for (NSNumber* number in sortedKeys) {
    PBField* value = [fields objectForKey:number];
    [value writeTo:(SInt32)number.integerValue output:output];
  }
}


- (void) writeToOutputStream:(NSOutputStream*) output {
  PBCodedOutputStream* codedOutput = [PBCodedOutputStream streamWithOutputStream:output];
  [self writeToCodedOutputStream:codedOutput];
  [codedOutput flush];
}


- (void) writeDescriptionTo:(NSMutableString*) output
                 withIndent:(NSString *)indent {
  NSArray* sortedKeys = [fields.allKeys sortedArrayUsingSelector:@selector(compare:)];
  for (NSNumber* number in sortedKeys) {
    PBField* value = [fields objectForKey:number];
    [value writeDescriptionFor:(SInt32)number.integerValue to:output withIndent:indent];
  }
}

- (void) storeInDictionary: (NSMutableDictionary *) dic;
{
    //TODO: Ignore unknown field sets for now :D
}

+ (PBUnknownFieldSet*) parseFromCodedInputStream:(PBCodedInputStream*) input {
  return [[[PBUnknownFieldSet builder] mergeFromCodedInputStream:input] build];
}


+ (PBUnknownFieldSet*) parseFromData:(NSData*) data {
  return [[[PBUnknownFieldSet builder] mergeFromData:data] build];
}


+ (PBUnknownFieldSet*) parseFromInputStream:(NSInputStream*) input {
  return [[[PBUnknownFieldSet builder] mergeFromInputStream:input] build];
}


+ (PBUnknownFieldSetBuilder*) builder {
    return [[PBUnknownFieldSetBuilder alloc] init];
}


+ (PBUnknownFieldSetBuilder*) builderWithUnknownFields:(PBUnknownFieldSet*) copyFrom {
  return [[PBUnknownFieldSet builder] mergeUnknownFields:copyFrom];
}


/** Get the number of bytes required to encode this set. */
- (SInt32) serializedSize {
  SInt32 result = 0;
  for (NSNumber* number in fields) {
    result += [[fields objectForKey:number] getSerializedSize:(SInt32)number.integerValue];
  }
  return result;
}

/**
 * Serializes the set and writes it to {@code output} using
 * {@code MessageSet} wire format.
 */
- (void) writeAsMessageSetTo:(PBCodedOutputStream*) output {
  for (NSNumber* number in fields) {
    [[fields objectForKey:number] writeAsMessageSetExtensionTo:(SInt32)number.integerValue output:output];
  }
}


/**
 * Get the number of bytes required to encode this set using
 * {@code MessageSet} wire format.
 */
- (SInt32) serializedSizeAsMessageSet {
  SInt32 result = 0;
  for (NSNumber* number in fields) {
    result += [[fields objectForKey:number] getSerializedSizeAsMessageSetExtension:(SInt32)number.integerValue];
  }
  return result;
}


/**
 * Serializes the message to a {@code ByteString} and returns it. This is
 * just a trivial wrapper around {@link #writeTo(PBCodedOutputStream)}.
 */
- (NSData*) data {
  NSMutableData* data = [NSMutableData dataWithLength:self.serializedSize];
  PBCodedOutputStream* output = [PBCodedOutputStream streamWithData:data];

  [self writeToCodedOutputStream:output];
  return data;
}

@end
