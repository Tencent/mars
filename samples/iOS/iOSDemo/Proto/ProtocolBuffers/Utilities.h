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

#import "Message.h"

SInt64 convertFloat64ToInt64(Float64 f);
SInt32 convertFloat32ToInt32(Float32 f);
Float64 convertInt64ToFloat64(SInt64 f);
Float32 convertInt32ToFloat32(SInt32 f);

UInt64 convertInt64ToUInt64(SInt64 i);
SInt64  convertUInt64ToInt64(UInt64 u);
UInt32 convertInt32ToUInt32(SInt32 i);
SInt32  convertUInt32ToInt32(UInt32 u);

SInt32 logicalRightShift32(SInt32 value, SInt32 spaces);
SInt64 logicalRightShift64(SInt64 value, SInt32 spaces);


/**
 * Decode a ZigZag-encoded 32-bit value.  ZigZag encodes signed integers
 * into values that can be efficiently encoded with varint.  (Otherwise,
 * negative values must be sign-extended to 64 bits to be varint encoded,
 * thus always taking 10 bytes on the wire.)
 *
 * @param n An unsigned 32-bit integer, stored in a signed int.
 * @return A signed 32-bit integer.
 */
SInt32 decodeZigZag32(SInt32 n);

/**
 * Decode a ZigZag-encoded 64-bit value.  ZigZag encodes signed integers
 * into values that can be efficiently encoded with varint.  (Otherwise,
 * negative values must be sign-extended to 64 bits to be varint encoded,
 * thus always taking 10 bytes on the wire.)
 *
 * @param n An unsigned 64-bit integer, stored in a signed int.
 * @return A signed 64-bit integer.
 */
SInt64 decodeZigZag64(SInt64 n);


/**
 * Encode a ZigZag-encoded 32-bit value.  ZigZag encodes signed integers
 * into values that can be efficiently encoded with varint.  (Otherwise,
 * negative values must be sign-extended to 64 bits to be varint encoded,
 * thus always taking 10 bytes on the wire.)
 *
 * @param n A signed 32-bit integer.
 * @return An unsigned 32-bit integer, stored in a signed int.
 */
SInt32 encodeZigZag32(SInt32 n);

/**
 * Encode a ZigZag-encoded 64-bit value.  ZigZag encodes signed integers
 * into values that can be efficiently encoded with varint.  (Otherwise,
 * negative values must be sign-extended to 64 bits to be varint encoded,
 * thus always taking 10 bytes on the wire.)
 *
 * @param n A signed 64-bit integer.
 * @return An unsigned 64-bit integer, stored in a signed int.
 */
SInt64 encodeZigZag64(SInt64 n);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code double} field, including tag.
 */
SInt32 computeDoubleSize(SInt32 fieldNumber, Float64 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code float} field, including tag.
 */
SInt32 computeFloatSize(SInt32 fieldNumber, Float32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code uint64} field, including tag.
 */
SInt32 computeUInt64Size(SInt32 fieldNumber, SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code int64} field, including tag.
 */
SInt32 computeInt64Size(SInt32 fieldNumber, SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code int32} field, including tag.
 */
SInt32 computeInt32Size(SInt32 fieldNumber, SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code fixed64} field, including tag.
 */
SInt32 computeFixed64Size(SInt32 fieldNumber, SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code fixed32} field, including tag.
 */
SInt32 computeFixed32Size(SInt32 fieldNumber, SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code bool} field, including tag.
 */
SInt32 computeBoolSize(SInt32 fieldNumber, BOOL value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code string} field, including tag.
 */
SInt32 computeStringSize(SInt32 fieldNumber, const NSString* value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code group} field, including tag.
 */
SInt32 computeGroupSize(SInt32 fieldNumber, const id<PBMessage> value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code group} field represented by an {@code PBUnknownFieldSet}, including
 * tag.
 */
SInt32 computeUnknownGroupSize(SInt32 fieldNumber, const PBUnknownFieldSet* value);

/**
 * Compute the number of bytes that would be needed to encode an
 * embedded message field, including tag.
 */
