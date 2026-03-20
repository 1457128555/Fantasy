# Fantasy — Android 图片滤镜 App

## 项目简介

Fantasy 是一个 Android 图片滤镜应用，类似 InShot，目前只做图片模式。底层用 C++ 实现跨平台渲染核心，上层用 Kotlin 做 Android 原生 UI。

## 架构分层

```
Android UI (Kotlin)
       ↓ JNI（粗粒度，传配置）
Filter Framework (C++，纯链式管线，key-value 参数表)
       ↓
RHI 协议层 (C++ 抽象接口)
       ↓
GLES 3.0 实现
```

### 各层职责

- **Android UI 层**：原生 Kotlin，负责界面交互、图片加载、预览显示
- **JNI 桥接层**：粗粒度接口，Java 侧传配置（滤镜链 + 参数）和图片 byte[]，C++ 侧一次性处理并返回结果
- **Filter Framework 层**：C++ 实现的滤镜框架，仿 GPUImage 的链式管线，每个滤镜用统一的 key-value 参数表
- **RHI 层**：渲染硬件抽象接口，隔离上层与具体图形 API，当前只实现 GLES 3.0 后端，未来可扩展 Metal / Vulkan

## 核心设计决策

### RHI 层

- **执行模型**：立即执行，调用即生效，不做 CommandBuffer 录制
- **线程模型**：单线程，不做多线程渲染
- **资源生命周期**：智能指针管理（std::shared_ptr / std::unique_ptr）
- **资源创建**：各资源类提供静态工厂方法，不使用统一的 Device 对象
- **Shader 管理**：RHI 内部管理 shader 源码，上层按名字引用，不暴露 GLSL
- **VertexLayout**：灵活描述，可自定义属性（名字、类型、偏移、步长等）
- **Uniform 传参**：绑定参数字典/结构体（RHIUniformSet），draw 时自动提交到 GPU
- **错误处理**：抛异常（std::runtime_error 或自定义异常类）

### Filter Framework 层

- **拓扑结构**：纯链式 A → B → C，不支持 DAG
- **参数系统**：统一 key-value 参数表（FilterParam），支持 float / int / vec 等类型
- **初期滤镜**：亮度（Brightness）、对比度（Contrast）、饱和度（Saturation）

### JNI 桥接层

- **接口粒度**：粗粒度，Java 侧组装好配置传下去，C++ 一次性处理
- **数据传递**：Bitmap → byte[] 传入 C++，处理后 byte[] 返回 Java 侧还原为 Bitmap
- **预览**：渲染到屏幕显示
- **导出**：离屏渲染输出 Bitmap，保存到相册

## 目录结构

```
Fantasy/
├── CLAUDE.md                         # 本文件，项目上下文
├── PLAN.md                           # 开发计划与进度
│
├── app/                              # Android 应用层
│   └── src/main/
│       ├── java/com/fantasy/
│       │   ├── MainActivity.kt            # 主界面，PickVisualMedia + EditorScreen
│       │   ├── bridge/
│       │   │   └── NativeBridge.kt        # JNI 接口声明
│       │   ├── viewmodel/
│       │   │   └── EditorViewModel.kt     # 图片加载、滤镜参数、debounce 渲染
│       │   └── ui/
│       │       ├── EditorScreen.kt        # 编辑器主界面 Composable
│       │       └── components/
│       │           ├── TopToolBar.kt      # 内置图片/相册选图按钮
│       │           ├── ImagePreview.kt    # 图片预览 + loading 指示器
│       │           └── FilterPanel.kt     # 滤镜参数滑条
│       ├── res/
│       │   └── drawable/
│       │       └── builtin_sample.jpg     # 内置样例图
│       └── AndroidManifest.xml
│
├── native/                           # C++ 层
│   ├── CMakeLists.txt                # 顶层 CMake
│   │
│   ├── rhi/                          # RHI 抽象层
│   │   ├── CMakeLists.txt
│   │   ├── include/rhi/             # 公开头文件
│   │   │   ├── RHITexture.h
│   │   │   ├── RHIShader.h
│   │   │   ├── RHIFramebuffer.h
│   │   │   ├── RHIVertexBuffer.h
│   │   │   ├── RHIVertexLayout.h
│   │   │   ├── RHIUniformSet.h
│   │   │   ├── RHIRenderer.h
│   │   │   └── RHIRenderState.h
│   │   └── src/
│   │       └── gles/                # GLES 3.0 后端实现
│   │           ├── GLTexture.cpp
│   │           ├── GLShader.cpp
│   │           ├── GLFramebuffer.cpp
│   │           ├── GLVertexBuffer.cpp
│   │           ├── GLRenderer.cpp
│   │           └── ...
│   │
│   ├── filter/                       # Filter Framework 层
│   │   ├── CMakeLists.txt
│   │   ├── include/filter/
│   │   │   ├── Filter.h
│   │   │   ├── FilterChain.h
│   │   │   ├── FilterParam.h
│   │   │   └── filters/
│   │   │       ├── BrightnessFilter.h
│   │   │       ├── ContrastFilter.h
│   │   │       └── SaturationFilter.h
│   │   └── src/
│   │       ├── Filter.cpp
│   │       ├── FilterChain.cpp
│   │       └── filters/
│   │           ├── BrightnessFilter.cpp
│   │           ├── ContrastFilter.cpp
│   │           └── SaturationFilter.cpp
│   │
│   └── bridge/                       # JNI 桥接层
│       ├── CMakeLists.txt
│       └── src/
│           └── NativeBridge.cpp
│
└── build.gradle / settings.gradle
```

