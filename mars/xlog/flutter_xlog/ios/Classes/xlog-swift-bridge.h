#import <Foundation/Foundation.h>

@interface XLogBridge: NSObject

- (void)open: (NSUInteger)level cacheDir:(NSString*)cacheDir logDir:(NSString*)logDir prefix:(NSString*)prefix cacheDays:(NSUInteger)cacheDays consoleLogOpen:(BOOL)consoleLogOpen;

- (void)close;
@end
