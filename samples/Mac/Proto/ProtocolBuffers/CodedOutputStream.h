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

/**
 * Encodes and writes protocol message fields.
 *
 * <p>This class contains two kinds of methods:  methods that write specific
 * protocol message constructs and field types (e.g. {@link #writeTag} and
 * {@link #writeInt32}) and methods that write low-level values (e.g.
 * {@link #writeRawVarint32} and {@link #writeRawBytes}).  If you are
 * writing encoded protocol messages, you should use the former methods, but if
 * you are writing some other format of your own design, use the latter.
 *
 * <p>This class is totally unsynchronized.
 *
 * @author Cyrus Najmabadi
 */

#import <Foundation/Foundation.h>

@class PBUnknownFieldSet;
@class RingBuffer;
@protocol PBMessage;

@interface PBCodedOutputStream : NSObject {
    NSOutputStream *output;
    RingBuffer *buffer;
}

+ (PBCodedOutputStream*) streamWithData:(NSMutableData*) data;
+ (PBCodedOutputStream*) streamWithOutputStream:(NSOutputStream*) output;
+ (PBCodedOutputStream*) streamWithOutputStream:(NSOutputStream*) output bufferSize:(SInt32) bufferSize;

/**
 * Flushes the stream and forces any buffered bytes to be written.  This
 * does not flush the underlying NSOutputStream. Returns free space in buffer.
 */
- (void) flush;

/** Write a single byte. */
- (void) writeRawByte:(uint8_t) value;

/** Encode and write a tag. */
- (void) writeTag:(SInt32) fieldNumber format:(SInt32) format;

/** Write a little-endian 32-bit integer. */
- (void) writeRawLittleEndian32:(SInt32) value;
/** Write a little-endian 64-bit integer. */
- (void) writeRawLittleEndian64:(SInt64) value;

/**
 * Encode and write a varint.  {@code value} is treated as
 * unsigned, so it won't be sign-extended if negative.
 */
- (void) writeRawVarint32:(SInt32) value;
/** Encode and write a varint. */
- (void) writeRawVarint64:(SInt64) value;

//- (void) writeRawLittleEndian32:(SInt32) value;
//- (void) writeRawLittleEndian64:(SInt64) value;

/** Write an array of bytes. */
- (void) writeRawData:(const NSData*) data;
- (void) writeRawData:(const NSData*) data offset:(SInt32) offset length:(SInt32) length;

- (void) writeData:(SInt32) fieldNumber value:(const NSData*) value;

- (void) writeDouble:(SInt32) fieldNumber value:(Float64) value;
- (void) writeFloat:(SInt32) fieldNumber value:(Float32) value;
- (void) writeUInt64:(SInt32) fieldNumber value:(SInt64) value;
- (void) writeInt64:(SInt32) fieldNumber value:(SInt64) value;
- (void) writeInt32:(SInt32) fieldNumber value:(SInt32) value;
- (void) writeFixed64:(SInt32) fieldNumber value:(SInt64) value;
- (void) writeFixed32:(SInt32) fieldNumber value:(SInt32) value;
- (void) writeBool:(SInt32) fieldNumber value:(BOOL) value;
- (void) writeString:(SInt32) fieldNumber value:(const NSString*) value;
- (void) writeGroup:(SInt32) fieldNumber value:(const id<PBMessage>) value;
- (void) writeUnknownGroup:(SInt32) fieldNumber value:(const PBUnknownFieldSet*) value;
- (void) writeMessage:(SInt32) fieldNumber value:(const id<PBMessage>) value;
- (void) writeUInt32:(SInt32) fieldNumber value:(SInt32) value;
- (void) writeSFixed32:(SInt32) fieldNumber value:(SInt32) value;
- (void) writeSFixed64:(SInt32) fieldNumber value:(SInt64) value;
- (void) writeSInt32:(SInt32) fieldNumber value:(SInt32) value;
- (void) writeSInt64:(SInt32) fieldNumber value:(SInt64) value;

- (void) writeDoubleNoTag:(Float64) value;
- (void) writeFloatNoTag:(Float32) value;
- (void) writeUInt64NoTag:(SInt64) value;
- (void) writeInt64NoTag:(SInt64) value;
- (void) writeInt32NoTag:(SInt32) value;
- (void) writeFixed64NoTag:(SInt64) value;
- (void) writeFixed32NoTag:(SInt32) value;
- (void) writeBoolNoTag:(BOOL) value;
- (void) writeStringNoTag:(const NSString*) value;
- (void) writeGroupNoTag:(SInt32) fieldNumber value:(const id<PBMessage>) value;
- (void) writeUnknownGroupNoTag:(SInt32) fieldNumber value:(const PBUnknownFieldSet*) value;
- (void) writeMessageNoTag:(const id<PBMessage>) value;
- (void) writeDataNoTag:(const NSData*) value;
- (void) writeUInt32NoTag:(SInt32) value;
- (void) writeEnumNoTag:(SInt32) value;
- (void) writeSFixed32NoTag:(SInt32) value;
- (void) writeSFixed64NoTag:(SInt64) value;
- (void) writeSInt32NoTag:(SInt32) value;
- (void) writeSInt64NoTag:(SInt64) value;


/**
 * Write a MessageSet extension field to the stream.  For historical reasons,
 * the wire format differs from normal fields.
 */
- (void) writeMessageSetExtension:(SInt32) fieldNumber value:(const id<PBMessage>) value;

/**
 * Write an unparsed MessageSet extension field to the stream.  For
 * historical reasons, the wire format differs from normal fields.
 */
- (void) writeRawMessageSetExtension:(SInt32) fieldNumber value:(const NSData*) value;

/**
 * Write an enum field, including tag, to the stream.  Caller is responsible
 * for converting the enum value to its numeric value.
 */
- (void) writeEnum:(SInt32) fieldNumber value:(SInt32) value;

@end
