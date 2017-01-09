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

#import "ConcreteExtensionField.h"

#import "AbstractMessage.h"
#import "CodedInputStream.h"
#import "CodedOutputStream.h"
#import "ExtendableMessageBuilder.h"
#import "MessageBuilder.h"
#import "Utilities.h"
#import "WireFormat.h"

@interface PBConcreteExtensionField()
@property PBExtensionType type;
@property (assign) Class extendedClass;
@property SInt32 fieldNumber;
@property (strong) id defaultValue;
@property (assign) Class messageOrGroupClass;
@property BOOL isRepeated;
@property BOOL isPacked;
@property BOOL isMessageSetWireFormat;
@end

@implementation PBConcreteExtensionField

@synthesize type;
@synthesize extendedClass;
@synthesize fieldNumber;
@synthesize defaultValue;
@synthesize messageOrGroupClass;
@synthesize isRepeated;
@synthesize isPacked;
@synthesize isMessageSetWireFormat;



- (instancetype) initWithType:(PBExtensionType) type_
          extendedClass:(Class) extendedClass_
            fieldNumber:(SInt32) fieldNumber_
           defaultValue:(id) defaultValue_
    messageOrGroupClass:(Class) messageOrGroupClass_
             isRepeated:(BOOL) isRepeated_
               isPacked:(BOOL) isPacked_
    isMessageSetWireFormat:(BOOL) isMessageSetWireFormat_ {
  if ((self = [super init])) {
    self.type = type_;
    self.extendedClass = extendedClass_;
    self.fieldNumber = fieldNumber_;
    self.defaultValue = defaultValue_;
    self.messageOrGroupClass = messageOrGroupClass_;
    self.isRepeated = isRepeated_;
    self.isPacked = isPacked_;
    self.isMessageSetWireFormat = isMessageSetWireFormat_;
  }

  return self;
}


+ (PBConcreteExtensionField*) extensionWithType:(PBExtensionType) type
                                extendedClass:(Class) extendedClass
                                  fieldNumber:(SInt32) fieldNumber
                                 defaultValue:(id) defaultValue
                    messageOrGroupClass:(Class) messageOrGroupClass
                                   isRepeated:(BOOL) isRepeated
                                     isPacked:(BOOL) isPacked
                       isMessageSetWireFormat:(BOOL) isMessageSetWireFormat {
  return [[PBConcreteExtensionField alloc] initWithType:type
                                         extendedClass:extendedClass
                                           fieldNumber:fieldNumber
                                          defaultValue:defaultValue
                             messageOrGroupClass:messageOrGroupClass
                                            isRepeated:isRepeated
                                              isPacked:isPacked
                                 isMessageSetWireFormat:isMessageSetWireFormat];
}


