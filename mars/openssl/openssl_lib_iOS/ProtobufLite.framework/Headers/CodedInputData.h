//
//  CodedInputData.h
//  MicroMessenger
//
//  Created by Guo Ling on 4/26/13.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

#ifdef __cplusplus
#import <MacTypes.h>
#endif

#ifdef __cplusplus

#ifndef NDEBUG
#import <vector>
#endif

@class WXPBGeneratedMessage;

class CodedInputData {
	uint8_t* bufferPointer;
	int32_t bufferSize;
	int32_t bufferSizeAfterLimit;
	int32_t bufferPos;
	int32_t lastTag;
	
	/** The absolute position of the end of the current message. */
	int32_t currentLimit;
	
	/** See setRecursionLimit() */
	int32_t recursionDepth;
	int32_t recursionLimit;
	
	/** See setSizeLimit() */
	int32_t sizeLimit;

public:
	CodedInputData(NSData* oData);
	~CodedInputData();
	
	bool isAtEnd() { return bufferPos == bufferSize; };
	void checkLastTagWas(int32_t value);
	
	int32_t readTag();
	BOOL readBool();
	Float64 readDouble();
	Float32 readFloat();
	int64_t readUInt64();
	int32_t readUInt32();
	int64_t readInt64();
	int32_t readInt32();
	int64_t readFixed64();
	int32_t readFixed32();
	int32_t readEnum();
	int32_t readSFixed32();
	int64_t readSFixed64();
	int32_t readSInt32();
	int64_t readSInt64();
	
	NSString* readString();
	NSData* readData();
	
	/**
	 * Read one byte from the input.
	 *
	 * @throws InvalidProtocolBuffer The end of the stream or the current
	 *                                        limit was reached.
	 */
	int8_t readRawByte();
	
	/**
	 * Read a raw Varint from the stream.  If larger than 32 bits, discard the
	 * upper bits.
	 */
	int32_t readRawVarint32();
	int64_t readRawVarint64();
	int32_t readRawLittleEndian32();
	int64_t readRawLittleEndian64();

	BOOL skipField(int32_t tag);
	
	int32_t decodeZigZag32(int32_t n);
	int64_t decodeZigZag64(int64_t n);
	
	int32_t setSizeLimit(int32_t limit);
	int32_t pushLimit(int32_t byteLimit);
	void recomputeBufferSizeAfterLimit();
	void popLimit(int32_t oldLimit);
	int32_t bytesUntilLimit();
	
	int8_t readRawByte(int8_t* bufferPointer, int32_t* bufferPos, int32_t bufferSize);
	void skipRawData(int32_t size);

	void readMessage(WXPBGeneratedMessage* builder);

#ifndef NDEBUG
	std::vector<int32_t>* m_fieldStack;
	void pushField(int32_t fieldNumber);
	void popField();
#endif
};

#endif
