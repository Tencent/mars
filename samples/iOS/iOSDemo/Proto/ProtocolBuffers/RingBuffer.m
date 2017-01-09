#import "RingBuffer.h"

@implementation RingBuffer

- (instancetype)initWithData:(NSMutableData*)data {
  if ((self = [super init])) {
      buffer = data;
  }
	return self;
}


- (UInt32)freeSpace {
	return (UInt32)(position < tail ? tail - position : (buffer.length - position) + tail) - (tail ? 1 : 0);
}


- (BOOL)appendByte:(uint8_t)byte {
	if (self.freeSpace < 1) return NO;
	((uint8_t*)buffer.mutableBytes)[position++] = byte;
	return YES;
}


- (SInt32)appendData:(const NSData*)value offset:(SInt32)offset length:(SInt32)length {
	SInt32 totalWritten = 0;
	const uint8_t *input = value.bytes;
	uint8_t *data = buffer.mutableBytes;
	
	if (position >= tail) {
		totalWritten = MIN((UInt32)buffer.length - position, length);
		memcpy(data + position, input + offset, totalWritten);
		position += totalWritten;
		if (totalWritten == length) return length;
		length -= totalWritten;
		offset += totalWritten;
	}
	
	UInt32 freeSpace = self.freeSpace;
	if (!freeSpace) return totalWritten;
	
	if (position == buffer.length) {
		position = 0;
	}
	
	// position < tail
	SInt32 written = MIN(freeSpace, length);
	memcpy(data + position, input + offset, written);
	position += written;
	totalWritten += written;
	
	return totalWritten;
}


- (SInt32)flushToOutputStream:(NSOutputStream*)stream {
	SInt32 totalWritten = 0;
	const uint8_t *data = buffer.bytes;
	
	if (tail > position) {
		SInt32 written = (SInt32)[stream write:data + tail maxLength:buffer.length - tail];
        if (written <= 0) return totalWritten;
        totalWritten += written;
		tail += written;
		if (tail == buffer.length) {
			tail = 0;
		}
	}

	if (tail < position) {
		SInt32 written = (SInt32)[stream write:data + tail maxLength:position - tail];
		if (written <= 0) return totalWritten;
		totalWritten += written;
		tail += written;
	}

    if (tail == position) {
        tail = position = 0;
    }

    if (position == buffer.length && tail > 0) {
        position = 0;
    }

    if (tail == buffer.length) {
        tail = 0;
    }
	
	return totalWritten;
}

@end