SInt32 computeMessageSize(SInt32 fieldNumber, const id<PBMessage> value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code bytes} field, including tag.
 */
SInt32 computeDataSize(SInt32 fieldNumber, const NSData* value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code uint32} field, including tag.
 */
SInt32 computeUInt32Size(SInt32 fieldNumber, SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sfixed32} field, including tag.
 */
SInt32 computeSFixed32Size(SInt32 fieldNumber, SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sfixed64} field, including tag.
 */
SInt32 computeSFixed64Size(SInt32 fieldNumber, SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sint32} field, including tag.
 */
SInt32 computeSInt32Size(SInt32 fieldNumber, SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sint64} field, including tag.
 */
SInt32 computeSInt64Size(SInt32 fieldNumber, SInt64 value);

/** Compute the number of bytes that would be needed to encode a tag. */
SInt32 computeTagSize(SInt32 fieldNumber);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code double} field, including tag.
 */
SInt32 computeDoubleSizeNoTag(Float64 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code float} field, including tag.
 */
SInt32 computeFloatSizeNoTag(Float32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code uint64} field, including tag.
 */
SInt32 computeUInt64SizeNoTag(SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code int64} field, including tag.
 */
SInt32 computeInt64SizeNoTag(SInt64 value);
/**
 * Compute the number of bytes that would be needed to encode an
 * {@code int32} field, including tag.
 */
SInt32 computeInt32SizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code fixed64} field, including tag.
 */
SInt32 computeFixed64SizeNoTag(SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code fixed32} field, including tag.
 */
SInt32 computeFixed32SizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code bool} field, including tag.
 */
SInt32 computeBoolSizeNoTag(BOOL value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code string} field, including tag.
 */
SInt32 computeStringSizeNoTag(const NSString* value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code group} field, including tag.
 */
SInt32 computeGroupSizeNoTag(const id<PBMessage> value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code group} field represented by an {@code PBUnknownFieldSet}, including
 * tag.
 */
SInt32 computeUnknownGroupSizeNoTag(const PBUnknownFieldSet* value);

/**
 * Compute the number of bytes that would be needed to encode an
 * embedded message field, including tag.
 */
SInt32 computeMessageSizeNoTag(const id<PBMessage> value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code bytes} field, including tag.
 */
SInt32 computeDataSizeNoTag(const NSData* value);

/**
 * Compute the number of bytes that would be needed to encode a
 * {@code uint32} field, including tag.
 */
SInt32 computeUInt32SizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * enum field, including tag.  Caller is responsible for converting the
 * enum value to its numeric value.
 */
SInt32 computeEnumSizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sfixed32} field, including tag.
 */
SInt32 computeSFixed32SizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sfixed64} field, including tag.
 */
SInt32 computeSFixed64SizeNoTag(SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sint32} field, including tag.
 */
SInt32 computeSInt32SizeNoTag(SInt32 value);

/**
 * Compute the number of bytes that would be needed to encode an
 * {@code sint64} field, including tag.
 */
SInt32 computeSInt64SizeNoTag(SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode a varint.
 * {@code value} is treated as unsigned, so it won't be sign-extended if
 * negative.
 */
SInt32 computeRawVarint32Size(SInt32 value);

/** Compute the number of bytes that would be needed to encode a varint. */
SInt32 computeRawVarint64Size(SInt64 value);

/**
 * Compute the number of bytes that would be needed to encode a
 * MessageSet extension to the stream.  For historical reasons,
 * the wire format differs from normal fields.
 */
SInt32 computeMessageSetExtensionSize(SInt32 fieldNumber, const id<PBMessage> value);

/**
 * Compute the number of bytes that would be needed to encode an
 * unparsed MessageSet extension field to the stream.  For
 * historical reasons, the wire format differs from normal fields.
 */
SInt32 computeRawMessageSetExtensionSize(SInt32 fieldNumber, const NSData* value);

/**
 * Compute the number of bytes that would be needed to encode an
 * enum field, including tag.  Caller is responsible for converting the
 * enum value to its numeric value.
 */
SInt32 computeEnumSize(SInt32 fieldNumber, SInt32 value);
