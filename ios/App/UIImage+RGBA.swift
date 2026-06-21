import UIKit

extension UIImage {

    // 解码成紧密排列的 RGBA8 像素（row 0 = 图片顶部），并先把 EXIF 朝向摊平。
    // ≈ Android 的 ImageDecoder 解码 + lockPixels 逐行拷贝
    //
    // 注意 row 0 = 顶部：CGContextDrawImage 出来的就是「顶行在前」，而 GL 纹理约定
    // 「行 0 在底」——这正是 Renderer 的 fragment shader 里 (1.0 - vUV.y) 翻 v 的原因，
    // 两端（Android/iOS）行序一致，所以同一套 shader 通吃。
    func rgbaPixels() -> (data: Data, width: Int, height: Int)? {
        guard let cg = normalizedUp().cgImage else { return nil }
        let w = cg.width, h = cg.height
        let bytesPerRow = w * 4
        var buf = [UInt8](repeating: 0, count: bytesPerRow * h)
        let space = CGColorSpaceCreateDeviceRGB()
        let info = CGImageAlphaInfo.premultipliedLast.rawValue   // RGBA 顺序
        guard let ctx = CGContext(data: &buf,
                                  width: w, height: h,
                                  bitsPerComponent: 8,
                                  bytesPerRow: bytesPerRow,
                                  space: space,
                                  bitmapInfo: info) else { return nil }
        ctx.draw(cg, in: CGRect(x: 0, y: 0, width: w, height: h))
        return (Data(buf), w, h)
    }

    // 相册里的照片常带 EXIF 朝向（cgImage 是原始未旋转像素）。先重绘成 .up，
    // 这样取到的像素方位和你肉眼在相册里看到的一致。
    private func normalizedUp() -> UIImage {
        if imageOrientation == .up { return self }
        let format = UIGraphicsImageRendererFormat.default()
        format.scale = 1   // 按像素出图，别再乘 screen scale
        let renderer = UIGraphicsImageRenderer(size: size, format: format)
        return renderer.image { _ in
            draw(in: CGRect(origin: .zero, size: size))
        }
    }
}
