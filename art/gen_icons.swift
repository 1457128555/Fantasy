// 图标生成器:一张方形母版 → Android/iOS 全部尺寸的不透明 PNG。
// 用法:  xcrun swift art/gen_icons.swift            (从仓库根目录跑)
//        xcrun swift art/gen_icons.swift --probe    (只切 1 张到 /tmp 验证无 alpha)
//
// 为什么要 Swift 而不是 sips:iOS 图标禁止 alpha 通道,sips 去不掉,
// 这里画进 CGImageAlphaInfo.noneSkipLast 的 opaque 上下文,导出即无 alpha。
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers

func loadCGImage(_ path: String) -> CGImage? {
    guard let data = FileManager.default.contents(atPath: path),
          let src = CGImageSourceCreateWithData(data as CFData, nil),
          let img = CGImageSourceCreateImageAtIndex(src, 0, nil) else { return nil }
    return img
}

// 画到不带 alpha 的 opaque 上下文(先铺白底兜底,防止母版边缘有半透明像素混进黑底)
func writeOpaquePNG(_ image: CGImage, size: Int, to path: String) -> Bool {
    let cs = CGColorSpaceCreateDeviceRGB()
    let bitmapInfo = CGImageAlphaInfo.noneSkipLast.rawValue
    guard let ctx = CGContext(data: nil, width: size, height: size,
                              bitsPerComponent: 8, bytesPerRow: 0,
                              space: cs, bitmapInfo: bitmapInfo) else { return false }
    ctx.setFillColor(CGColor(red: 1, green: 1, blue: 1, alpha: 1))
    ctx.fill(CGRect(x: 0, y: 0, width: size, height: size))
    ctx.interpolationQuality = .high
    ctx.draw(image, in: CGRect(x: 0, y: 0, width: size, height: size))
    guard let out = ctx.makeImage() else { return false }
    let dir = (path as NSString).deletingLastPathComponent
    try? FileManager.default.createDirectory(atPath: dir, withIntermediateDirectories: true)
    guard let dest = CGImageDestinationCreateWithURL(URL(fileURLWithPath: path) as CFURL,
                                                     UTType.png.identifier as CFString, 1, nil) else { return false }
    CGImageDestinationAddImage(dest, out, nil)
    return CGImageDestinationFinalize(dest)
}

let master = "art/icon-master.png"
guard let img = loadCGImage(master) else { fatalError("无法读取母版 \(master)") }

struct Job { let size: Int; let path: String }

if CommandLine.arguments.contains("--probe") {
    let p = "/tmp/icon_probe.png"
    print(writeOpaquePNG(img, size: 128, to: p) ? "✓ probe -> \(p)" : "✗ probe 失败")
    exit(0)
}

let resA = "android/app/src/main/res"
var jobs: [Job] = []
// Android 旧机(API 24-25)用的密度位图:mdpi..xxxhdpi
let legacy: [(String, Int)] = [("mdpi", 48), ("hdpi", 72), ("xhdpi", 96), ("xxhdpi", 144), ("xxxhdpi", 192)]
for (d, s) in legacy {
    jobs.append(Job(size: s, path: "\(resA)/mipmap-\(d)/ic_launcher.png"))
    jobs.append(Job(size: s, path: "\(resA)/mipmap-\(d)/ic_launcher_round.png"))
}
// Android adaptive(API 26+)两层用的整图:108dp@4x = 432px,放 nodpi 一份够
jobs.append(Job(size: 432, path: "\(resA)/drawable-nodpi/ic_launcher_img.png"))
// iOS 单尺寸 AppIcon:1024(720 升采样;设备实际显示≤180,此图仅 App Store 展示)
jobs.append(Job(size: 1024, path: "ios/App/Assets.xcassets/AppIcon.appiconset/icon-1024.png"))

for j in jobs {
    let ok = writeOpaquePNG(img, size: j.size, to: j.path)
    print(ok ? "✓ \(j.size)\t\(j.path)" : "✗ 失败\t\(j.path)")
}
print("完成,共 \(jobs.count) 张。")
