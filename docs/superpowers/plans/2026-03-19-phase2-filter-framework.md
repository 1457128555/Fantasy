# Phase 2 — Filter Framework 基础 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the filter framework layer — FilterParam, Filter base class, FilterChain, and BrightnessFilter — with full test coverage via JNI instrumented tests.

**Architecture:** Filters are C++ objects that each own a shader name and a `FilterParam` (key-value parameter map). Each Filter renders input texture → output FBO using the RHI layer. `FilterChain` chains filters sequentially with ping-pong FBOs. The first concrete filter is `BrightnessFilter` which adjusts pixel brightness via a `u_brightness` uniform.

**Tech Stack:** C++17, GLES 3.0 (via RHI), Android Instrumented Tests (Kotlin + JUnit4)

---

## File Structure

### New files to create:
| File | Responsibility |
|------|---------------|
| `native/filter/include/filter/FilterParam.h` | Key-value parameter table (wraps `RHIUniformSet`) |
| `native/filter/include/filter/FilterChain.h` | Chain of filters with ping-pong FBO execution |
| `native/filter/include/filter/filters/BrightnessFilter.h` | Brightness filter header |
| `native/filter/src/FilterChain.cpp` | FilterChain implementation |
| `native/filter/src/filters/BrightnessFilter.cpp` | Brightness filter + shader registration |

### Existing files to modify:
| File | Change |
|------|--------|
| `native/filter/include/filter/Filter.h` | Add full base class interface (apply, getShaderName, buildUniforms) |
| `native/filter/src/Filter.cpp` | Implement base class `apply()` using RHI pipeline |
| `native/filter/CMakeLists.txt` | Add new source files |
| `native/rhi/src/gles/GLShader.cpp` | Add `registerShader()` public API for filter shaders |
| `native/rhi/include/rhi/RHIShader.h` | Add `registerShader()` declaration |
| `native/bridge/src/NativeBridge.cpp` | Add filter test JNI functions |
| `app/src/main/java/com/fantasy/bridge/NativeBridge.kt` | Add filter test external declarations |

---

## Task 1: FilterParam — key-value 参数表

**Files:**
- Create: `native/filter/include/filter/FilterParam.h`

FilterParam is a thin wrapper providing a filter-friendly API that builds an `RHIUniformSet` internally. This keeps the filter layer decoupled from RHI internals while reusing its uniform system.

- [ ] **Step 1: Create FilterParam.h**

```cpp
// native/filter/include/filter/FilterParam.h
#pragma once

#include "rhi/RHIUniformSet.h"
#include <string>

namespace fantasy {
namespace filter {

class FilterParam {
public:
    void setFloat(const std::string& key, float value) {
        m_uniforms.setFloat(key, value);
    }

    void setInt(const std::string& key, int value) {
        m_uniforms.setInt(key, value);
    }

    void setFloat2(const std::string& key, float x, float y) {
        m_uniforms.setFloat2(key, x, y);
    }

    void setFloat3(const std::string& key, float x, float y, float z) {
        m_uniforms.setFloat3(key, x, y, z);
    }

    void setFloat4(const std::string& key, float x, float y, float z, float w) {
        m_uniforms.setFloat4(key, x, y, z, w);
    }

    const rhi::RHIUniformSet& toUniformSet() const { return m_uniforms; }

    void clear() { m_uniforms.clear(); }

private:
    rhi::RHIUniformSet m_uniforms;
};

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 2: Verify compiles**

No separate test needed — this is a header-only class that will be tested implicitly through Filter. Verify it compiles by building:

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 3: Commit**

```bash
git add native/filter/include/filter/FilterParam.h
git commit -m "feat(filter): add FilterParam key-value parameter table"
```

---

## Task 2: RHIShader — 添加 registerShader 公开接口

**Files:**
- Modify: `native/rhi/include/rhi/RHIShader.h:20` — add `registerShader` declaration
- Modify: `native/rhi/src/gles/GLShader.cpp:125-132` — add `registerShader` implementation

Filters need to register their own shaders. Currently only `registerBuiltins()` exists (hardcoded passthrough). Add a public `registerShader(name, vertexSrc, fragmentSrc)` so filters can register custom shaders.

- [ ] **Step 1: Add declaration to RHIShader.h**

Add after `static std::shared_ptr<RHIShader> get(const std::string& name);` (line 26):

```cpp
    // 注册自定义 shader（滤镜层使用）
    static void registerShader(const std::string& name,
                               const std::string& vertexSrc,
                               const std::string& fragmentSrc);
