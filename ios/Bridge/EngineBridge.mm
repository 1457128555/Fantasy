#import "EngineBridge.h"

#include "Engine.h"

using namespace Fantasy;

@implementation EngineBridge

+ (void)setImageRGBA:(NSData *)rgba width:(int)width height:(int)height {
    if (width <= 0 || height <= 0) return;
    if (rgba.length < (NSUInteger)width * (NSUInteger)height * 4) return;   // 防短读

    Engine::Instance()->setImage((void *)rgba.bytes, width, height);
    Engine::Instance()->renderOneFrame(0.f);
}

@end
