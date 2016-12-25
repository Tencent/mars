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

/**
 * All generated protocol message classes extend this class.  This class
 * implements most of the Message and Builder interfaces using Java reflection.
 * Users can ignore this class and pretend that generated messages implement
 * the Message interface directly.
 *
 * @author Cyrus Najmabadi
 */
@class PBExtensionRegistry;
@class PBCodedInputStream;
@protocol GeneratedMessageProtocol <PBMessage>
+ (id<GeneratedMessageProtocol>) parseFromData:(NSData*) data;
+ (id<GeneratedMessageProtocol>) parseFromData:(NSData*) data extensionRegistry:(PBExtensionRegistry*)extensionRegistry;
+ (id<GeneratedMessageProtocol>) parseFromInputStream:(NSInputStream*) input;
+ (id<GeneratedMessageProtocol>) parseFromInputStream:(NSInputStream*) input extensionRegistry:(PBExtensionRegistry*) extensionRegistry;
+ (id<GeneratedMessageProtocol>) parseFromCodedInputStream:(PBCodedInputStream*) input;
+ (id<GeneratedMessageProtocol>) parseFromCodedInputStream:(PBCodedInputStream*) input extensionRegistry:(PBExtensionRegistry*) extensionRegistry;
@end

@interface PBGeneratedMessage : PBAbstractMessage {
@private
  PBUnknownFieldSet* unknownFields;

@protected
  SInt32 memoizedSerializedSize;
}

@end
