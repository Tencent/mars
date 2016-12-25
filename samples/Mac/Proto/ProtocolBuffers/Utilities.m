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

#import "Utilities.h"

#import "UnknownFieldSet.h"
#import "WireFormat.h"

const SInt32 LITTLE_ENDIAN_32_SIZE = 4;
const SInt32 LITTLE_ENDIAN_64_SIZE = 8;


SInt64 convertFloat64ToInt64(Float64 v) {
  union { Float64 f; SInt64 i; } u;
  u.f = v;
  return u.i;
}


SInt32 convertFloat32ToInt32(Float32 v) {
  union { Float32 f; SInt32 i; } u;
  u.f = v;
  return u.i;
}


Float64 convertInt64ToFloat64(SInt64 v) {
  union { Float64 f; SInt64 i; } u;
  u.i = v;
  return u.f;
}


Float32 convertInt32ToFloat32(SInt32 v) {
  union { Float32 f; SInt32 i; } u;
  u.i = v;
  return u.f;
}


UInt64 convertInt64ToUInt64(SInt64 v) {
  union { SInt64 i; UInt64 u; } u;
  u.i = v;
  return u.u;
}


SInt64 convertUInt64ToInt64(UInt64 v) {
  union { SInt64 i; UInt64 u; } u;
  u.u = v;
  return u.i;
}

UInt32 convertInt32ToUInt32(SInt32 v) {
  union { SInt32 i; UInt32 u; } u;
  u.i = v;
  return u.u;
}


SInt32 convertUInt32ToInt32(UInt32 v) {
  union { SInt32 i; UInt32 u; } u;
  u.u = v;
  return u.i;
}


SInt32 logicalRightShift32(SInt32 value, SInt32 spaces) {
  return convertUInt32ToInt32((convertInt32ToUInt32(value) >> spaces));
}


SInt64 logicalRightShift64(SInt64 value, SInt32 spaces) {
  return convertUInt64ToInt64((convertInt64ToUInt64(value) >> spaces));
}


SInt32 decodeZigZag32(SInt32 n) {
	return logicalRightShift32(n, 1) ^ -(n & 1);
}


SInt64 decodeZigZag64(SInt64 n) {
	return logicalRightShift64(n, 1) ^ -(n & 1);
}


SInt32 encodeZigZag32(SInt32 n) {
	// Note:  the right-shift must be arithmetic
	return (n << 1) ^ (n >> 31);
}


SInt64 encodeZigZag64(SInt64 n) {
	// Note:  the right-shift must be arithmetic
	return (n << 1) ^ (n >> 63);
}


SInt32 computeDoubleSizeNoTag(Float64 value) {
	return LITTLE_ENDIAN_64_SIZE;
}


SInt32 computeFloatSizeNoTag(Float32 value) {
	return LITTLE_ENDIAN_32_SIZE;
}


SInt32 computeUInt64SizeNoTag(SInt64 value) {
	return computeRawVarint64Size(value);
}


SInt32 computeInt64SizeNoTag(SInt64 value) {
	return computeRawVarint64Size(value);
}


SInt32 computeInt32SizeNoTag(SInt32 value) {
	if (value >= 0) {
		return computeRawVarint32Size(value);
	} else {
		// Must sign-extend.
		return 10;
	}
}


SInt32 computeFixed64SizeNoTag(SInt64 value) {
	return LITTLE_ENDIAN_64_SIZE;
}


SInt32 computeFixed32SizeNoTag(SInt32 value) {
	return LITTLE_ENDIAN_32_SIZE;
}


SInt32 computeBoolSizeNoTag(BOOL value) {
	return 1;
}


