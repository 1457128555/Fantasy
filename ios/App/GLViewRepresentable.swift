import SwiftUI
import UIKit

struct GLViewRepresentable: UIViewRepresentable {
    func makeUIView(context: Context) -> FantasyGLView {
        FantasyGLView(frame: .zero)
    }

    func updateUIView(_ uiView: FantasyGLView, context: Context) {}
}
