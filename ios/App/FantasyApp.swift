import SwiftUI

@main
struct FantasyApp: App {
    var body: some Scene {
        WindowGroup {
            HomeView()
        }
    }
}

// 选好待编辑的图(RGBA8 像素 + 尺寸)。用 class + 身份相等,
// 避免把大块像素 Data 塞进 NavigationPath 做哈希/拷贝。
final class PickedImage: Hashable {
    let data: Data
    let width: Int
    let height: Int

    init(data: Data, width: Int, height: Int) {
        self.data = data
        self.width = width
        self.height = height
    }

    static func == (lhs: PickedImage, rhs: PickedImage) -> Bool { lhs === rhs }
    func hash(into hasher: inout Hasher) { hasher.combine(ObjectIdentifier(self)) }
}
