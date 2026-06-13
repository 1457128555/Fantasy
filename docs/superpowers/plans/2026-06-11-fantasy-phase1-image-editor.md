# Fantasy 第一阶段（图片编辑）实现计划

> **执行方式：** 本计划由用户本人以学习模式执行，Claude 担任概念讲解、code review 和调试协助（见 spec 第 7 节）。**不适用** subagent/agent 自动执行。
> 对应 spec：`docs/superpowers/specs/2026-06-11-fantasy-image-editor-design.md`

**目标：** 做出一个可用的 Android 图片编辑 app（选图 → 滤镜/调色/裁剪 → 导出相册），同时系统学习 Android/Kotlin/NDK。

**架构：** Kotlin (Compose UI + ViewModel) / 薄 JNI 边界 / C++20 引擎（EditSession + EffectChain + Renderer，GLES3 后端，轻量 RHI 抽象）。单渲染线程 + 命令队列。

**技术栈：** Kotlin、Jetpack Compose、NDK r27+、CMake、C++20、GLES3、stb_image、GoogleTest。

---

## 使用方式

- 每个里程碑按顺序做：**概念铺垫 → 动手步骤 → 验收 → review**。
- 概念铺垫：开工前找 Claude 把"先学"清单过一遍（也可自己查资料后只问不懂的）。
- 步骤里打 `- [ ]` 的是你要做的事；标 🔧 的是工程脚手架，可以直接让 Claude 写。
- 每完成一个里程碑：真机跑一遍全流程 → commit → 找 Claude review 代码 → 铺垫下一个。
- 渲染/shader 相关任务不做概念铺垫——那是你的熟练区，直接写。

## 最终文件结构（全期视图）

```
Fantasy/
├── app/                                # Kotlin app module
│   └── src/main/java/com/fan/fantasy/
│       ├── MainActivity.kt             # 入口 + Navigation
│       ├── picker/PickerScreen.kt      # 选图入口（Photo Picker）
│       ├── editor/
│       │   ├── EditorScreen.kt         # 编辑主屏（预览+工具栏+滑杆）
│       │   ├── EditorViewModel.kt      # UI 状态机、撤销栈
│       │   ├── PreviewSurface.kt       # AndroidView 包 SurfaceView
│       │   └── CropOverlay.kt          # 裁剪框手势交互（M8）
│       └── export/Exporter.kt          # Bitmap + MediaStore 写入
├── core/                               # 纯 C++ 库（plain CMake，零平台依赖）
│   ├── CMakeLists.txt                  # 产出静态库 fantasy_core
│   ├── include/fantasy/                # 公开头文件
│   │   ├── gl_context.h                # IGLContext 注入接口（EGL/EAGL 各自实现）
│   │   ├── logger.h                    # ILogger 注入接口
│   │   ├── edit_session.h             # 编辑状态（不碰 GL）
│   │   ├── effect.h                    # Effect 接口 + 参数声明
│   │   └── rhi/rhi.h                   # Texture/Shader/RenderTarget 抽象
│   ├── src/
│   │   ├── command_queue.h             # 线程安全命令队列
│   │   ├── render_thread.cpp           # 渲染线程（makeCurrent 注入的 context）
│   │   ├── edit_session.cpp            # 编辑状态
│   │   ├── effects/                    # BrightnessEffect、LutEffect 等
│   │   ├── rhi/gles/                   # GLES3 后端（draw 调用，两端通用）
│   │   ├── renderer.cpp                # EditSession → draw 序列
│   │   └── third_party/stb/            # stb_image, stb_image_write
│   └── test/                           # GoogleTest（跑在 Mac，M6 起）
├── engine/                       # Android Library module（Android 胶水）
│   ├── src/main/java/com/fan/engine/
│   │   └── EngineBridge.kt       # 唯一 JNI 绑定类
│   └── src/main/cpp/
│       ├── CMakeLists.txt              # add_subdirectory(core) 链 fantasy_core
│       ├── jni_bridge.cpp              # JNI 入口，只做转发
│       └── egl_context.cpp            # 用 EGL 实现 core 的 IGLContext（M2）
├── platform/ios/                       # 【未来】iOS 胶水，复用同一 core
└── docs/superpowers/{specs,plans}/
```

铁律：`core/` 下任何文件都不许 `#include` 平台头（jni/android/EGL/OpenGLES）；
平台专属实现（EGL context、JNI 转发、日志）住 `engine/src/main/cpp/`。

**EngineBridge 完整 API（各里程碑逐步填充，命名以此为准）：**

