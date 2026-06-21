import SwiftUI
import PhotosUI

@main
struct FantasyApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

struct ContentView: View {
    @State private var pickerItem: PhotosPickerItem?

    var body: some View {
        ZStack(alignment: .bottom) {
            GLViewRepresentable()
                .ignoresSafeArea()

            // 系统照片选择器（零权限，≈ Android PickVisualMedia）
            PhotosPicker(selection: $pickerItem, matching: .images) {
                Text("选图")
                    .font(.headline)
                    .foregroundStyle(.black)
                    .padding(.horizontal, 28)
                    .padding(.vertical, 12)
                    .background(.white.opacity(0.9), in: Capsule())
            }
            .padding(.bottom, 40)
        }
        .onChange(of: pickerItem) { newItem in
            guard let newItem else { return }
            Task { await load(newItem) }
        }
    }

    // 选中后：异步取出 Data → 解码 → 摊成 RGBA → 喂引擎
    private func load(_ item: PhotosPickerItem) async {
        guard let data = try? await item.loadTransferable(type: Data.self),
              let image = UIImage(data: data),
              let px = image.rgbaPixels() else { return }
        EngineBridge.setImageRGBA(px.data, width: Int32(px.width), height: Int32(px.height))
    }
}
