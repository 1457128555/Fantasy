#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface EngineBridge : NSObject
+ (void)setImageRGBA:(NSData *)rgba width:(int)width height:(int)height;
@end

NS_ASSUME_NONNULL_END
