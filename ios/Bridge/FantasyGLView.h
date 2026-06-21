#import <UIKit/UIKit.h>

@interface FantasyGLView : UIView
// 在创建时注入选好的图(RGBA8,row 0 = 顶部),layoutSubviews 会用它而非合成图。
// 必须在加入视图层级(首次 layout)前调用,避免与 layoutSubviews 的时序竞争。
- (void)setPickedImageData:(NSData *)data width:(int)width height:(int)height;
@end