```kotlin
object EngineBridge {
    external fun nativeHello(): String                    // M1，M2 后删除
    external fun initialize()                             // M2
    external fun release()                                // M2
    external fun attachSurface(surface: Surface)          // M2
    external fun detachSurface()                          // M2
    external fun openImage(fd: Int): Boolean              // M3
    external fun setParam(key: String, value: Float)      // M4
    external fun exportImage(): ByteArray?                // M5，RGBA8，前 8 字节为宽高
    external fun getImageSize(): IntArray                 // M5，[width, height]
    external fun setGeometry(cropL: Float, cropT: Float,  // M8，归一化裁剪框
        cropR: Float, cropB: Float, rotateQuarters: Int,  //     旋转=90°的倍数
        flipH: Boolean, flipV: Boolean)
    external fun snapshotParams(): String                 // M9，JSON，撤销用
    external fun restoreParams(json: String)              // M9
}
```

---

## 里程碑 0：空壳 app

**先学（Claude 铺垫，约一次对话）：**
- Android 项目解剖：Gradle/AGP、module、`build.gradle.kts`、manifest 各自管什么
- Activity 是什么、生命周期最小集（onCreate/onResume/onPause/onDestroy）
- Compose 心智模型：`@Composable` 函数、声明式 vs 命令式（对比你熟悉的"每帧重画" immediate mode，Compose 是"状态变了才重组"）
- Kotlin 第一课：`val/var`、函数语法、字符串模板、空安全 `?.`/`!!`

**步骤：**
- [ ] 安装/更新 Android Studio，SDK + NDK + CMake 组件
- [ ] 新建项目：模板 Empty Activity（Compose），包名 `com.fan.fantasy`，项目位置就是本仓库根目录
- [ ] 真机开 USB 调试，跑起来
- [ ] 把默认文字改成自己的，再跑一遍，体会一次"改码→部署"循环
- [ ] 🔧 确认 `.gitignore` 覆盖 `build/`、`.idea/`、`local.properties`
- [ ] `git add -A && git commit -m "feat: M0 空壳 app"`

**验收：** 真机显示你写的文字。
**Review 点：** 无代码可审，过一遍项目结构即可。

---

## 里程碑 1：JNI 打通

**先学：**
- NDK 是什么：交叉编译工具链 + 平台头文件/库，产物是 `.so`
- JNI 机制：`System.loadLibrary`、`Java_包名_类名_方法名` 命名规则、`JNIEnv*`、`jstring` 与 C 字符串互转
- Kotlin `external fun` = Java `native` 方法
- Gradle 的 `externalNativeBuild` 怎么把 CMake 挂进构建
- Kotlin 第二课：`object`（单例）、companion object、`init` 块

**步骤：**
- [x] 新建 module：Android Library，名为 `engine`，包 `com.fan.engine`
- [x] 🔧 配置 `engine/build.gradle.kts`（externalNativeBuild + abiFilters）+ `engine/src/main/cpp/CMakeLists.txt`（库名 `engine`，C++20，链 `log` + `fantasy_core`）+ 顶层 `core/` 骨架（Claude 写）
- [ ] **第一步小验证**：Sync + Build，确认 `libengine.so`（含空 `fantasy_core`）编出来 → 跨平台构建链路通
- [ ] 写 `jni_bridge.cpp`：实现 `nativeHello`，返回 C++ 构造的字符串（顺手 `__android_log_print` 打日志，学会看 Logcat）
- [ ] 写 `EngineBridge.kt`：`object` + `init { System.loadLibrary("engine") }` + `external fun nativeHello(): String`
- [ ] app module 依赖 engine（`implementation(project(":engine"))`），主屏显示 `EngineBridge.nativeHello()` 的返回值
- [ ] 真机验证 + `git commit -m "feat: M1 JNI 打通"`

**验收：** 屏幕显示来自 C++ 的字符串；Logcat 能看到 C++ 打的日志。
**Review 点：** JNI 命名、字符串转换的内存归属（谁分配谁释放）。
**注：** `nativeHello` 这次自洽地写在 JNI 胶水里即可（不绕 core）；core 的第一段真实代码从 M2 开始。

---

## 里程碑 2：GL 三角形