```

- [ ] **Step 2: Implement in GLShader.cpp**

Add after `registerBuiltins()` function (after line 132):

```cpp
void RHIShader::registerShader(const std::string& name,
                               const std::string& vertexSrc,
                               const std::string& fragmentSrc) {
    if (s_shaderRegistry.find(name) != s_shaderRegistry.end()) {
        return; // 已注册则跳过
    }
    s_shaderRegistry[name] = std::make_shared<GLShader>(vertexSrc, fragmentSrc);
    FANTASY_LOGI(TAG, "Custom shader registered: %s", name.c_str());
}
```

- [ ] **Step 3: Build to verify**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 4: Commit**

```bash
git add native/rhi/include/rhi/RHIShader.h native/rhi/src/gles/GLShader.cpp
git commit -m "feat(rhi): add registerShader for custom filter shaders"
```

---

## Task 3: Filter 基类 — 完整实现

**Files:**
- Modify: `native/filter/include/filter/Filter.h` — full base class interface
- Modify: `native/filter/src/Filter.cpp` — implement `apply()` using RHI
- Modify: `native/filter/CMakeLists.txt` — (no change yet, Filter.cpp already listed)

Filter base class does the heavy lifting: each subclass only provides a shader name and builds uniforms. The base class `apply()` method handles the full RHI render pass (create quad VBO, bind shader/texture/uniforms, draw, return output FBO's texture).

- [ ] **Step 1: Rewrite Filter.h with full interface**

```cpp
// native/filter/include/filter/Filter.h
#pragma once

#include "filter/FilterParam.h"
#include "rhi/RHITexture.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIRenderer.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include <memory>
#include <string>

namespace fantasy {
namespace filter {

class Filter {
public:
    virtual ~Filter() = default;

    // 设置滤镜参数
    void setParam(const std::string& key, float value);

    // 获取参数表
    const FilterParam& getParam() const { return m_param; }

    // 应用滤镜：输入纹理 → 输出纹理
    // renderer 由调用方（FilterChain）提供，保证复用
    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer);

protected:
    // 子类实现：返回使用的 shader 名字
    virtual std::string getShaderName() const = 0;

    // 子类实现：注册自己的 shader（仅首次调用时执行）
    virtual void registerShader() const = 0;

    // 子类实现：把 m_param 转为渲染用的 uniform
    // 默认实现直接返回 m_param.toUniformSet()，子类可覆盖
    virtual rhi::RHIUniformSet buildUniforms() const;

