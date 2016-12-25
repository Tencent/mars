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

#import "AbstractMessage.h"

#import "CodedOutputStream.h"

@implementation PBAbstractMessage

- (instancetype) init {
  if ((self = [super init])) {
  }

  return self;
}


- (NSData*) data {
  NSMutableData* data = [NSMutableData dataWithLength:self.serializedSize];
  PBCodedOutputStream* stream = [PBCodedOutputStream streamWithData:data];
  [self writeToCodedOutputStream:stream];
  return data;
}


- (BOOL) isInitialized {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (SInt32) serializedSize {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (void) writeToCodedOutputStream:(PBCodedOutputStream*) output {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (void) writeToOutputStream:(NSOutputStream*) output {
  PBCodedOutputStream* codedOutput = [PBCodedOutputStream streamWithOutputStream:output];
  [self writeToCodedOutputStream:codedOutput];
  [codedOutput flush];
}


- (instancetype) defaultInstance {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (PBUnknownFieldSet*) unknownFields {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (id<PBMessageBuilder>) builder {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (id<PBMessageBuilder>) toBuilder {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (void) writeDescriptionTo:(NSMutableString*) output
                 withIndent:(NSString*) indent {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


- (NSString*) description {
  NSMutableString* output = [NSMutableString string];
  [self writeDescriptionTo:output withIndent:@""];
  return output;
}

- (void) storeInDictionary: (NSMutableDictionary *) dic {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}

- (NSDictionary *) dictionaryRepresentation {
  @throw [NSException exceptionWithName:@"ImproperSubclassing" reason:@"" userInfo:nil];
}


@end
