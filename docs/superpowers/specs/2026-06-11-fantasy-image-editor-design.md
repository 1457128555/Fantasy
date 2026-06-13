# Fantasy — 图片/视频编辑 App 设计文档（第一阶段：图片编辑）

日期：2026-06-11
状态：已与用户逐节确认通过

## 1. 项目定位与目标

做一个类似 InShot 的移动端图片/视频编辑 app。**学习与作品并重**：
以学习节奏推进 MVP，架构按"可演进到上架产品"设计，避免一次性玩具代码。

学习目标（按优先级）：

1. Android 开发（Compose、平台能力、NDK）
2. Kotlin 语言（跳过 Java，只需能读懂）
3. 端侧深度学习推理（第二阶段引入）

用户背景：图形渲染工程师，C++/GLES 熟练（InShot 在职，Android filter 方向），
Android/Java 能力一般，正在学 Vulkan。时间投入不稳定，里程碑需切小、
每步独立可验证。

### 范围决策

- **第一阶段只做图片编辑**：滤镜 + 调色、裁剪/旋转/翻转。视频编辑是第二阶段。
- **渲染后端用 GLES3 起步**，预留轻量 RHI 抽象层，将来可加 Vulkan 后端。
  本项目不承担 Vulkan 练习场角色（一次只开一条学习主线）。
- **跨平台（架构原则，本期不实现 iOS）**：C++ 核心设计为平台无关的独立库
  `core/`，不依赖任何平台头（jni/android/EGL/OpenGLES）；平台服务（日志、
  GL context、文件 IO）通过接口由各平台胶水注入。Android 用 `RenderEngine`
  module 做胶水，将来 iOS 用 `platform/ios` 复用同一份 core。本期只建立这条
  接缝和纪律（成本≈0，因 core 从零起步），**不写 iOS 胶水**（YAGNI）。
  注：iOS 上 GLES 已 deprecated，正经做需在同一 RHI 接口后加 Metal 后端——
  这正是 RHI 抽象的第二个用途（除了 Vulkan）。
- **ML 第二阶段再引入**（端侧推理：人像分割/抠图等），第一阶段不碰。
- **推进路线：垂直切片**——第一个里程碑就打通完整链路，之后每个里程碑
  在已验证链路上加一个维度。

### 第一阶段 MVP 功能清单

- 系统 Photo Picker 选图
- 滤镜（内置 LUT，可调强度）
- 调色（亮度、对比度、饱和度、色温等 5-6 个参数，可叠加）
- 裁剪/旋转/翻转（手势交互）
- 导出保存到系统相册
- 撤销/重做、对比原图

明确不做（YAGNI）：贴纸/文字图层、画笔涂鸦、自建相册浏览、
云同步、多图批处理。

## 2. 总体架构

```
┌──────────────────────────┐   ┌──────────────────────────┐
│  Android app (Kotlin)    │   │  iOS app (Swift) 【未来】 │
│  Compose · ViewModel      │   │  SwiftUI                  │
├──────────────────────────┤   ├──────────────────────────┤
│  平台胶水：RenderEngine    │   │  平台胶水：platform/ios   │
│  JNI 绑定 + EGL context   │   │  Obj-C++ + EAGL/Metal     │
│  实现 IGLContext 注入 core │   │  实现 IGLContext 注入 core │
└─────────────┬────────────┘   └────────────┬─────────────┘
              │         两端共享同一份         │
              └──────────────┬───────────────┘
                             ▼
┌──────────────────────────────────────────────────────────┐
│  core/  纯 C++20 库（平台无关，零平台头）                    │
│  ┌───────────────┐  ┌──────────────────────────────────┐ │
│  │ 编辑模型        │  │ 渲染层                            │ │
│  │ EditSession    │  │ Renderer · RHI 抽象接口           │ │
│  │ 效果链/参数      │  │  └─ GLES3 后端（draw 调用，两端通用）│ │
│  └───────────────┘  │  IGLContext/ILogger 等注入接口     │ │
│                     │  （将来 Vulkan / Metal 后端）       │ │
│                     └──────────────────────────────────┘ │
└──────────────────────────────────────────────────────────┘
```

关键决策：

- **UI 用 Jetpack Compose**（不用 View/XML）。新项目标准选择，声明式模型
  通用性强，学习材料以它为主流。