## 开发阶段

详见 PLAN.md。共 5 个 Phase：

1. **Phase 1 — RHI 层基础**：搭建项目，实现 RHI 全部资源类型，验证 passthrough 渲染管线
2. **Phase 2 — Filter Framework 基础**：滤镜基类、链式管线、BrightnessFilter
3. **Phase 3 — JNI 桥接 + 预览**：打通 Java ↔ C++，屏幕上看到滤镜效果
4. **Phase 4 — 补全基础滤镜**：Contrast、Saturation，UI 滑条调参
5. **Phase 5 — 离屏导出**：离屏渲染输出 Bitmap，保存到相册

## 编码规范

### C++ 层

- C++ 标准：C++17
- 命名风格：
  - 类名：PascalCase（如 RHITexture、BrightnessFilter）
  - 方法名：camelCase（如 createTexture、setParam）
  - 成员变量：m_ 前缀（如 m_width、m_textureId）
  - 常量/枚举值：UPPER_SNAKE_CASE
- 头文件用 `#pragma once`
- RHI 抽象接口定义在 `include/rhi/` 下，GLES 实现放 `src/gles/` 下
- 每个 RHI 资源类都有静态工厂方法 `create(...)`，返回 `std::shared_ptr<T>`
- Shader 源码以字符串常量形式存储在 GLES 实现内部，不暴露给上层

### Kotlin 层

- 包名：com.fantasy
- 使用 Kotlin 协程处理异步操作
- JNI 方法集中在 NativeBridge.kt 中声明

### 构建

- Android Gradle Plugin + CMake
- native 库通过 CMake 编译为 .so
- minSdk：24（Android 7.0，保证 GLES 3.0 支持）
- targetSdk：34

## 重要约束

- **目前只做图片模式**，不考虑视频
- **不直接调用 OpenGL**，所有渲染操作必须经过 RHI 层
- **滤镜框架不依赖 Android 平台**，保持纯 C++ 实现以便未来跨平台
- **JNI 层尽量薄**，业务逻辑不放在桥接层

## 当前进度

- [x] Phase 1 — RHI 层基础（已完成：Texture, Shader, VertexBuffer, VertexLayout, Framebuffer, UniformSet, Renderer, 完整 passthrough 管线验证通过）
- [x] Phase 2 — Filter Framework 基础（已完成：FilterParam, Filter 基类, FilterChain, BrightnessFilter, 测试通过）
- [x] Phase 3 — JNI 桥接 + 预览（已完成：nativeApplyFilters JNI 接口, EditorViewModel, 编辑器 UI, 内置图片 + 相册选图, 亮度滤镜预览）
- [x] Phase 4 — 补全基础滤镜（已完成：ContrastFilter, SaturationFilter, UI 滑条全部启用, 多滤镜链式叠加实时调参）
- [x] Phase 5 — 离屏导出（已完成：exportImage 全分辨率离屏渲染, MediaStore 保存到相册, Toast 反馈）
