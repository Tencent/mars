#import <Foundation/Foundation.h>

@interface RingBuffer : NSObject {
	NSMutableData *buffer;
	SInt32 position;
	SInt32 tail;
}
@property (nonatomic, readonly) UInt32 freeSpace;

- (instancetype)initWithData:(NSMutableData*)data;

// Returns false if there is not enough free space in buffer
- (BOOL)appendByte:(uint8_t)byte;

// Returns number of bytes written
- (SInt32)appendData:(const NSData*)value offset:(SInt32)offset length:(SInt32)length;

// Returns number of bytes written
- (SInt32)flushToOutputStream:(NSOutputStream*)stream;

@end
