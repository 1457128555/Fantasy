#include <jni.h>
#include <string>
#include <vector>
#include <sstream>
#include <GLES3/gl3.h>
#include "rhi/RHITexture.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIUniformSet.h"
#include "rhi/RHIRenderer.h"
#include "rhi/EGLHelper.h"
#include "rhi/Log.h"

#include "filter/Filter.h"
#include "filter/FilterChain.h"
#include "filter/FilterParam.h"
#include "filter/filters/BrightnessFilter.h"

#define TAG "FantasyBridge"

using namespace fantasy::rhi;

// Helper: parse "brightness:0.2\ncontrast:0.0\n..." into FilterChain
static std::shared_ptr<fantasy::filter::FilterChain> parseFilterConfig(const std::string& config) {
    using namespace fantasy::filter;
    auto chain = std::make_shared<FilterChain>();

    std::istringstream stream(config);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string name = line.substr(0, colonPos);
        float value = std::stof(line.substr(colonPos + 1));

        if (name == "brightness") {
            auto f = std::make_shared<BrightnessFilter>();
            f->setBrightness(value);
            chain->addFilter(f);
        }
        // Phase 4 will add: contrast, saturation
    }
    return chain;
}

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeGetVersion(JNIEnv *env, jobject /* this */) {
    FANTASY_LOGI(TAG, "Fantasy native layer initialized");
    return env->NewStringUTF("Fantasy v0.1.0 - RHI ready");
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestTexture(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;

        const int width = 2;
        const int height = 2;

        uint8_t testData[] = {
            255, 0,   0,   255,
            0,   255, 0,   255,
            0,   0,   255, 255,
            255, 255, 255, 255,
        };

        auto texture = RHITexture::create(testData, width, height, TextureFormat::RGBA8);
        FANTASY_LOGI(TAG, "Texture created: %dx%d", texture->getWidth(), texture->getHeight());

        std::vector<uint8_t> readBack(width * height * 4);
        texture->readPixels(readBack.data());

        bool match = true;
        for (int i = 0; i < width * height * 4; i++) {
            if (testData[i] != readBack[i]) {
                FANTASY_LOGE(TAG, "Pixel mismatch at index %d: expected %d, got %d", i, testData[i], readBack[i]);
                match = false;
                break;
            }
        }

        FANTASY_LOGI(TAG, "Texture test %s", match ? "PASSED" : "FAILED");
        return match ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "Texture test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestShader(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;

        // 每个测试有独立的 EGL 上下文，需要清除旧的 shader 注册
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        auto shader = RHIShader::get("passthrough");
        shader->bind();

        int texLoc = shader->getUniformLocation("uTexture");
        FANTASY_LOGI(TAG, "Shader test: uTexture location = %d", texLoc);

        shader->unbind();

        // 清除注册表，避免 EGL 上下文销毁后持有 stale handle
        RHIShader::clearRegistry();

        bool passed = (texLoc >= 0);
        FANTASY_LOGI(TAG, "Shader test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "Shader test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestVertexBuffer(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;

        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };

        RHIVertexLayout layout;
        layout.add("aPosition", VertexAttribType::Float2, 0)
              .add("aTexCoord", VertexAttribType::Float2, 1);

        auto vbo = RHIVertexBuffer::create(quadVertices, sizeof(quadVertices), layout);

        vbo->bind();
        vbo->unbind();

        bool passed = (vbo != nullptr && layout.getStride() == 16 && layout.getAttribs().size() == 2);
        FANTASY_LOGI(TAG, "VertexBuffer test %s: stride=%zu attribs=%zu",
                     passed ? "PASSED" : "FAILED", layout.getStride(), layout.getAttribs().size());
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "VertexBuffer test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestFramebuffer(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;

        const int width = 64;
        const int height = 64;

        auto fbo = RHIFramebuffer::create(width, height, TextureFormat::RGBA8);

        fbo->bind();
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto colorTex = fbo->getColorAttachment();
        std::vector<uint8_t> pixels(width * height * 4);
        colorTex->readPixels(pixels.data());

        fbo->unbind();

        bool passed = (pixels[0] == 255 && pixels[1] == 0 && pixels[2] == 0 && pixels[3] == 255);
        FANTASY_LOGI(TAG, "Framebuffer test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "Framebuffer test exception: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestPipeline(JNIEnv *env, jobject /* this */) {
    try {
        EGLHelper egl;

        const int width = 2;
        const int height = 2;

        // 1. 创建输入纹理：纯绿色
        std::vector<uint8_t> inputData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            inputData[i * 4 + 0] = 0;
            inputData[i * 4 + 1] = 200;
            inputData[i * 4 + 2] = 0;
            inputData[i * 4 + 3] = 255;
        }
        auto inputTex = RHITexture::create(inputData.data(), width, height, TextureFormat::RGBA8);

        // 2. 创建输出 FBO
        auto fbo = RHIFramebuffer::create(width, height, TextureFormat::RGBA8);

        // 3. 创建全屏四边形
        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };
        RHIVertexLayout layout;
        layout.add("aPosition", VertexAttribType::Float2, 0)
              .add("aTexCoord", VertexAttribType::Float2, 1);
        auto vbo = RHIVertexBuffer::create(quadVertices, sizeof(quadVertices), layout);

        // 4. shader（新 EGL 上下文，需要重新创建）
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();
        auto shader = RHIShader::get("passthrough");

        // 5. uniform
        RHIUniformSet uniforms;
        uniforms.setInt("uTexture", 0);

        // 6. 渲染
        auto renderer = RHIRenderer::create();
        renderer->beginPass(fbo);
        renderer->setShader(shader);
        renderer->setTexture(inputTex, 0);
        renderer->setVertexBuffer(vbo);
        renderer->setUniforms(uniforms);
        renderer->drawTriangleStrip(4);
        renderer->endPass();

        // 7. 读回输出像素
        auto outputTex = fbo->getColorAttachment();
        std::vector<uint8_t> outputData(width * height * 4);
        outputTex->readPixels(outputData.data());

        // 8. 验证输出 == 输入
        bool passed = true;
        for (int i = 0; i < width * height * 4; i++) {
            if (inputData[i] != outputData[i]) {
                FANTASY_LOGE(TAG, "Pipeline mismatch at %d: expected %d got %d", i, inputData[i], outputData[i]);
                passed = false;
                break;
            }
        }

        // 清除注册表
        RHIShader::clearRegistry();

        FANTASY_LOGI(TAG, "Pipeline test %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "Pipeline test exception: %s", e.what());
        return JNI_FALSE;
    }
}

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

        // 验证：两次 +0.1 叠加，允许 ±3 误差（两次 GPU 舍入累积）
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

JNIEXPORT jbyteArray JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeApplyFilters(
    JNIEnv *env, jobject /* this */,
    jbyteArray imageData, jint width, jint height, jstring filterConfig) {
    try {
        // 1. Copy input pixels
        jsize len = env->GetArrayLength(imageData);
        std::vector<uint8_t> inputPixels(len);
        env->GetByteArrayRegion(imageData, 0, len, reinterpret_cast<jbyte*>(inputPixels.data()));

        // 2. Parse filter config
        const char* configStr = env->GetStringUTFChars(filterConfig, nullptr);
        std::string config(configStr);
        env->ReleaseStringUTFChars(filterConfig, configStr);

        auto chain = parseFilterConfig(config);

        // 3. Create EGL context + init shaders
        EGLHelper egl;
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        // 4. Create input texture
        auto inputTex = RHITexture::create(inputPixels.data(), width, height, TextureFormat::RGBA8);

        // 5. Apply filter chain
        auto renderer = RHIRenderer::create();
        auto outputTex = chain->apply(inputTex, renderer);

        // 6. Read back pixels
        std::vector<uint8_t> outputPixels(width * height * 4);
        outputTex->readPixels(outputPixels.data());

        // 7. Cleanup shaders before EGL context is destroyed
        RHIShader::clearRegistry();

        // 8. Return result byte[]
        jbyteArray result = env->NewByteArray(outputPixels.size());
        env->SetByteArrayRegion(result, 0, outputPixels.size(),
                                reinterpret_cast<const jbyte*>(outputPixels.data()));
        return result;

    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "nativeApplyFilters exception: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_fantasy_bridge_NativeBridge_nativeTestApplyFilters(JNIEnv *env, jobject thiz) {
    try {
        EGLHelper egl;
        RHIShader::clearRegistry();
        RHIShader::registerBuiltins();

        using namespace fantasy::filter;

        const int width = 2;
        const int height = 2;

        // Input: RGBA (100, 100, 100, 255)
        std::vector<uint8_t> inputData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            inputData[i * 4 + 0] = 100;
            inputData[i * 4 + 1] = 100;
            inputData[i * 4 + 2] = 100;
            inputData[i * 4 + 3] = 255;
        }
        auto inputTex = RHITexture::create(inputData.data(), width, height, TextureFormat::RGBA8);

        // Build chain via parseFilterConfig
        auto chain = parseFilterConfig("brightness:0.2");

        auto renderer = RHIRenderer::create();
        auto outputTex = chain->apply(inputTex, renderer);

        std::vector<uint8_t> output(width * height * 4);
        outputTex->readPixels(output.data());

        // Verify: (100/255 + 0.2) * 255 ≈ 151, allow ±2
        bool passed = true;
        for (int i = 0; i < width * height; i++) {
            uint8_t r = output[i * 4 + 0];
            uint8_t g = output[i * 4 + 1];
            uint8_t b = output[i * 4 + 2];
            uint8_t a = output[i * 4 + 3];
            int expected = 151;
            if (std::abs(r - expected) > 2 || std::abs(g - expected) > 2 ||
                std::abs(b - expected) > 2 || a != 255) {
                FANTASY_LOGE(TAG, "nativeTestApplyFilters pixel %d: (%d,%d,%d,%d) expected ~(%d,%d,%d,255)",
                    i, r, g, b, a, expected, expected, expected);
                passed = false;
                break;
            }
        }

        RHIShader::clearRegistry();
        FANTASY_LOGI(TAG, "nativeTestApplyFilters %s", passed ? "PASSED" : "FAILED");
        return passed ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        FANTASY_LOGE(TAG, "nativeTestApplyFilters exception: %s", e.what());
        return JNI_FALSE;
    }
}

} // extern "C"
