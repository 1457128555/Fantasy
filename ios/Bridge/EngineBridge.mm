#import "EngineBridge.h"

#include "Engine.h"

using namespace Fantasy;

@implementation EngineBridge

// 在主线程调：setImage 只是 memcpy 进 Engine（不碰 GL），renderOneFrame 把活 post 给渲染线程。
// 引擎在 FantasyGLView 创建时已 initialize，所以这里 Engine::Instance() 一定有效。
+ (void)setImageRGBA:(NSData *)rgba width:(int)width height:(int)height {
    if (width <= 0 || height <= 0) return;
    if (rgba.length < (NSUInteger)width * (NSUInteger)height * 4) return;   // 防短读

    Engine::Instance()->setImage((void *)rgba.bytes, width, height);
    Engine::Instance()->renderOneFrame(0.f);
}

@end
