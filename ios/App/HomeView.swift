import SwiftUI
import PhotosUI

// 主页:参考图背景 + 顶栏(名字左 / 齿轮右) + 下方「选图」。选完跳编辑页。≈ Android HomeScreen.kt
struct HomeView: View {
    @State private var pickerItem: PhotosPickerItem?
    @State private var path = NavigationPath()

    // 品牌色:深粉棕,压在粉色壁纸上够清楚
    private let brand = Color(red: 0.50, green: 0.24, blue: 0.34)

    var body: some View {
        NavigationStack(path: $path) {
            ZStack {
                Image("ref-bg")                       // Assets.xcassets 里的渐变壁纸
                    .resizable()
                    .scaledToFill()                   // 填充+裁切,不变形 ≈ ContentScale.Crop
                    .ignoresSafeArea()

                VStack {
                    // 顶栏:名字靠左 + 齿轮靠右
                    HStack {
                        Text("Fantasy")
                            .font(.system(size: 32, weight: .bold, design: .rounded))
                            .tracking(0.5)            // 字距,更像 logo
                            .foregroundStyle(brand)
                            .shadow(color: .black.opacity(0.12), radius: 2, x: 0, y: 1)

                        Spacer()

                        Button {
                            // TODO: 设置页(暂占位,不跳转)
                        } label: {
                            Image(systemName: "gearshape.fill")
                                .font(.system(size: 22, weight: .semibold))
                                .foregroundStyle(brand)
                                .shadow(color: .black.opacity(0.12), radius: 2, x: 0, y: 1)
                        }
                    }
                    .padding(.horizontal, 36)
                    .padding(.top, 12)

                    Spacer()                          // 把按钮顶到下方

                    // 选图按钮:压到下方拇指区,离底边留空隙
                    PhotosPicker(selection: $pickerItem, matching: .images) {
                        HStack(spacing: 8) {
                            Image(systemName: "photo.fill")
                            Text("选图")
                        }
                        .font(.system(size: 19, weight: .semibold))
                        .foregroundStyle(brand)
                        .padding(.horizontal, 52)
                        .padding(.vertical, 18)
                        .background(.white.opacity(0.92), in: Capsule())
                        .shadow(color: brand.opacity(0.25), radius: 12, x: 0, y: 5)
                    }
                    .padding(.bottom, 80)
                }
            }
            .navigationDestination(for: PickedImage.self) { img in
                EditorView(image: img)
            }
        }
        .onChange(of: pickerItem) { newItem in
            guard let newItem else { return }
            Task {
                // 解码摊成 RGBA(CPU 活,放后台);拿到再回主线程跳页
                guard let data = try? await newItem.loadTransferable(type: Data.self),
                      let ui = UIImage(data: data),
                      let px = ui.rgbaPixels() else { return }
                await MainActor.run {
                    pickerItem = nil                  // 复位,便于再次选择
                    path.append(PickedImage(data: px.data, width: px.width, height: px.height))
                }
            }
        }
    }
}
