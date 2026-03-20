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

### Phase 5 — 离屏导出

1. 离屏渲染输出 Bitmap
2. 保存到相册
3. **测试点**：导出的图片滤镜效果与预览一致