- **core 是平台无关的独立库**：所有引擎逻辑住 `core/`，用 plain CMake 构建，
  零平台头。平台专属的东西（context 创建、JNI、日志）由胶水层实现注入接口
  （`IGLContext`、`ILogger` 等）。GL ES 的 *draw 调用* 两端通用、放 core；
  *context/surface 创建*（EGL vs EAGL）是平台胶水的活。
- **JNI 边界刻意做薄**：Kotlin 侧只发"命令 + 参数"，不在边界传像素数据。
  图片像素在 C++ 侧加载和持有，避免 JNI 数组拷贝，边界易维护。JNI 绑定本身
  是 Android 胶水（住 RenderEngine module），可以用 `__android_log_print` 等
  平台 API——铁律只约束 core。
- **RHI 抽象保持轻量**：只抽象实际用到的概念（纹理、shader/管线、
  framebuffer、draw）。GLES3 是当前唯一实现；抽象的目的是分离"渲染概念"
  与"GL 写法"——这同时服务三个未来：学 Vulkan 的思维转换、Vulkan 后端、
  iOS 的 Metal 后端。
- **显示路径**：Compose 嵌 `SurfaceView`，EGL surface 交给 C++ 渲染线程；
  导出走 FBO 离屏渲染 + 回读，Kotlin 侧写入 MediaStore。

## 3. C++ 引擎内部设计

核心对象三个：

- **`EditSession`**：一次编辑的全部状态——源图片、效果链、几何变换
  （裁剪/旋转/翻转）。纯数据 + 逻辑，不碰 GL，可单元测试。视频阶段
  复用此概念（视频 = 随时间变化的 EditSession）。
- **`EffectChain`**：有序效果节点列表。每个节点实现 `Effect` 接口
  （`BrightnessEffect`、`LutFilterEffect`…），声明自己的参数集
  （名字、范围、默认值）。MVP 阶段链是固定顺序
  （几何变换 → 调色 → LUT），接口按"可重排"设计。
- **`Renderer`**：持有渲染线程和 RHI，把 EditSession 解释成 draw call
  序列：源纹理 → 逐效果 ping-pong FBO → 输出（屏幕或回读）。
  **只有 Renderer 知道 GL 的存在。**

线程模型（第一天定死）：

- core 持有一条专属渲染线程，所有 GL 调用只在此线程。线程启动时调
  `IGLContext::makeCurrent()` 绑定 context——**context 由平台胶水创建并注入**
  （Android = EGL，iOS = EAGL），core 的渲染线程不知道自己跑在哪种 context 上。
- 平台胶水（JNI / Obj-C++）进来的调用只往渲染线程的命令队列投递消息
  （改参数、请求渲染、请求导出）。
- 视频阶段在此基础上扩展出解码/编码线程。

参数更新路径（最高频操作）：

滑杆拖动 → Kotlin `engine.setParam(key, value)` → JNI → 命令队列 →
渲染线程取最新值重绘。拖动期间多次更新只取最新（丢弃中间帧），保证跟手。

## 4. 工程结构与 Kotlin 侧

工程结构（顶层 core/ + 两个 Gradle module）：

```
Fantasy/
├── core/          # 纯 C++ 库（plain CMake）：引擎逻辑，零平台依赖
│   ├── include/fantasy/   # 公开头文件（含 IGLContext 等注入接口）
│   └── src/               # EditSession / Effect / Renderer / rhi/gles ...
├── RenderEngine/  # Android Library module：Android 胶水
│   └── src/main/cpp/      # jni_bridge.cpp + EGL 实现 IGLContext；
│                          #   CMake add_subdirectory(core) 链 fantasy_core
├── platform/ios/  # 【未来】iOS 胶水：Obj-C++ + EAGL/Metal，复用同一 core
└── app/           # Kotlin：UI、ViewModel、平台能力
```

分层意图：core 不属于任何平台（iOS 也能直接复用）；RenderEngine 是薄胶水，
app 无法绕过绑定类碰 native；换 UI / 加平台时 core 原样保留。
（命名：实际 module 名 `RenderEngine`，包名 `com.fan.renderengine`，
native 库 `librenderengine.so`。）

Kotlin 侧三层：