SInt32 computeStringSizeNoTag(const NSString* value) {
	const UInt32 length = (UInt32)[value lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
	return computeRawVarint32Size(length) + length;
}


SInt32 computeGroupSizeNoTag(const id<PBMessage> value) {
	return [value serializedSize];
}


SInt32 computeUnknownGroupSizeNoTag(const PBUnknownFieldSet* value) {
	return value.serializedSize;
}


SInt32 computeMessageSizeNoTag(const id<PBMessage> value) {
	SInt32 size = [value serializedSize];
	return computeRawVarint32Size(size) + size;
}


SInt32 computeDataSizeNoTag(const NSData* value) {
	return computeRawVarint32Size((UInt32)value.length) + (UInt32)value.length;
}


SInt32 computeUInt32SizeNoTag(SInt32 value) {
	return computeRawVarint32Size(value);
}


SInt32 computeEnumSizeNoTag(SInt32 value) {
	return computeRawVarint32Size(value);
}


SInt32 computeSFixed32SizeNoTag(SInt32 value) {
	return LITTLE_ENDIAN_32_SIZE;
}


SInt32 computeSFixed64SizeNoTag(SInt64 value) {
	return LITTLE_ENDIAN_64_SIZE;
}


SInt32 computeSInt32SizeNoTag(SInt32 value) {
	return computeRawVarint32Size(encodeZigZag32(value));
}


SInt32 computeSInt64SizeNoTag(SInt64 value) {
	return computeRawVarint64Size(encodeZigZag64(value));
}


SInt32 computeDoubleSize(SInt32 fieldNumber, Float64 value) {
	return computeTagSize(fieldNumber) + computeDoubleSizeNoTag(value);
}


SInt32 computeFloatSize(SInt32 fieldNumber, Float32 value) {
	return computeTagSize(fieldNumber) + computeFloatSizeNoTag(value);
}


SInt32 computeUInt64Size(SInt32 fieldNumber, SInt64 value) {
	return computeTagSize(fieldNumber) + computeUInt64SizeNoTag(value);
}


SInt32 computeInt64Size(SInt32 fieldNumber, SInt64 value) {
	return computeTagSize(fieldNumber) + computeInt64SizeNoTag(value);
}


SInt32 computeInt32Size(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeInt32SizeNoTag(value);
}


SInt32 computeFixed64Size(SInt32 fieldNumber, SInt64 value) {
	return computeTagSize(fieldNumber) + computeFixed64SizeNoTag(value);
}


SInt32 computeFixed32Size(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeFixed32SizeNoTag(value);
}


SInt32 computeBoolSize(SInt32 fieldNumber, BOOL value) {
	return computeTagSize(fieldNumber) + computeBoolSizeNoTag(value);
}


SInt32 computeStringSize(SInt32 fieldNumber, const NSString* value) {
	return computeTagSize(fieldNumber) + computeStringSizeNoTag(value);
}


SInt32 computeGroupSize(SInt32 fieldNumber, const id<PBMessage> value) {
	return computeTagSize(fieldNumber) * 2 + computeGroupSizeNoTag(value);
}


SInt32 computeUnknownGroupSize(SInt32 fieldNumber, const PBUnknownFieldSet* value) {
	return computeTagSize(fieldNumber) * 2 + computeUnknownGroupSizeNoTag(value);
}


SInt32 computeMessageSize(SInt32 fieldNumber, const id<PBMessage> value) {
	return computeTagSize(fieldNumber) + computeMessageSizeNoTag(value);
}


SInt32 computeDataSize(SInt32 fieldNumber, const NSData* value) {
	return computeTagSize(fieldNumber) + computeDataSizeNoTag(value);
}


SInt32 computeUInt32Size(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeUInt32SizeNoTag(value);
}


SInt32 computeEnumSize(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeEnumSizeNoTag(value);
}


SInt32 computeSFixed32Size(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeSFixed32SizeNoTag(value);
}


SInt32 computeSFixed64Size(SInt32 fieldNumber, SInt64 value) {
	return computeTagSize(fieldNumber) + computeSFixed64SizeNoTag(value);
}


SInt32 computeSInt32Size(SInt32 fieldNumber, SInt32 value) {
	return computeTagSize(fieldNumber) + computeSInt32SizeNoTag(value);
}


SInt32 computeTagSize(SInt32 fieldNumber) {
	return computeRawVarint32Size(PBWireFormatMakeTag(fieldNumber, 0));
}


SInt32 computeSInt64Size(SInt32 fieldNumber, SInt64 value) {
	return computeTagSize(fieldNumber) +
	computeRawVarint64Size(encodeZigZag64(value));
}


SInt32 computeRawVarint32Size(SInt32 value) {
	if ((value & (0xffffffff <<  7)) == 0) return 1;
	if ((value & (0xffffffff << 14)) == 0) return 2;
	if ((value & (0xffffffff << 21)) == 0) return 3;
	if ((value & (0xffffffff << 28)) == 0) return 4;
	return 5;
}


SInt32 computeRawVarint64Size(SInt64 value) {
	if ((value & (0xffffffffffffffffL <<  7)) == 0) return 1;
	if ((value & (0xffffffffffffffffL << 14)) == 0) return 2;
	if ((value & (0xffffffffffffffffL << 21)) == 0) return 3;
	if ((value & (0xffffffffffffffffL << 28)) == 0) return 4;
	if ((value & (0xffffffffffffffffL << 35)) == 0) return 5;
	if ((value & (0xffffffffffffffffL << 42)) == 0) return 6;
	if ((value & (0xffffffffffffffffL << 49)) == 0) return 7;
	if ((value & (0xffffffffffffffffL << 56)) == 0) return 8;
	if ((value & (0xffffffffffffffffL << 63)) == 0) return 9;
	return 10;
}


SInt32 computeMessageSetExtensionSize(SInt32 fieldNumber, const id<PBMessage> value) {
	return computeTagSize(PBWireFormatMessageSetItem) * 2 +
	computeUInt32Size(PBWireFormatMessageSetTypeId, fieldNumber) +
	computeMessageSize(PBWireFormatMessageSetMessage, value);
}


SInt32 computeRawMessageSetExtensionSize(SInt32 fieldNumber, const NSData* value) {
	return computeTagSize(PBWireFormatMessageSetItem) * 2 +
	computeUInt32Size(PBWireFormatMessageSetTypeId, fieldNumber) +
	computeDataSize(PBWireFormatMessageSetMessage, value);
}
