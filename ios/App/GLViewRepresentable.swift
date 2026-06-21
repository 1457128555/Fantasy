import SwiftUI
import UIKit

struct GLViewRepresentable: UIViewRepresentable {
    let image: PickedImage

    func makeUIView(context: Context) -> FantasyGLView {
        let v = FantasyGLView(frame: .zero)
        // 创建即注入选好的图,确保首次 layoutSubviews 渲染的是真图而非合成图
        v.setPickedImageData(image.data, width: Int32(image.width), height: Int32(image.height))
        return v
    }

    func updateUIView(_ uiView: FantasyGLView, context: Context) {}
}