- **`RenderEngineBridge`**（RenderEngine module）：唯一 JNI 绑定类。
  `external fun` 声明 + `System.loadLibrary("renderengine")`；方法对应 C++
  命令：`createSession(path)`、`setParam(key, value)`、`setCrop(rect)`、
  `export(path)`、`attachSurface(surface)` 等。C++ 指针以 `Long` handle 持有，
  生命周期与 `close()` 显式绑定。
- **`EditorViewModel`**（app module）：编辑界面状态机。持有 UI 状态
  （选中工具、参数值、撤销栈），把用户操作翻译成 RenderEngineBridge 调用。
  协程与 `StateFlow` 在此学习。
- **Compose UI**：编辑主屏 = 预览区（`AndroidView` 包 `SurfaceView`）
  + 底部工具栏（滤镜/调节/裁剪三 tab）+ 参数滑杆区。图片选择入口用
  系统 Photo Picker（零权限申请）。

导出路径：C++ 渲染到 FBO → 回读 RGBA → JNI 回调把 buffer 交给 Kotlin →
`Bitmap` + `MediaStore` 存入系统相册。压缩编码与 MediaStore 刻意留在
Kotlin 侧（属于 Android 平台知识，是学习目标）。

错误处理边界：C++ 异常不穿过 JNI（catch 后转错误码）；Kotlin 侧把
错误码转 sealed class 结果类型给 UI。

## 5. 里程碑

每个里程碑独立闭环（做完真机可见，随时可暂停/恢复），粒度 5-15 小时：

| # | 里程碑 | 验收标准（真机） | 主要学习点 |
|---|---|---|---|
| 0 | 空壳 app | Compose 显示一句话 | 项目搭建、Gradle、Compose 入门 |
| 1 | JNI 打通 | 屏幕显示来自 C++ 的字符串 | NDK、CMake、JNI 基础 |
| 2 | GL 三角形 | C++ 渲染线程在 SurfaceView 画三角形 | EGL、渲染线程、命令队列 |
| 3 | 显示图片 | 选图 → C++ 解码上传纹理 → 显示 | 纹理上传、解码、Compose 嵌 SurfaceView |
| 4 | 第一个效果 | 亮度滑杆实时调节，跟手 | 效果链雏形、参数路径、StateFlow |
| 5 | 导出 | 编辑结果存入系统相册 | FBO 回读、Bitmap、MediaStore |
| 6 | 调色组 | 5-6 个调色参数，多效果叠加 | EffectChain 完整形态、ping-pong FBO |
| 7 | LUT 滤镜 | 多个 LUT 可切换、可调强度 | LUT 采样、资源管理 |
| 8 | 裁剪/旋转/翻转 | 裁剪框手势 + 应用到导出 | Compose 手势、几何变换矩阵 |
| 9 | 收尾 | 撤销/重做、对比原图、打磨 | 命令模式、状态管理 |

里程碑 5 完成即得到第一个完整可用的 app（选图 → 调亮度 → 保存）。

第二阶段方向（本期不展开）：ML 端侧推理（人像分割/抠图）→ 视频编辑
（播放、滤镜实时预览、导出）。架构预留：Effect 接口可挂 ML 效果，
线程模型可扩展解码/编码线程。

## 6. 验证策略

- **C++ 引擎逻辑**（EditSession、EffectChain、参数系统）：GoogleTest
  单元测试，跑在 Mac 上（不依赖 GL/Android）。
- **渲染正确性**：真机肉眼验证为主，每个里程碑的验收标准即验证手段。
  进阶可选 golden test（导出结果与预期图像素对比），MVP 不强制。
- **Kotlin 侧**：MVP 不追求 UI 测试覆盖；ViewModel 有可测逻辑
  （如撤销栈）时写 JUnit。
- **每个里程碑结束跑一次真机全流程**，确认旧功能未坏。

## 7. 协作方式（学习模式）

- **用户自己写全部代码**：Kotlin、JNI、C++ 引擎——这是学习目标本身。
- **Claude 的角色**：讲解概念（Kotlin 语法、协程、NDK 机制、Compose
  模型）、code review、调试分析、每个里程碑开始前铺垫概念。
- **例外**：纯工程脚手架（Gradle/CMake 样板）在用户明确说"帮我写"时
  可由 Claude 直接写。
- Kotlin 是用户的新语言：遇到惯用法（`?.`、`let`、data class、协程等）
  主动解释，并类比 C++ 概念。