- (PBWireFormat) wireType {
  if (isPacked) {
    return PBWireFormatLengthDelimited;
  }

  switch (type) {
    case PBExtensionTypeBool:     return PBWireFormatVarint;
    case PBExtensionTypeFixed32:  return PBWireFormatFixed32;
    case PBExtensionTypeSFixed32: return PBWireFormatFixed32;
    case PBExtensionTypeFloat:    return PBWireFormatFixed32;
    case PBExtensionTypeFixed64:  return PBWireFormatFixed64;
    case PBExtensionTypeSFixed64: return PBWireFormatFixed64;
    case PBExtensionTypeDouble:   return PBWireFormatFixed64;
    case PBExtensionTypeInt32:    return PBWireFormatVarint;
    case PBExtensionTypeInt64:    return PBWireFormatVarint;
    case PBExtensionTypeSInt32:   return PBWireFormatVarint;
    case PBExtensionTypeSInt64:   return PBWireFormatVarint;
    case PBExtensionTypeUInt32:   return PBWireFormatVarint;
    case PBExtensionTypeUInt64:   return PBWireFormatVarint;
    case PBExtensionTypeBytes:    return PBWireFormatLengthDelimited;
    case PBExtensionTypeString:   return PBWireFormatLengthDelimited;
    case PBExtensionTypeMessage:  return PBWireFormatLengthDelimited;
    case PBExtensionTypeGroup:    return PBWireFormatStartGroup;
    case PBExtensionTypeEnum:     return PBWireFormatVarint;
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


BOOL typeIsFixedSize(PBExtensionType type) {
  switch (type) {
    case PBExtensionTypeBool:
    case PBExtensionTypeFixed32:
    case PBExtensionTypeSFixed32:
    case PBExtensionTypeFloat:
    case PBExtensionTypeFixed64:
    case PBExtensionTypeSFixed64:
    case PBExtensionTypeDouble:
      return YES;
    default:
      return NO;
  }
}


SInt32 typeSize(PBExtensionType type) {
  switch (type) {
    case PBExtensionTypeBool:
      return 1;
    case PBExtensionTypeFixed32:
    case PBExtensionTypeSFixed32:
    case PBExtensionTypeFloat:
      return 4;
    case PBExtensionTypeFixed64:
    case PBExtensionTypeSFixed64:
    case PBExtensionTypeDouble:
      return 8;
    default:
      break;
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (void)           writeSingleValue:(id) value
    includingTagToCodedOutputStream:(PBCodedOutputStream*) output {
  switch (type) {
    case PBExtensionTypeBool:
      [output writeBool:fieldNumber value:[value boolValue]];
      return;
    case PBExtensionTypeFixed32:
      [output writeFixed32:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeSFixed32:
      [output writeSFixed32:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeFloat:
      [output writeFloat:fieldNumber value:[value floatValue]];
      return;
    case PBExtensionTypeFixed64:
      [output writeFixed64:fieldNumber value:[value longLongValue]];
      return;
    case PBExtensionTypeSFixed64:
      [output writeSFixed64:fieldNumber value:[value longLongValue]];
      return;
    case PBExtensionTypeDouble:
      [output writeDouble:fieldNumber value:[value doubleValue]];
      return;
    case PBExtensionTypeInt32:
      [output writeInt32:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeInt64:
      [output writeInt64:fieldNumber value:[value longLongValue]];
      return;
    case PBExtensionTypeSInt32:
      [output writeSInt32:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeSInt64:
      [output writeSInt64:fieldNumber value:[value longLongValue]];
      return;
    case PBExtensionTypeUInt32:
      [output writeUInt32:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeUInt64:
      [output writeUInt64:fieldNumber value:[value longLongValue]];
      return;
    case PBExtensionTypeBytes:
      [output writeData:fieldNumber value:value];
      return;
    case PBExtensionTypeString:
      [output writeString:fieldNumber value:value];
      return;
    case PBExtensionTypeGroup:
      [output writeGroup:fieldNumber value:value];
      return;
    case PBExtensionTypeEnum:
      [output writeEnum:fieldNumber value:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeMessage:
      if (isMessageSetWireFormat) {
        [output writeMessageSetExtension:fieldNumber value:value];
      } else {
        [output writeMessage:fieldNumber value:value];
      }
      return;
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (void)           writeSingleValue:(id) value
    noTagToCodedOutputStream:(PBCodedOutputStream*) output {
  switch (type) {
    case PBExtensionTypeBool:
      [output writeBoolNoTag:[value boolValue]];
      return;
    case PBExtensionTypeFixed32:
      [output writeFixed32NoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeSFixed32:
      [output writeSFixed32NoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeFloat:
      [output writeFloatNoTag:[value floatValue]];
      return;
    case PBExtensionTypeFixed64:
      [output writeFixed64NoTag:[value longLongValue]];
      return;
    case PBExtensionTypeSFixed64:
      [output writeSFixed64NoTag:[value longLongValue]];
      return;
    case PBExtensionTypeDouble:
      [output writeDoubleNoTag:[value doubleValue]];
      return;
    case PBExtensionTypeInt32:
      [output writeInt32NoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeInt64:
      [output writeInt64NoTag:[value longLongValue]];
      return;
    case PBExtensionTypeSInt32:
      [output writeSInt32NoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeSInt64:
      [output writeSInt64NoTag:[value longLongValue]];
      return;
    case PBExtensionTypeUInt32:
      [output writeUInt32NoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeUInt64:
      [output writeUInt64NoTag:[value longLongValue]];
      return;
    case PBExtensionTypeBytes:
      [output writeDataNoTag:value];
      return;
    case PBExtensionTypeString:
      [output writeStringNoTag:value];
      return;
    case PBExtensionTypeGroup:
      [output writeGroupNoTag:fieldNumber value:value];
      return;
    case PBExtensionTypeEnum:
      [output writeEnumNoTag:(SInt32)[value integerValue]];
      return;
    case PBExtensionTypeMessage:
      [output writeMessageNoTag:value];
      return;
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (SInt32) computeSingleSerializedSizeNoTag:(id) value {
  switch (type) {
    case PBExtensionTypeBool:     return computeBoolSizeNoTag([value boolValue]);
    case PBExtensionTypeFixed32:  return computeFixed32SizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeSFixed32: return computeSFixed32SizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeFloat:    return computeFloatSizeNoTag([value floatValue]);
    case PBExtensionTypeFixed64:  return computeFixed64SizeNoTag([value longLongValue]);
    case PBExtensionTypeSFixed64: return computeSFixed64SizeNoTag([value longLongValue]);
    case PBExtensionTypeDouble:   return computeDoubleSizeNoTag([value doubleValue]);
    case PBExtensionTypeInt32:    return computeInt32SizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeInt64:    return computeInt64SizeNoTag([value longLongValue]);
    case PBExtensionTypeSInt32:   return computeSInt32SizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeSInt64:   return computeSInt64SizeNoTag([value longLongValue]);
    case PBExtensionTypeUInt32:   return computeUInt32SizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeUInt64:   return computeUInt64SizeNoTag([value longLongValue]);
    case PBExtensionTypeBytes:    return computeDataSizeNoTag(value);
    case PBExtensionTypeString:   return computeStringSizeNoTag(value);
    case PBExtensionTypeGroup:    return computeGroupSizeNoTag(value);
    case PBExtensionTypeEnum:     return computeEnumSizeNoTag((SInt32)[value integerValue]);
    case PBExtensionTypeMessage:  return computeMessageSizeNoTag(value);
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (SInt32) computeSingleSerializedSizeIncludingTag:(id) value {
  switch (type) {
    case PBExtensionTypeBool:     return computeBoolSize(fieldNumber, [value boolValue]);
    case PBExtensionTypeFixed32:  return computeFixed32Size(fieldNumber,(SInt32) [value integerValue]);
    case PBExtensionTypeSFixed32: return computeSFixed32Size(fieldNumber, (SInt32)[value integerValue]);
    case PBExtensionTypeFloat:    return computeFloatSize(fieldNumber, [value floatValue]);
    case PBExtensionTypeFixed64:  return computeFixed64Size(fieldNumber, [value longLongValue]);
    case PBExtensionTypeSFixed64: return computeSFixed64Size(fieldNumber, [value longLongValue]);
    case PBExtensionTypeDouble:   return computeDoubleSize(fieldNumber, [value doubleValue]);
    case PBExtensionTypeInt32:    return computeInt32Size(fieldNumber, (SInt32)[value integerValue]);
    case PBExtensionTypeInt64:    return computeInt64Size(fieldNumber, [value longLongValue]);
    case PBExtensionTypeSInt32:   return computeSInt32Size(fieldNumber, (SInt32)[value integerValue]);
    case PBExtensionTypeSInt64:   return computeSInt64Size(fieldNumber, [value longLongValue]);
    case PBExtensionTypeUInt32:   return computeUInt32Size(fieldNumber, (SInt32)[value integerValue]);
    case PBExtensionTypeUInt64:   return computeUInt64Size(fieldNumber, [value longLongValue]);
    case PBExtensionTypeBytes:    return computeDataSize(fieldNumber, value);
    case PBExtensionTypeString:   return computeStringSize(fieldNumber, value);
    case PBExtensionTypeGroup:    return computeGroupSize(fieldNumber, value);
    case PBExtensionTypeEnum:     return computeEnumSize(fieldNumber, (SInt32)[value integerValue]);
    case PBExtensionTypeMessage:
      if (isMessageSetWireFormat) {
        return computeMessageSetExtensionSize(fieldNumber, value);
      } else {
        return computeMessageSize(fieldNumber, value);
      }
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (void) writeDescriptionOfSingleValue:(id) value
                                    to:(NSMutableString*) output
                            withIndent:(NSString*) indent {
  switch (type) {
    case PBExtensionTypeBool:
    case PBExtensionTypeFixed32:
    case PBExtensionTypeSFixed32:
    case PBExtensionTypeFloat:
    case PBExtensionTypeFixed64:
    case PBExtensionTypeSFixed64:
    case PBExtensionTypeDouble:
    case PBExtensionTypeInt32:
    case PBExtensionTypeInt64:
    case PBExtensionTypeSInt32:
    case PBExtensionTypeSInt64:
    case PBExtensionTypeUInt32:
    case PBExtensionTypeUInt64:
    case PBExtensionTypeBytes:
    case PBExtensionTypeString:
    case PBExtensionTypeEnum:
      [output appendFormat:@"%@%@\n", indent, value];
      return;
    case PBExtensionTypeGroup:
    case PBExtensionTypeMessage:
      [((PBAbstractMessage *)value) writeDescriptionTo:output withIndent:indent];
      return;
  }
  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (void)writeRepeatedValues:(NSArray*) values includingTagsToCodedOutputStream:(PBCodedOutputStream*) output {
  if (isPacked) {
    [output writeTag:fieldNumber format:PBWireFormatLengthDelimited];
    SInt32 dataSize = 0;
    if (typeIsFixedSize(type)) {
      dataSize = (SInt32)(values.count * typeSize(type));
    } else {
      for (id value in values) {
        dataSize += [self computeSingleSerializedSizeNoTag:value];
      }
    }
    [output writeRawVarint32:dataSize];
    for (id value in values) {
      [self writeSingleValue:value noTagToCodedOutputStream:output];
    }
  } else {
    for (id value in values) {
      [self writeSingleValue:value includingTagToCodedOutputStream:output];
    }
  }
}


- (void) writeValue:(id) value includingTagToCodedOutputStream:(PBCodedOutputStream*) output {
  if (isRepeated) {
    [self writeRepeatedValues:value includingTagsToCodedOutputStream:output];
  } else {
    [self writeSingleValue:value includingTagToCodedOutputStream:output];
  }
}


- (SInt32) computeRepeatedSerializedSizeIncludingTags:(NSArray*) values {
  if (isPacked) {
    SInt32 size = 0;
    if (typeIsFixedSize(type)) {
      size = (SInt32)(values.count * typeSize(type));
    } else {
      for (id value in values) {
        size += [self computeSingleSerializedSizeNoTag:value];
      }
    }
    return size + computeTagSize(fieldNumber) + computeRawVarint32Size(size);
  } else {
    SInt32 size = 0;
    for (id value in values) {
      size += [self computeSingleSerializedSizeIncludingTag:value];
    }
    return size;
  }
}


- (SInt32) computeSerializedSizeIncludingTag:(id) value {
  if (isRepeated) {
    return [self computeRepeatedSerializedSizeIncludingTags:value];
  } else {
    return [self computeSingleSerializedSizeIncludingTag:value];
  }
}


- (void) writeDescriptionOf:(id)value
                         to:(NSMutableString *)output
                 withIndent:(NSString *)indent {
  if (isRepeated) {
    NSArray* values = value;
    for (id singleValue in values) {
      [self writeDescriptionOfSingleValue:singleValue to:output withIndent:indent];
    }
  } else {
    [self writeDescriptionOfSingleValue:value to:output withIndent:indent];
  }
}

- (id) dictionaryObjectValueForObject: (id) object {
    switch (type) {
        case PBExtensionTypeBool:
        case PBExtensionTypeFixed32:
        case PBExtensionTypeSFixed32:
        case PBExtensionTypeFloat:
        case PBExtensionTypeFixed64:
        case PBExtensionTypeSFixed64:
        case PBExtensionTypeDouble:
        case PBExtensionTypeInt32:
        case PBExtensionTypeInt64:
        case PBExtensionTypeSInt32:
        case PBExtensionTypeSInt64:
        case PBExtensionTypeUInt32:
        case PBExtensionTypeUInt64:
        case PBExtensionTypeBytes:
        case PBExtensionTypeString:
        case PBExtensionTypeEnum:
            return object;
        case PBExtensionTypeGroup:
        case PBExtensionTypeMessage:
        {
            NSMutableDictionary * dic = [NSMutableDictionary new];
            [((PBAbstractMessage *)object) storeInDictionary:dic];
            return dic;
        }
    }
}

- (void) addDictionaryEntriesOf:(id) value
                             to:(NSMutableDictionary*) dictionary {
    if (isRepeated) {
        NSArray* values = value;
        NSMutableArray * arr = [NSMutableArray new];
        for (id singleValue in values) {
            [arr addObject: [self dictionaryObjectValueForObject:singleValue]];
        }
        [dictionary setObject: arr forKey: NSStringFromClass([self extendedClass])];
    } else {
        [dictionary setObject: [self dictionaryObjectValueForObject:value] forKey: NSStringFromClass([self extendedClass])];
    }
}

- (void) mergeMessageSetExtentionFromCodedInputStream:(PBCodedInputStream*) input
                                        unknownFields:(PBUnknownFieldSetBuilder*) unknownFields {
  @throw [NSException exceptionWithName:@"NYI" reason:@"" userInfo:nil];

  // The wire format for MessageSet is:
  //   message MessageSet {
  //     repeated group Item = 1 {
  //       required int32 typeId = 2;
  //       required bytes message = 3;
  //     }
  //   }
  // "typeId" is the extension's field number.  The extension can only be
  // a message type, where "message" contains the encoded bytes of that
  // message.
  //
  // In practice, we will probably never see a MessageSet item in which
  // the message appears before the type ID, or where either field does not
  // appear exactly once.  However, in theory such cases are valid, so we
  // should be prepared to accept them.

  //int typeId = 0;
//  ByteString rawBytes = null;
//
//  while (true) {
//    final int tag = input.readTag();
//    if (tag == 0) {
//      break;
//    }
//
//    if (tag == WireFormat.MESSAGE_SET_TYPE_ID_TAG) {
//      typeId = input.readUInt32();
//      // Zero is not a valid type ID.
//      if (typeId != 0) {
//        if (rawBytes != null) {
//          unknownFields.mergeField(typeId,
//                                   UnknownFieldSet.Field.newBuilder()
//                                   .addLengthDelimited(rawBytes)
//                                   .build());
//          rawBytes = null;
//        }
//      }
//    } else if (tag == WireFormat.MESSAGE_SET_MESSAGE_TAG) {
//      if (typeId == 0) {
//        // We haven't seen a type ID yet, so we have to store the raw bytes
//        // for now.
//        rawBytes = input.readBytes();
//      } else {
//        unknownFields.mergeField(typeId,
//                                 UnknownFieldSet.Field.newBuilder()
//                                 .addLengthDelimited(input.readBytes())
//                                 .build());
//      }
//    } else {
//      // Unknown fieldNumber.  Skip it.
//      if (!input.skipField(tag)) {
//        break;  // end of group
//      }
//    }
//  }
//
//  input.checkLastTagWas(WireFormat.MESSAGE_SET_ITEM_END_TAG);
}


- (id) readSingleValueFromCodedInputStream:(PBCodedInputStream*) input
                         extensionRegistry:(PBExtensionRegistry*) extensionRegistry {
  switch (type) {
    case PBExtensionTypeBool:     return [NSNumber numberWithBool:[input readBool]];
    case PBExtensionTypeFixed32:  return @([input readFixed32]);
    case PBExtensionTypeSFixed32: return @([input readSFixed32]);
    case PBExtensionTypeFloat:    return [NSNumber numberWithFloat:[input readFloat]];
    case PBExtensionTypeFixed64:  return [NSNumber numberWithLongLong:[input readFixed64]];
    case PBExtensionTypeSFixed64: return [NSNumber numberWithLongLong:[input readSFixed64]];
    case PBExtensionTypeDouble:   return [NSNumber numberWithDouble:[input readDouble]];
    case PBExtensionTypeInt32:    return @([input readInt32]);
    case PBExtensionTypeInt64:    return [NSNumber numberWithLongLong:[input readInt64]];
    case PBExtensionTypeSInt32:   return @([input readSInt32]);
    case PBExtensionTypeSInt64:   return [NSNumber numberWithLongLong:[input readSInt64]];
    case PBExtensionTypeUInt32:   return @([input readUInt32]);
    case PBExtensionTypeUInt64:   return [NSNumber numberWithLongLong:[input readUInt64]];
    case PBExtensionTypeBytes:    return [input readData];
    case PBExtensionTypeString:   return [input readString];
    case PBExtensionTypeEnum:     return @([input readEnum]);
    case PBExtensionTypeGroup:
    {
      id<PBMessageBuilder> builder = [messageOrGroupClass builder];
      [input readGroup:fieldNumber builder:builder extensionRegistry:extensionRegistry];
      return [builder build];
    }

    case PBExtensionTypeMessage:
    {
      id<PBMessageBuilder> builder = [messageOrGroupClass builder];
      [input readMessage:builder extensionRegistry:extensionRegistry];
      return [builder build];
    }
  }

  @throw [NSException exceptionWithName:@"InternalError" reason:@"" userInfo:nil];
}


- (void) mergeFromCodedInputStream:(PBCodedInputStream*) input
                     unknownFields:(PBUnknownFieldSetBuilder*) unknownFields
     extensionRegistry:(PBExtensionRegistry*) extensionRegistry
    builder:(PBExtendableMessageBuilder*) builder
                               tag:(SInt32) tag {
  if (isPacked) {
    SInt32 length = [input readRawVarint32];
    SInt32 limit = [input pushLimit:length];
    while ([input bytesUntilLimit] > 0) {
      id value = [self readSingleValueFromCodedInputStream:input extensionRegistry:extensionRegistry];
      [builder addExtension:self value:value];
    }
    [input popLimit:limit];
  } else if (isMessageSetWireFormat) {
    [self mergeMessageSetExtentionFromCodedInputStream:input
                                         unknownFields:unknownFields];
  } else {
    id value = [self readSingleValueFromCodedInputStream:input extensionRegistry:extensionRegistry];
    if (isRepeated) {
      [builder addExtension:self value:value];
    } else {
      [builder setExtension:self value:value];
    }
  }
}


@end