    FilterParam m_param;
};

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 2: Implement Filter.cpp**

```cpp
// native/filter/src/Filter.cpp
#include "filter/Filter.h"
#include "rhi/Log.h"

#define TAG "Filter"

namespace fantasy {
namespace filter {

void Filter::setParam(const std::string& key, float value) {
    m_param.setFloat(key, value);
}

rhi::RHIUniformSet Filter::buildUniforms() const {
    return m_param.toUniformSet();
}

std::shared_ptr<rhi::RHITexture> Filter::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    // 确保 shader 已注册
    registerShader();

    int w = input->getWidth();
    int h = input->getHeight();

    // 创建输出 FBO
    auto fbo = rhi::RHIFramebuffer::create(w, h, input->getFormat());

    // 全屏四边形
    float quadVertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };
    rhi::RHIVertexLayout layout;
    layout.add("aPosition", rhi::VertexAttribType::Float2, 0)
          .add("aTexCoord", rhi::VertexAttribType::Float2, 1);
    auto vbo = rhi::RHIVertexBuffer::create(quadVertices, sizeof(quadVertices), layout);

    // 获取 shader
    auto shader = rhi::RHIShader::get(getShaderName());

    // 构建 uniforms
    auto uniforms = buildUniforms();
    uniforms.setInt("uTexture", 0);

    // 渲染
    renderer->beginPass(fbo);
    renderer->setShader(shader);
    renderer->setTexture(input, 0);
    renderer->setVertexBuffer(vbo);
    renderer->setUniforms(uniforms);
    renderer->drawTriangleStrip(4);
    renderer->endPass();

    return fbo->getColorAttachment();
}

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 3: Build to verify**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 4: Commit**

```bash
git add native/filter/include/filter/Filter.h native/filter/src/Filter.cpp
git commit -m "feat(filter): implement Filter base class with RHI render pipeline"
```

---

## Task 4: BrightnessFilter — 第一个具体滤镜

**Files:**
- Create: `native/filter/include/filter/filters/BrightnessFilter.h`
- Create: `native/filter/src/filters/BrightnessFilter.cpp`
- Modify: `native/filter/CMakeLists.txt` — add BrightnessFilter.cpp

BrightnessFilter adjusts brightness by adding a `u_brightness` value (range -1.0 to 1.0) to each RGB channel. Default is 0.0 (no change).

- [ ] **Step 1: Create BrightnessFilter.h**

```cpp
// native/filter/include/filter/filters/BrightnessFilter.h
#pragma once

#include "filter/Filter.h"

namespace fantasy {
namespace filter {

class BrightnessFilter : public Filter {
public:
    // brightness: -1.0 ~ 1.0, default 0.0
    void setBrightness(float brightness);

protected:
    std::string getShaderName() const override;
    void registerShader() const override;
};

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 2: Create BrightnessFilter.cpp**

```cpp
// native/filter/src/filters/BrightnessFilter.cpp
#include "filter/filters/BrightnessFilter.h"
#include "rhi/RHIShader.h"

namespace fantasy {
namespace filter {

static const char* BRIGHTNESS_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char* BRIGHTNESS_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_brightness;
out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb += u_brightness;
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    fragColor = color;
}
)";

void BrightnessFilter::setBrightness(float brightness) {
    m_param.setFloat("u_brightness", brightness);
}

std::string BrightnessFilter::getShaderName() const {
    return "brightness";
}

void BrightnessFilter::registerShader() const {
    rhi::RHIShader::registerShader("brightness", BRIGHTNESS_VERTEX, BRIGHTNESS_FRAGMENT);
}

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 3: Update CMakeLists.txt**

Replace `native/filter/CMakeLists.txt` with:

```cmake
add_library(filter STATIC
    src/Filter.cpp
    src/filters/BrightnessFilter.cpp
)

target_include_directories(filter PUBLIC include)

target_link_libraries(filter
    rhi
    log
)
```

- [ ] **Step 4: Build to verify**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 5: Commit**

```bash
git add native/filter/include/filter/filters/BrightnessFilter.h \
        native/filter/src/filters/BrightnessFilter.cpp \
        native/filter/CMakeLists.txt
git commit -m "feat(filter): add BrightnessFilter with brightness shader"
```

---

## Task 5: FilterChain — 链式管线

**Files:**
- Create: `native/filter/include/filter/FilterChain.h`
- Create: `native/filter/src/FilterChain.cpp`
- Modify: `native/filter/CMakeLists.txt` — add FilterChain.cpp

FilterChain holds an ordered list of filters and applies them sequentially. Each filter's output becomes the next filter's input (ping-pong pattern handled naturally since each `Filter::apply` creates its own FBO).

- [ ] **Step 1: Create FilterChain.h**

```cpp
// native/filter/include/filter/FilterChain.h
#pragma once

#include "filter/Filter.h"
#include "rhi/RHITexture.h"
#include "rhi/RHIRenderer.h"
#include <vector>
#include <memory>

namespace fantasy {
namespace filter {

class FilterChain {
public:
    void addFilter(std::shared_ptr<Filter> filter);
    void clearFilters();
    int getFilterCount() const;

