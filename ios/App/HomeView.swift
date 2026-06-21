import SwiftUI
import PhotosUI

// 主页:参考图背景 + 「选图」按钮。选完跳编辑页。≈ Android HomeScreen.kt
struct HomeView: View {
    @State private var pickerItem: PhotosPickerItem?
    @State private var path = NavigationPath()

    var body: some View {
        NavigationStack(path: $path) {
            ZStack {
                Image("ref-bg")                       // Assets.xcassets 里的渐变壁纸
                    .resizable()
                    .scaledToFill()                   // 填充+裁切,不变形 ≈ ContentScale.Crop
                    .ignoresSafeArea()

                PhotosPicker(selection: $pickerItem, matching: .images) {
                    Text("选图")
                        .font(.headline)
                        .foregroundStyle(.black)
                        .padding(.horizontal, 28)
                        .padding(.vertical, 12)
                        .background(.white.opacity(0.9), in: Capsule())
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