> **进度（2026-06-13）：** core 骨架已写并编译通过（`Common::Singleton`/`Macros`/
> `Logger`/`Engine`/`Render::System`/`Render::Context`，host + Android arm64 全过）。
> 实际目录是 `core/include/{Common,Render}/ + Engine.h`、`core/src/` 镜像（非下方
> "最终文件结构"里的 runtime/platform 布局——那是早期设想，以实际为准）。
> **均为空壳**，M2 真正内容（IGLContext 接口、CommandQueue、RenderThread、EGL 胶水、
> 三角形、生命周期）未写。下一步：先写 `IGLContext` 接口。

**先学：**
- `SurfaceView` / `Surface` / `ANativeWindow` 三者关系（Surface 是跨进程的图形缓冲队列句柄，C++ 侧用 `ANativeWindow_fromSurface` 拿到原生窗口）
- EGL 在 Android 上的形态（你在 InShot 用过，重点是这次**自己从零搭**：eglGetDisplay → ChooseConfig → CreateContext → CreateWindowSurface → MakeCurrent）
- Surface 生命周期回调：`surfaceCreated` / `surfaceChanged` / `surfaceDestroyed`，和渲染线程的协调（destroyed 时必须同步等渲染线程释放 EGLSurface）
- Kotlin 第三课：lambda 与尾随 lambda（Compose 到处都是）、`Unit`、接口实现

**核心/胶水划线（本里程碑第一次实践跨平台分层）：**
- **core**：`CommandQueue`、`RenderThread`、`IGLContext` 接口、渲染逻辑——零平台头
- **engine 胶水**：用 EGL 实现 `IGLContext`（`egl_context.cpp`）、`jni_bridge.cpp` 转发

**步骤：**
- [ ] core：定义 `IGLContext` 接口（`makeCurrent()` / `swapBuffers()` / `destroy()`），放 `core/include/fantasy/gl_context.h`
- [ ] core：写 `CommandQueue`（`std::mutex` + `std::condition_variable` + `std::deque<std::function<void()>>`，注意退出语义）
- [ ] core：写 `RenderThread`：`std::thread` 跑 loop，启动时 `ctx->makeCurrent()`，从队列取命令执行；提供 `post(fn)` 和 `quitAndJoin()`
- [ ] engine 胶水：用 EGL 实现 `IGLContext`（`ANativeWindow_fromSurface` 拿窗口 → eglCreateWindowSurface/Context）
- [ ] engine 胶水：`jni_bridge.cpp` 实现 `initialize/release/attachSurface/detachSurface`，构造 EGL context 注入 core，全部转发到命令队列
- [ ] core：attach 后渲染一帧：clear 成深灰 + 画一个带顶点色的三角形（shader 内联硬编码即可，M3 重构进 RHI）
- [ ] Kotlin：写 `PreviewSurface.kt`：`AndroidView` 包 `SurfaceView`，`SurfaceHolder.Callback` 三回调调 `EngineBridge` 的 attach/detach
- [ ] 处理 Activity 生命周期：`onDestroy` 时 `release()`；旋转屏幕/退后台再回来不崩、不黑屏
- [ ] 真机验证 + `git commit -m "feat: M2 渲染线程画出三角形"`

**验收：** 真机 SurfaceView 显示三角形；旋转屏幕、退后台再回来均正常。
**Review 点：** 线程边界（哪些调用在 UI 线程、哪些在渲染线程）、surfaceDestroyed 的同步等待、EGL 资源释放顺序。**这是全项目地基，值得仔细 review。**

---

## 里程碑 3：显示图片

**先学：**
- Photo Picker（`PickVisualMedia` ActivityResult contract）：零权限选图，返回 `Uri`
- `Uri` 不是路径：要经 `ContentResolver.openFileDescriptor` 拿 fd，把 **fd（Int）** 传给 C++（这就是 JNI 边界"只传命令和参数"的体现）
- Kotlin 第四课：`null` 处理惯用法（`?.let { }`、Elvis `?:`）、`use { }`（≈ RAII）

**步骤：**
- [ ] 🔧 把 stb_image/stb_image_write 放进 `third_party/stb/`，挂进 CMake
- [ ] C++：用 `dup(fd)` + `fdopen` + `stbi_load_from_file` 解码 RGBA8（注意 fd 归属：Kotlin 侧 `use` 会关原 fd，所以 C++ 要 dup）
- [ ] C++：搭 RHI 初版：`rhi.h` 定义 `Texture`、`Shader`、`RenderTarget` 接口 + `Device` 工厂；GLES 实现；把 M2 的三角形代码重构进来
- [ ] C++：`Renderer`：上传图片纹理，按图片宽高比 letterbox 画全屏 quad
- [ ] Kotlin：`PickerScreen` 一个按钮唤起 Photo Picker，选中后导航到 `EditorScreen`，fd 传给 `EngineBridge.openImage`
- [ ] 真机验证 + `git commit -m "feat: M3 选图并显示"`

