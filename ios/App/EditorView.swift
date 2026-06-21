import SwiftUI

// 编辑页:GL 渲染选好的图 + 左上「返回」。≈ Android EditorScreen.kt
struct EditorView: View {
    let image: PickedImage
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        ZStack(alignment: .topLeading) {
            GLViewRepresentable(image: image)
                .ignoresSafeArea()

            Button {
                dismiss()
            } label: {
                Text("返回")
                    .font(.headline)
                    .foregroundStyle(.black)
                    .padding(.horizontal, 16)
                    .padding(.vertical, 8)
                    .background(.white.opacity(0.85), in: Capsule())
            }
            .padding(16)
        }
        .navigationBarBackButtonHidden(true)   // 用自定义「返回」
    }
}
