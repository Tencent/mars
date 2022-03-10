// Copyright 2008 Cyrus Najmabadi
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#import "PBUtility.h"

typedef enum {
  PBWireFormatVarint = 0,
  PBWireFormatFixed64 = 1,
  PBWireFormatLengthDelimited = 2,
  PBWireFormatStartGroup = 3,
  PBWireFormatEndGroup = 4,
  PBWireFormatFixed32 = 5,

  PBWireFormatTagTypeBits = 3,
  PBWireFormatTagTypeMask = 7 /* = (1 << PBWireFormatTagTypeBits) - 1*/,

  PBWireFormatMessageSetItem = 1,
  PBWireFormatMessageSetTypeId = 2,
  PBWireFormatMessageSetMessage = 3
} PBWireFormat;

typedef enum {
    FieldTypeDouble         = 1,
    FieldTypeFloat          = 2,
    FieldTypeInt64          = 3,
    FieldTypeUint64         = 4,
    FieldTypeInt32          = 5,
    FieldTypeFixed64        = 6,
    FieldTypeFixed32        = 7,
    FieldTypeBool           = 8,
    FieldTypeString         = 9,
    FieldTypeGroup          = 10,
    FieldTypeMessage        = 11,
    FieldTypeBytes          = 12,
    FieldTypeUint32         = 13,
    FieldTypeEnum           = 14,
    FieldTypeSfixed32       = 15,
    FieldTypeSfixed64       = 16,
    FieldTypeSint32         = 17,
    FieldTypeSint64         = 18,
    MAX_FIELD_TYPE          = 18
} FieldType;

// fieldType->WriteFormat
extern PBWireFormat kWireTypeForFieldType[MAX_FIELD_TYPE + 1];

#ifdef __cplusplus
extern "C" {
#endif

static inline int32_t PBWireFormatMakeTag(int32_t fieldNumber, int32_t wireType) {
	return (fieldNumber << PBWireFormatTagTypeBits) | wireType;
}

static inline int32_t PBWireFormatMakeTagByFieldType(int32_t fieldNumber, Byte fieldType, BOOL isPacked) {
    if (isPacked) {
        return PBWireFormatMakeTag(fieldNumber, PBWireFormatLengthDelimited);
    } else {
        return PBWireFormatMakeTag(fieldNumber, kWireTypeForFieldType[fieldType]);
    }
}
    
static inline int32_t PBWireFormatGetTagWireType(int32_t tag) {
	return tag & PBWireFormatTagTypeMask;
}

static inline int32_t PBWireFormatGetTagFieldNumber(int32_t tag) {
	return logicalRightShift32(tag, PBWireFormatTagTypeBits);
}
    
static inline int32_t PBWireFormatTagSize(int32_t fieldNumber, int32_t wireType) {
    uint32_t value = (fieldNumber << PBWireFormatTagTypeBits);
        
    if (value < (1 << 7)) {
        return 1;
    } else if (value < (1 << 14)) {
        return 2;
    } else if (value < (1 << 21)) {
        return 3;
    } else if (value < (1 << 28)) {
        return 4;
    } else {
        return 5;
    }
}
	
	
#ifdef __cplusplus
}
#endif