**验收：** 选任意相册图片，正确显示、比例不变形；横竖图都对；选超大图（≥50MP）不崩（解码时按需降采样，上限边长 4096）。
**Review 点：** fd 生命周期、RHI 接口边界是否干净（GL 类型有没有泄漏到 rhi.h）。

---

## 里程碑 4：第一个效果（亮度）

**先学（本里程碑是 Kotlin/架构重点）：**
- `ViewModel` 存在的意义（配置变更存活）、`viewModelScope`
- `StateFlow` + `collectAsStateWithLifecycle`：状态从 ViewModel 流向 Compose 的标准管道
- 单向数据流（UDF）：UI 发事件 → ViewModel 改状态 → UI 订阅重组（和你熟悉的"渲染状态机"对照讲）
- Kotlin 第五课：data class、`copy()`、sealed class/interface

**步骤：**
- [ ] C++：定义 `Effect` 接口（`name()`、`parameters()` 返回参数描述、`apply(device, src, dst)`）和 `EffectChain`；`EditSession` 持有链 + 参数值表
- [ ] C++：实现 `BrightnessEffect`；`setParam` 命令更新 EditSession 后请求重绘
- [ ] C++：命令队列加"丢帧合并"：渲染请求 pending 时不重复入队，参数永远取最新（latest-wins）
- [ ] Kotlin：`EditorViewModel`：`data class EditorUiState(val brightness: Float = 0f, ...)` + `StateFlow`；`onBrightnessChange` 同时更新 state 和调 `EngineBridge.setParam("brightness", v)`
- [ ] Kotlin：`EditorScreen` 加 `Slider`，绑定 ViewModel
- [ ] 真机验证 + `git commit -m "feat: M4 亮度调节"`

**验收：** 快速来回拖滑杆，预览实时跟手无卡顿、无积压延迟；旋转屏幕后滑杆值和画面效果保持。
**Review 点：** UDF 是否成环（状态只有一个来源）、latest-wins 实现、Effect 参数声明设计。

---

## 里程碑 5：导出 🏁 第一个完整可用版本

**先学：**
- 协程基础：`suspend`、`Dispatchers.IO`、`withContext`（导出是慢操作，不能卡 UI 线程）
- `Bitmap.createBitmap` + `copyPixelsFromBuffer`；JPEG 压缩
- MediaStore 写入流程（`ContentValues` + `IS_PENDING` 标记）

**步骤：**
- [ ] C++：`exportImage`：投递到渲染线程，按**原图分辨率**走离屏 FBO 渲染完整效果链，回读 RGBA 返回（JNI 侧用 `NewByteArray`；调用方阻塞等待渲染线程完成，Kotlin 侧负责不在主线程调它）
- [ ] Kotlin：`Exporter.kt`：`suspend fun export()` 在 `Dispatchers.IO` 调 `exportImage()` + `getImageSize()`，转 Bitmap，压 JPEG（质量 95），写 MediaStore
- [ ] Kotlin：导出按钮 + 导出中 loading 态 + 成功/失败提示（错误码 → sealed class `ExportResult`）
- [ ] 真机验证 + `git commit -m "feat: M5 导出到相册"`

**验收:** 调过亮度的图出现在系统相册，分辨率与原图一致、效果正确；导出期间 UI 不冻结、可见 loading；对 50MP 大图导出不 OOM、不崩。
**Review 点：** 阻塞 JNI 调用 + IO dispatcher 的组合是否正确、Bitmap 内存及时回收、IS_PENDING 收尾。

---

## 里程碑 6：调色组 + 引擎单元测试

**先学：**
- GoogleTest 接入：CMake 在 Mac 上单独构建 `core/`（不依赖 GL/Android 正是为了这一步）
- Kotlin 无新内容，本里程碑重心在 C++ 架构

**步骤：**
- [ ] C++：实现调色参数组：亮度、对比度、饱和度、色温、色调、曝光（6 个；合并为一个 `ColorAdjustEffect` 单 pass，shader 你闭眼写）
- [ ] C++：`Renderer` 实现 ping-pong FBO：链上多个激活 effect 依次执行（几何 → 调色 → LUT 的固定顺序在 `EffectChain` 里定义）
- [ ] 🔧 配 Mac 侧 CMake preset + GoogleTest 拉取
- [ ] C++：给 `EditSession`/`EffectChain`/参数系统写单测：参数 set/get/范围 clamp、链顺序、快照/恢复（为 M9 预埋）
- [ ] Kotlin：调节 tab 做成参数列表（每行名字 + 滑杆），数据驱动（参数描述从 Effect 声明来，UI 不硬编码参数名）
- [ ] 真机验证 + 跑单测 + `git commit -m "feat: M6 调色组与引擎单测"`

