#import "FantasyGLView.h"

#import <QuartzCore/CAEAGLLayer.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#include <vector>
#include <cstdio>
#include "Engine.h"
#include "Common/Logger.h"

using namespace Fantasy;

@implementation FantasyGLView {
    BOOL _ready;
}

+ (Class)layerClass { return [CAEAGLLayer class]; }

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        CAEAGLLayer *layer = (CAEAGLLayer *)self.layer;
        layer.opaque = YES;
        layer.drawableProperties = @{
            kEAGLDrawablePropertyRetainedBacking : @NO,
            kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8,
        };
        self.contentScaleFactor = UIScreen.mainScreen.scale;
        [self bootEngineOnce];
    }
    return self;
}

- (void)bootEngineOnce {
    static bool inited = false;
    if (inited) return;
    inited = true;

    new Engine();   
    Common::Logger::Instance()->setSink(
        [](Common::Logger::Level, const std::string& tag, const std::string& msg) {
            fprintf(stderr, "[Fantasy][%s] %s\n", tag.c_str(), msg.c_str());   // simctl 可抓
        });

    if (!Engine::Instance()->initialize()) {
        fprintf(stderr, "[Fantasy] Engine initialize FAILED\n");
    }
}

- (void)layoutSubviews {
    [super layoutSubviews];
    if (_ready) return;

    CGFloat s = self.contentScaleFactor;
    int pw = (int)(self.bounds.size.width  * s);
    int ph = (int)(self.bounds.size.height * s);
    if (pw <= 0 || ph <= 0) return;   
    _ready = YES;

    [self setSyntheticImage];                                   
    Engine::Instance()->onSurfaceCreated((__bridge void *)self.layer); 
    Engine::Instance()->onSurfaceChanged(pw, ph);                
    Engine::Instance()->renderOneFrame(0.f);                     
}

- (void)setSyntheticImage {
    const int W = 256, H = 256;
    std::vector<uint8_t> px((size_t)W * H * 4);
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            uint8_t *p = px.data() + ((size_t)y * W + x) * 4;
            bool right = (x >= W / 2), bottom = (y >= H / 2);
            if      (!right && !bottom) { p[0] = 255; p[1] = 0;   p[2] = 0;   }  // 红
            else if ( right && !bottom) { p[0] = 0;   p[1] = 255; p[2] = 0;   }  // 绿
            else if (!right &&  bottom) { p[0] = 0;   p[1] = 0;   p[2] = 255; }  // 蓝
            else                        { p[0] = 255; p[1] = 255; p[2] = 255; }  // 白
            p[3] = 255;
        }
    }
    Engine::Instance()->setImage(px.data(), W, H);
}

@end
