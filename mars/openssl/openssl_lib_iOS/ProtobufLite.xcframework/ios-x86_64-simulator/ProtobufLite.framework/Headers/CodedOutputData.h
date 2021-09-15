//
//  CodedOutputData.h
//  PBCoder
//
//  Created by Ling Guo on 4/17/14.
//  Copyright (c 2014 Tencent. All rights reserved.
//

#pragma once

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#endif

#ifdef __cplusplus
#import <MacTypes.h>
#endif

#ifdef __cplusplus

#pragma pack(push, 1)

// a small buffer to encode all primitive types (sint32/int64/double)
struct SmallBuffer {
    uint8_t length;
    uint8_t buffer[16 - sizeof(length)];

    explicit SmallBuffer(size_t size);
};

#pragma pack(pop)

// a buffer wrap (doesn't take ownership of the buffer)
struct NormalBufferWrap {
    size_t length;
    void *ptr;

    NormalBufferWrap() = default;
    explicit NormalBufferWrap(SmallBuffer &buffer);
    explicit NormalBufferWrap(__unsafe_unretained NSData *odata);
};

@class WXPBGeneratedMessage;

class CodedOutputData {
    uint8_t *const bufferPointer;
    size_t bufferLength;
    int32_t position;

    void checkNoSpaceLeft();

public:
    CodedOutputData(void *ptr, size_t len);
    CodedOutputData(__unsafe_unretained NSMutableData *odata);
    CodedOutputData(SmallBuffer &buffer);
    CodedOutputData(NormalBufferWrap &buffer);

    int32_t spaceLeft();
    void seek(size_t addedSize);

    uint8_t *curWritePointer();

    void writeRawByte(uint8_t value);

    void writeTag(int32_t fieldNumber, int32_t format);

    void writeRawLittleEndian32(int32_t value);
    void writeRawLittleEndian64(int64_t value);

    /**
	 * Encode and write a varint.  value is treated as
	 * unsigned, so it won't be sign-extended if negative.
	 */
    void writeRawVarint32(int32_t value);
    void writeRawVarint64(int64_t value);

    void writeRawData(__unsafe_unretained NSData *data);
    void writeRawData(__unsafe_unretained NSData *data, int32_t offset, int32_t length);
    void writeRawData(const NormalBufferWrap &value);

    void writeData(int32_t fieldNumber, __unsafe_unretained NSData *value);
    void writeData(int32_t fieldNumber, const NormalBufferWrap &value);

    void writeDouble(int32_t fieldNumber, Float64 value);
    void writeFloat(int32_t fieldNumber, Float32 value);
    void writeUInt64(int32_t fieldNumber, int64_t value);
    void writeInt64(int32_t fieldNumber, int64_t value);
    void writeInt32(int32_t fieldNumber, int32_t value);
    void writeFixed64(int32_t fieldNumber, int64_t value);
    void writeFixed32(int32_t fieldNumber, int32_t value);
    void writeBool(int32_t fieldNumber, BOOL value);
    void writeString(int32_t fieldNumber, NSString *value);
    void writeUInt32(int32_t fieldNumber, int32_t value);
    void writeSFixed32(int32_t fieldNumber, int32_t value);
    void writeSFixed64(int32_t fieldNumber, int64_t value);
    void writeSInt32(int32_t fieldNumber, int32_t value);
    void writeSInt64(int32_t fieldNumber, int64_t value);

    void writeDoubleNoTag(Float64 value);
    void writeFloatNoTag(Float32 value);
    void writeUInt64NoTag(int64_t value);
    void writeInt64NoTag(int64_t value);
    void writeInt32NoTag(int32_t value);
    void writeFixed64NoTag(int64_t value);
    void writeFixed32NoTag(int32_t value);
    void writeBoolNoTag(BOOL value);
    void writeStringNoTag(__unsafe_unretained NSString *value);
    void writeStringNoTag(__unsafe_unretained NSString *value, NSUInteger numberOfBytes);
    void writeDataNoTag(__unsafe_unretained NSData *value);
    void writeDataNoTag(const NormalBufferWrap &value);
    void writeUInt32NoTag(int32_t value);
    void writeEnumNoTag(int32_t value);
    void writeSFixed32NoTag(int32_t value);
    void writeSFixed64NoTag(int64_t value);
    void writeSInt32NoTag(int32_t value);
    void writeSInt64NoTag(int64_t value);

    /**
	 * Write an enum field, including tag, to the stream.  Caller is responsible
	 * for converting the enum value to its numeric value.
	 */
    void writeEnum(int32_t fieldNumber, int32_t value);

    void writeMessageNoTag(__unsafe_unretained WXPBGeneratedMessage *value);
    void writeMessage(int32_t fieldNumber, __unsafe_unretained WXPBGeneratedMessage *value);
};

#endif