**验收：** 6 个参数任意组合叠加效果正确；`ctest` 在 Mac 全绿；UI 参数列表由引擎声明驱动（C++ 加一个参数，Kotlin 不改代码就出现对应滑杆）。
**Review 点：** ping-pong 资源复用、参数声明→UI 的数据流。

---

## 里程碑 7：LUT 滤镜

**步骤（无新概念铺垫，纯熟练区 + 资源管理）：**
- [ ] 准备 5-8 个 LUT（512×512 的 2D 展开格式），放 `app/src/main/assets/luts/`
- [ ] Kotlin：`AssetManager` 读 LUT 字节流传给引擎（经 fd 或 byte[]，沿用"边界只传数据"原则）
- [ ] C++：`LutEffect`：LUT 纹理采样 + `intensity` 参数做 mix
- [ ] Kotlin：滤镜 tab：横向缩略图列表（用 Compose `LazyRow`，顺便学 lazy 布局）+ 选中态 + 强度滑杆
- [ ] 真机验证 + `git commit -m "feat: M7 LUT 滤镜"`

**验收：** 滤镜可切换、强度可调、和调色参数正确叠加；快速连续切换滤镜不闪烁不泄漏。
**Review 点：** LUT 纹理缓存策略（全部常驻 or LRU）。

---

## 里程碑 8：裁剪/旋转/翻转

**先学：**
- Compose 手势体系：`pointerInput` + `detectDragGestures` / `detectTransformGestures`
- `Canvas` Composable 画裁剪框遮罩

**步骤：**
- [ ] Kotlin：`CropOverlay.kt`：预览上叠裁剪框（四角拖拽 + 整框平移），框内亮框外暗
- [ ] Kotlin：旋转 90°、水平/垂直翻转三个按钮
- [ ] C++：`setGeometry` 进 `EditSession`；`Renderer` 在链首应用几何变换（顶点变换 + 视口适配，注意旋转后宽高比交换）
- [ ] 打通预览坐标 ↔ 图片归一化坐标的换算（letterbox 偏移要算对，这是本里程碑最容易错的地方）
- [ ] 导出验证几何变换生效
- [ ] 真机验证 + `git commit -m "feat: M8 裁剪旋转翻转"`

**验收：** 裁剪框拖拽流畅、不出界；旋转/翻转即时生效；导出结果与预览所见一致（重点验证裁剪+旋转组合）。
**Review 点：** 坐标系换算、手势状态管理。

---

## 里程碑 9：撤销/重做 + 收尾

**先学：**
- 命令模式 vs 快照模式做撤销的取舍（本项目用**快照**：参数集小，全量快照最简单——YAGNI）

**步骤：**
- [ ] C++：`snapshotParams()/restoreParams()`（JSON 序列化 EditSession 参数，M6 单测已覆盖核心逻辑）
- [ ] Kotlin：ViewModel 撤销栈：每次"操作结束"（滑杆松手、滤镜切换、几何确认）压栈；undo/redo 按钮 + 可用态
- [ ] Kotlin：对比原图按钮（按住显示原图，松开恢复——引擎加 `setParam("bypass", 1f)` 走直通）
- [ ] 整体打磨：异常路径过一遍（无图退出、导出失败、低内存）、启动图标和 app 名
- [ ] ViewModel 撤销栈写 JUnit 测试
- [ ] 真机验证 + `git commit -m "feat: M9 撤销重做与收尾"`

**验收：** 任意操作序列 undo/redo 行为正确（含撤销到底、重做后新操作截断 redo 栈）；按住对比原图即时切换；全流程无崩溃。
**Review 点：** "操作结束"的界定（拖动中不压栈）、栈上限。

---

## 第一阶段完成标志

真机演示完整流程：选图 → 套滤镜调强度 → 调 3 个以上调色参数 → 裁剪+旋转 → 撤销两步再重做 → 导出 → 相册查看，全程流畅无崩溃。

之后进入第二阶段规划（另起 spec）：端侧 ML（人像分割）→ 视频编辑。
