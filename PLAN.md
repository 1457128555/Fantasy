# Fantasy — 图片滤镜 App 开发计划

## 项目概述

一个 Android 图片滤镜应用，类似 InShot，目前只考虑图片模式。

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

## 各层设计约定

### RHI 层

- 立即执行，单线程
- 资源用智能指针管理（shared_ptr / unique_ptr）
- 各资源类提供静态工厂方法创建实例
- Shader 由 RHI 内部管理，上层按名字引用
- 灵活的 VertexLayout 描述（可自定义属性名、类型、偏移）
- Uniform 绑定参数字典，draw 时自动提交
- 错误处理：抛异常

### Filter Framework 层

- 纯链式拓扑：A → B → C
- 统一 key-value 参数表
- 初期滤镜：亮度、对比度、饱和度

### JNI 桥接层

- 粗粒度接口：Java 传配置，C++ 一次性处理
- 图像数据：Bitmap → byte[] 传入 C++
- 预览：渲染到屏幕
- 导出：离屏渲染输出 Bitmap

## 目录结构

```
Fantasy/
├── app/                              # Android 应用层
│   └── src/main/
│       ├── java/com/fantasy/
│       │   ├── MainActivity.kt            # 主界面
│       │   ├── bridge/
│       │   │   └── NativeBridge.kt        # JNI 接口定义
│       │   └── ui/
│       │       └── PreviewView.kt         # 图片预览控件
│       ├── res/
│       └── AndroidManifest.xml
│
├── native/                           # C++ 层
│   ├── CMakeLists.txt                # 顶层 CMake
│   │
│   ├── rhi/                          # RHI 抽象层
│   │   ├── CMakeLists.txt
│   │   ├── include/rhi/
│   │   │   ├── RHITexture.h
│   │   │   ├── RHIShader.h
│   │   │   ├── RHIFramebuffer.h
│   │   │   ├── RHIVertexBuffer.h
│   │   │   ├── RHIVertexLayout.h
│   │   │   ├── RHIUniformSet.h
│   │   │   ├── RHIRenderer.h
│   │   │   └── RHIRenderState.h
│   │   └── src/
│   │       └── gles/                 # GLES 3.0 实现
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
│   │   │   ├── Filter.h                  # 滤镜基类
│   │   │   ├── FilterChain.h             # 链式管线
│   │   │   ├── FilterParam.h             # key-value 参数表
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

## 开发步骤

### Phase 1 — RHI 层基础

1. 搭建 Android 项目 + CMake 编译环境，确保 C++ 能跑起来
2. 实现 RHITexture（GLES 纹理创建、上传像素数据）
3. 实现 RHIShader（内置一个 passthrough shader，按名字获取）
4. 实现 RHIVertexBuffer + RHIVertexLayout（全屏四边形）
5. 实现 RHIFramebuffer（离屏渲染目标）
6. 实现 RHIUniformSet + RHIRenderer（绑定资源、执行 draw）
7. **测试点**：C++ 层能把一张纹理 passthrough 渲染到 Framebuffer 并读回像素，验证 RHI 管线跑通

### Phase 2 — Filter Framework 基础

1. 实现 Filter 基类 + FilterParam（key-value 参数表）
2. 实现 FilterChain（链式串联）
3. 实现第一个滤镜：BrightnessFilter
4. **测试点**：一张图片经过 BrightnessFilter 处理后像素值正确变化

### Phase 3 — JNI 桥接 + 预览 ✅

1. ~~实现 NativeBridge（JNI 接口，Java 传图片数据 + 滤镜配置）~~ → `nativeApplyFilters`：接收 byte[] + config 字符串，C++ 侧创建 EGL 上下文 → FilterChain 渲染 → readPixels 返回
2. ~~Android 侧加载图片，传 byte[] 到 C++，处理后返回结果~~ → EditorViewModel：图片加载（内置/相册）、EXIF 旋转、inSampleSize 降采样、1920px 长边缩放、100ms debounce + limitedParallelism(1) 线程安全
3. ~~PreviewView 显示处理后的图片~~ → Compose UI：TopToolBar + ImagePreview + FilterPanel（滑条列表），EditorScreen 组合
4. **测试点**：选一张图片，加亮度滤镜，屏幕上看到效果 ✅

### Phase 4 — 补全基础滤镜 ✅

1. ~~ContrastFilter~~ → 以 0.5 为中心缩放 RGB，[-1,1] 映射为乘数 [0,2]
2. ~~SaturationFilter~~ → luminance 灰度混合，[-1,1] 映射为混合系数 [0,2]
3. ~~UI 上加滑条调参数~~ → 三个滑条全部启用，链式叠加实时调参
4. **测试点**：多个滤镜链式叠加，参数实时调节 ✅

### Phase 5 — 离屏导出 ✅

1. ~~离屏渲染输出 Bitmap~~ → `exportImage()` 用当前参数对缓存的全分辨率 RGBA byte[] 调用 `nativeApplyFilters`，结果转 Bitmap
2. ~~保存到相册~~ → 通过 MediaStore API 写入 Pictures/Fantasy 目录，JPEG 95% 质量，支持 Android Q+ IS_PENDING 原子写入
3. **测试点**：导出的图片滤镜效果与预览一致 ✅

### Phase 6 — GLSurfaceView 实时渲染 ✅

用 GLSurfaceView 持久 EGL 上下文 + GPU 直出屏幕，替代旧的 readPixels+Bitmap 预览链路。

1. ~~RHI 层新增 `beginDefaultPass`~~ → 渲染到默认帧缓冲（屏幕）
2. ~~新增 `RenderSession` C++ 类~~ → 管理持久化 GL 渲染状态（纹理缓存、滤镜链复用）
3. ~~新增 6 个 JNI 方法~~ → 支持 GLSurfaceView 生命周期（init / setImage / setFilter / render / export / destroy）
4. ~~新增 `FantasyRenderer` + `GLPreview`~~ → GLSurfaceView.Renderer 实现 + Compose 封装
5. ~~`EditorViewModel` 重构~~ → 去掉 debounce / previewBitmap / isProcessing，直接驱动 GL 渲染
6. ~~导出改为 GL 线程离屏渲染~~ → 通过 `queueEvent` 复用同一 EGL 上下文
7. **测试点**：滑条拖动实时跟手预览，无卡顿 ✅

### Phase 7 — LUT 滤镜 ✅

新增 LUT（Look-Up Table）滤镜，支持一键预设风格。

1. ~~LUTFilter C++ 实现~~ → 3D LUT 以 2D atlas 存储，蓝色切片插值采样
2. ~~Kotlin 侧代码生成 LUT 数据~~ → `LUTPresets` 生成 512x512 RGBA LUT（暖色 / 冷色 / 复古）
3. ~~UI 横滑预设面板~~ → `PresetPanel` Composable + 强度滑条
4. ~~LUT 滤镜插入链首~~ → 与亮度 / 对比度 / 饱和度叠加
5. **测试点**：选择预设后一键切换风格，强度可调 ✅

### Phase 8 — 扩展滤镜（锐化 / 模糊 / 暗角）✅

1. ~~SharpenFilter~~ → 8 邻域 Laplacian 锐化，需要 texelSize，override apply()
2. ~~BlurFilter~~ → 5×5 Box Blur，采样间距按 blurRadius 缩放（最大 20 texel），需要 texelSize，override apply()
3. ~~VignetteFilter~~ → UV 到中心距离 + smoothstep 衰减，纯片段着色器，用基类 apply()
4. ~~UI 滑条~~ → 锐化 / 模糊 / 暗角各 0~1 范围，参数为 0 时跳过滤镜不创建 pass
5. ~~drawFrame Y 翻转~~ → 修正 Bitmap(top-first) 与 GL(bottom-first) 行序差异，仅上屏 blit 翻转 texcoord Y
6. **测试点**：七个滤镜链式叠加，参数实时调节，预览与导出方向一致 ✅

### Phase 9 — UI 分类 Tab ✅

将底部滤镜面板从平铺改为 Tab 分类布局。

1. ~~TabRow 三分类~~ → 预设 / 调色 / 效果
2. ~~预设 Tab~~ → LUT 预设横滑面板 + 强度滑条（选中预设后显示）
3. ~~调色 Tab~~ → 亮度、对比度、饱和度
4. ~~效果 Tab~~ → 锐化、模糊、暗角
5. ~~固定高度~~ → Tab 内容区 160dp 固定高度，切换不跳动
6. **测试点**：切换 Tab 不影响已设参数，预览正常 ✅

### Phase 10 — 裁剪 + 旋转

图片编辑核心功能：裁剪区域选择 + 旋转。

1. 裁剪 UI：手势拖拽裁剪框，支持自由比例和固定比例（1:1, 4:3, 16:9）
2. 旋转 UI：90° 快捷旋转按钮 + 自由旋转滑条
3. C++ 层：vertex shader 变换矩阵实现裁剪和旋转
4. 导出：按裁剪区域 + 旋转角度输出最终图片
5. **测试点**：裁剪 + 旋转 + 滤镜叠加后，预览与导出一致

### Phase 11 — 撤销/重做

参数快照栈，支持操作回退和重做。

1. 参数快照数据类：记录所有滤镜参数 + LUT 预设状态
2. 撤销/重做栈：纯 Kotlin 实现，每次参数变更压栈
3. UI：撤销/重做按钮（TopToolBar 或底部）
4. **测试点**：多次调参后撤销/重做，参数和预览正确恢复
