#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// 给 Swift 调的薄桥：把解码好的 RGBA8 像素喂给引擎并触发一帧渲染。
// ≈ Android 的 EngineBridge.setImage(bitmap) / jni_bridge 的 nativeSetImage
@interface EngineBridge : NSObject
+ (void)setImageRGBA:(NSData *)rgba width:(int)width height:(int)height;
@end

NS_ASSUME_NONNULL_END