    // 依次执行所有滤镜，返回最终输出纹理
    // 如果链为空，返回原始输入
    std::shared_ptr<rhi::RHITexture> apply(
        std::shared_ptr<rhi::RHITexture> input,
        std::shared_ptr<rhi::RHIRenderer> renderer);

private:
    std::vector<std::shared_ptr<Filter>> m_filters;
};

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 2: Create FilterChain.cpp**

```cpp
// native/filter/src/FilterChain.cpp
#include "filter/FilterChain.h"
#include "rhi/Log.h"

#define TAG "FilterChain"

namespace fantasy {
namespace filter {

void FilterChain::addFilter(std::shared_ptr<Filter> filter) {
    m_filters.push_back(std::move(filter));
}

void FilterChain::clearFilters() {
    m_filters.clear();
}

int FilterChain::getFilterCount() const {
    return static_cast<int>(m_filters.size());
}

std::shared_ptr<rhi::RHITexture> FilterChain::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    auto current = input;
    for (auto& filter : m_filters) {
        current = filter->apply(current, renderer);
    }
    return current;
}

} // namespace filter
} // namespace fantasy
```

- [ ] **Step 3: Update CMakeLists.txt**

```cmake
add_library(filter STATIC
    src/Filter.cpp
    src/FilterChain.cpp
    src/filters/BrightnessFilter.cpp
)

target_include_directories(filter PUBLIC include)

target_link_libraries(filter
    rhi
    log
)
```

- [ ] **Step 4: Build to verify**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 5: Commit**

```bash
git add native/filter/include/filter/FilterChain.h \
        native/filter/src/FilterChain.cpp \
        native/filter/CMakeLists.txt
git commit -m "feat(filter): add FilterChain for sequential filter execution"
```

---

## Task 6: JNI 测试 — BrightnessFilter 像素验证

**Files:**
- Modify: `native/bridge/src/NativeBridge.cpp` — add `nativeTestBrightnessFilter` and `nativeTestFilterChain`
- Modify: `app/src/main/java/com/fantasy/bridge/NativeBridge.kt` — add external declarations

This is the critical test: create a known-color texture, apply BrightnessFilter, read back pixels, verify the brightness adjustment is correct.

- [ ] **Step 1: Add JNI test functions to NativeBridge.cpp**

Add these includes at the top of NativeBridge.cpp (after existing includes):

```cpp
#include "filter/Filter.h"
#include "filter/FilterChain.h"
#include "filter/FilterParam.h"
#include "filter/filters/BrightnessFilter.h"
```

Add before the closing `} // extern "C"`:

```cpp
JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestBrightnessFilter(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        using namespace fantasy::filter;

        const int width = 2;
        const int height = 2;

        // 输入：每个像素 RGBA = (100, 100, 100, 255)
        std::vector<uint8_t> inputData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            inputData[i * 4 + 0] = 100;
            inputData[i * 4 + 1] = 100;
            inputData[i * 4 + 2] = 100;
            inputData[i * 4 + 3] = 255;
        }
        auto inputTex = RHITexture::create(inputData.data(), width, height, TextureFormat::RGBA8);

        // 创建 BrightnessFilter，设置亮度 +0.2
        auto filter = std::make_shared<BrightnessFilter>();
        filter->setBrightness(0.2f);

        auto renderer = RHIRenderer::create();
        auto outputTex = filter->apply(inputTex, renderer);

        // 读回像素
        std::vector<uint8_t> outputData(width * height * 4);
        outputTex->readPixels(outputData.data());

        // 验证：100/255 + 0.2 = 0.592..., * 255 ≈ 151
        // GPU 精度允许 ±2 误差
        bool passed = true;
        for (int i = 0; i < width * height; i++) {
            uint8_t r = outputData[i * 4 + 0];
            uint8_t g = outputData[i * 4 + 1];
            uint8_t b = outputData[i * 4 + 2];
            uint8_t a = outputData[i * 4 + 3];

            int expectedRGB = 151; // (100/255 + 0.2) * 255
            if (std::abs(r - expectedRGB) > 2 ||
                std::abs(g - expectedRGB) > 2 ||
                std::abs(b - expectedRGB) > 2 ||
                a != 255) {
                FANTASY_LOGE(TAG, "BrightnessFilter pixel %d: got (%d,%d,%d,%d) expected ~(%d,%d,%d,255)",
                    i, r, g, b, a, expectedRGB, expectedRGB, expectedRGB);
                passed = false;
                break;
            }
        }

        RHIShader::clearRegistry();
        FANTASY_LOGI(TAG, "BrightnessFilter test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "BrightnessFilter test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestBrightnessIdentity(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        using namespace fantasy::filter;

        const int width = 2;
        const int height = 2;

        // 输入：每个像素 RGBA = (100, 150, 200, 255)
        std::vector<uint8_t> inputData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            inputData[i * 4 + 0] = 100;
            inputData[i * 4 + 1] = 150;
            inputData[i * 4 + 2] = 200;
            inputData[i * 4 + 3] = 255;
        }
        auto inputTex = RHITexture::create(inputData.data(), width, height, TextureFormat::RGBA8);

        // brightness = 0 应该是恒等变换
        auto filter = std::make_shared<BrightnessFilter>();
        filter->setBrightness(0.0f);

        auto renderer = RHIRenderer::create();
        auto outputTex = filter->apply(inputTex, renderer);

        std::vector<uint8_t> outputData(width * height * 4);
        outputTex->readPixels(outputData.data());

        bool passed = true;
        for (int i = 0; i < width * height * 4; i++) {
            if (std::abs(inputData[i] - outputData[i]) > 1) {
                FANTASY_LOGE(TAG, "BrightnessIdentity mismatch at %d: expected %d got %d",
                    i, inputData[i], outputData[i]);
                passed = false;
                break;
            }
        }

        RHIShader::clearRegistry();
        FANTASY_LOGI(TAG, "BrightnessIdentity test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "BrightnessIdentity test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestFilterChain(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        using namespace fantasy::filter;

        const int width = 2;
        const int height = 2;

        // 输入：每个像素 RGBA = (100, 100, 100, 255)
        std::vector<uint8_t> inputData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            inputData[i * 4 + 0] = 100;
            inputData[i * 4 + 1] = 100;
            inputData[i * 4 + 2] = 100;
            inputData[i * 4 + 3] = 255;
        }
        auto inputTex = RHITexture::create(inputData.data(), width, height, TextureFormat::RGBA8);

        // 链式：两个 BrightnessFilter 各 +0.1
        auto f1 = std::make_shared<BrightnessFilter>();
        f1->setBrightness(0.1f);
        auto f2 = std::make_shared<BrightnessFilter>();
        f2->setBrightness(0.1f);

        FilterChain chain;
        chain.addFilter(f1);
        chain.addFilter(f2);

        auto renderer = RHIRenderer::create();
        auto outputTex = chain.apply(inputTex, renderer);

        // 读回像素
        std::vector<uint8_t> outputData(width * height * 4);
        outputTex->readPixels(outputData.data());

        // 验证：第一次 100/255+0.1 ≈ 0.492 → 125.5 → 126 (GPU取整)
        //       第二次 126/255+0.1 ≈ 0.594 → 151.5 → 152 (GPU取整)
        // 两次链式叠加，允许 ±3 误差（两次 GPU 舍入累积）
        bool passed = true;
        for (int i = 0; i < width * height; i++) {
            uint8_t r = outputData[i * 4 + 0];
            uint8_t g = outputData[i * 4 + 1];
            uint8_t b = outputData[i * 4 + 2];
            uint8_t a = outputData[i * 4 + 3];

            // 预期大约 151-152 范围
            if (r < 148 || r > 155 ||
                g < 148 || g > 155 ||
                b < 148 || b > 155 ||
                a != 255) {
                FANTASY_LOGE(TAG, "FilterChain pixel %d: got (%d,%d,%d,%d) expected ~(151,151,151,255)",
                    i, r, g, b, a);
                passed = false;
                break;
            }
        }

        RHIShader::clearRegistry();
        FANTASY_LOGI(TAG, "FilterChain test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "FilterChain test exception: %s", e.what());
        return JNI_FALSE;
    }
}
```

- [ ] **Step 2: Add Kotlin external declarations**

Add to `NativeBridge.kt` (after `nativeTestPipeline`):

```kotlin
    external fun nativeTestBrightnessFilter(): Boolean
    external fun nativeTestBrightnessIdentity(): Boolean
    external fun nativeTestFilterChain(): Boolean
```

- [ ] **Step 3: Build to verify compilation**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew assembleDebug 2>&1 | tail -20`
Expected: BUILD SUCCESSFUL

- [ ] **Step 4: Commit**

```bash
git add native/bridge/src/NativeBridge.cpp \
        app/src/main/java/com/fantasy/bridge/NativeBridge.kt
git commit -m "feat(filter): add JNI tests for BrightnessFilter and FilterChain"
```

---

## Task 7: Instrumented Test — 在设备上验证

**Files:**
- Modify: `app/src/androidTest/java/com/fantasy/ExampleInstrumentedTest.kt` — add filter tests

Run the filter tests on a real device/emulator to verify GPU rendering correctness.

- [ ] **Step 1: Add instrumented tests**

Replace `app/src/androidTest/java/com/fantasy/ExampleInstrumentedTest.kt` content with:

```kotlin
package com.fantasy

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.fantasy.bridge.NativeBridge
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ExampleInstrumentedTest {

    private val bridge = NativeBridge()

    @Test
    fun useAppContext() {
        val appContext = InstrumentationRegistry.getInstrumentation().targetContext
        assertEquals("com.fantasy", appContext.packageName)
    }

    // --- RHI Tests (Phase 1) ---

    @Test
    fun testTexture() = runBlocking(Dispatchers.Default) {
        assertTrue("Texture test failed", bridge.nativeTestTexture())
    }

    @Test
    fun testShader() = runBlocking(Dispatchers.Default) {
        assertTrue("Shader test failed", bridge.nativeTestShader())
    }

    @Test
    fun testVertexBuffer() = runBlocking(Dispatchers.Default) {
        assertTrue("VertexBuffer test failed", bridge.nativeTestVertexBuffer())
    }

    @Test
    fun testFramebuffer() = runBlocking(Dispatchers.Default) {
        assertTrue("Framebuffer test failed", bridge.nativeTestFramebuffer())
    }

    @Test
    fun testPipeline() = runBlocking(Dispatchers.Default) {
        assertTrue("Pipeline test failed", bridge.nativeTestPipeline())
    }

    // --- Filter Tests (Phase 2) ---

    @Test
    fun testBrightnessFilter() = runBlocking(Dispatchers.Default) {
        assertTrue("BrightnessFilter test failed", bridge.nativeTestBrightnessFilter())
    }

    @Test
    fun testBrightnessIdentity() = runBlocking(Dispatchers.Default) {
        assertTrue("BrightnessIdentity test failed", bridge.nativeTestBrightnessIdentity())
    }

    @Test
    fun testFilterChain() = runBlocking(Dispatchers.Default) {
        assertTrue("FilterChain test failed", bridge.nativeTestFilterChain())
    }
}
```

- [ ] **Step 2: Run tests on device**

Run: `cd /c/Users/14571.FAN/Desktop/GitHubWork/Fantasy && ./gradlew connectedAndroidTest 2>&1 | tail -30`
Expected: All 8 tests PASSED

- [ ] **Step 3: Commit**

```bash
git add app/src/androidTest/java/com/fantasy/ExampleInstrumentedTest.kt
git commit -m "test: add instrumented tests for Phase 2 filter framework"
```

---

## Task 8: 更新项目文档

**Files:**
- Modify: `CLAUDE.md` — update progress
- Modify: `PLAN.md` — mark Phase 2 completed

- [ ] **Step 1: Update CLAUDE.md progress section**

Change `- [ ] Phase 2 — Filter Framework 基础` to `- [x] Phase 2 — Filter Framework 基础（已完成：FilterParam, Filter 基类, FilterChain, BrightnessFilter, 测试通过）`

- [ ] **Step 2: Commit**

```bash
git add CLAUDE.md PLAN.md
git commit -m "docs: mark Phase 2 complete"
```
