#include "bridge/RenderSession.h"
#include "rhi/Log.h"
#include <sstream>
#include <GLES3/gl3.h>

#define TAG "RenderSession"

namespace fantasy {
namespace bridge {

void RenderSession::init() {
    rhi::RHIShader::clearRegistry();
    rhi::RHIShader::registerBuiltins();

    m_renderer = rhi::RHIRenderer::create();

    // texcoord Y 翻转：修正 Bitmap(top-first) 与 GL(bottom-first) 行序差异
    float quadVertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
    };
    rhi::RHIVertexLayout layout;
    layout.add("aPosition", rhi::VertexAttribType::Float2, 0)
          .add("aTexCoord", rhi::VertexAttribType::Float2, 1);
    m_quadVBO = rhi::RHIVertexBuffer::create(quadVertices, sizeof(quadVertices), layout);

    m_chain = std::make_shared<filter::FilterChain>();

    FANTASY_LOGI(TAG, "RenderSession initialized");
}

void RenderSession::setImage(const uint8_t* data, int w, int h) {
    m_inputTexture = rhi::RHITexture::create(data, w, h, rhi::TextureFormat::RGBA8);
    m_imageWidth = w;
    m_imageHeight = h;
    FANTASY_LOGI(TAG, "Image uploaded: %dx%d", w, h);
}

void RenderSession::setLUT(const uint8_t* data, int w, int h) {
    m_lutTexture = rhi::RHITexture::create(data, w, h, rhi::TextureFormat::RGBA8);
    FANTASY_LOGI(TAG, "LUT uploaded: %dx%d", w, h);
}

void RenderSession::setFilterConfig(const std::string& config) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    m_filterConfig = config;
    m_configDirty = true;
}

void RenderSession::rebuildChain() {
    std::string config;
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (!m_configDirty) return;
        config = m_filterConfig;
        m_configDirty = false;
    }
    m_chain = parseFilterConfig(config, m_lutTexture);
}

void RenderSession::drawFrame(int viewW, int viewH) {
    if (!m_inputTexture || !m_renderer) return;

    rebuildChain();

    // Apply filter chain -> output texture
    auto outputTex = m_chain->apply(m_inputTexture, m_renderer);

    // Compute aspect-ratio letterbox viewport (use output texture size, may differ after crop)
    float imageAspect = static_cast<float>(outputTex->getWidth()) / outputTex->getHeight();
    float viewAspect = static_cast<float>(viewW) / viewH;

    int vpX, vpY, vpW, vpH;
    if (imageAspect > viewAspect) {
        // Image wider than view: pillarbox top/bottom
        vpW = viewW;
        vpH = static_cast<int>(viewW / imageAspect);
        vpX = 0;
        vpY = (viewH - vpH) / 2;
    } else {
        // Image taller than view: letterbox left/right
        vpH = viewH;
        vpW = static_cast<int>(viewH * imageAspect);
        vpX = (viewW - vpW) / 2;
        vpY = 0;
    }

    // Clear entire screen to black
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewW, viewH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw result texture to screen with correct viewport
    glViewport(vpX, vpY, vpW, vpH);

    auto shader = rhi::RHIShader::get("passthrough");
    shader->bind();

    rhi::RHIUniformSet uniforms;
    uniforms.setInt("uTexture", 0);

    outputTex->bind(0);
    m_quadVBO->bind();

    m_renderer->setShader(shader);
    m_renderer->setTexture(outputTex, 0);
    m_renderer->setVertexBuffer(m_quadVBO);
    m_renderer->setUniforms(uniforms);
    m_renderer->drawTriangleStrip(4);

    // Reset viewport
    glViewport(0, 0, viewW, viewH);
}

std::vector<uint8_t> RenderSession::exportImage(int w, int h) {
    if (!m_inputTexture || !m_renderer) {
        return {};
    }

    rebuildChain();

    auto outputTex = m_chain->apply(m_inputTexture, m_renderer);

    // Use actual output texture dimensions (may differ from input after crop/rotate)
    int outW = outputTex->getWidth();
    int outH = outputTex->getHeight();
    std::vector<uint8_t> pixels(outW * outH * 4);
    outputTex->readPixels(pixels.data());
    return pixels;
}

void RenderSession::destroy() {
    m_chain.reset();
    m_lutTexture.reset();
    m_inputTexture.reset();
    m_quadVBO.reset();
    m_renderer.reset();
    rhi::RHIShader::clearRegistry();
    FANTASY_LOGI(TAG, "RenderSession destroyed");
}

std::shared_ptr<filter::FilterChain> RenderSession::parseFilterConfig(
    const std::string& config, std::shared_ptr<rhi::RHITexture> lutTexture) {
    auto chain = std::make_shared<filter::FilterChain>();

    // First pass: collect crop/rotation params
    float cropX = 0.0f, cropY = 0.0f, cropW = 1.0f, cropH = 1.0f;
    int rotation90 = 0;
    float freeRotation = 0.0f;

    std::istringstream stream(config);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string name = line.substr(0, colonPos);
        float value = std::stof(line.substr(colonPos + 1));

        if (name == "crop_x") cropX = value;
        else if (name == "crop_y") cropY = value;
        else if (name == "crop_w") cropW = value;
        else if (name == "crop_h") cropH = value;
        else if (name == "rotation90") rotation90 = static_cast<int>(value);
        else if (name == "free_rotation") freeRotation = value;
    }

    // Insert CropRotateFilter at head if any transform is active
    bool hasCrop = (cropX != 0.0f || cropY != 0.0f || cropW != 1.0f || cropH != 1.0f);
    bool hasRotation = (rotation90 != 0 || freeRotation != 0.0f);
    if (hasCrop || hasRotation) {
        auto cropFilter = std::make_shared<filter::CropRotateFilter>();
        cropFilter->setCropRect(cropX, cropY, cropW, cropH);
        cropFilter->setRotation90(rotation90);
        cropFilter->setFreeRotation(freeRotation);
        chain->addFilter(cropFilter);
    }

    // Second pass: other filters
    std::istringstream stream2(config);
    while (std::getline(stream2, line)) {
        if (line.empty()) continue;
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string name = line.substr(0, colonPos);
        float value = std::stof(line.substr(colonPos + 1));

        if (name == "lut_strength" && lutTexture) {
            auto f = std::make_shared<filter::LUTFilter>();
            f->setLUT(lutTexture);
            f->setStrength(value);
            chain->addFilter(f);
        } else if (name == "brightness") {
            auto f = std::make_shared<filter::BrightnessFilter>();
            f->setBrightness(value);
            chain->addFilter(f);
        } else if (name == "contrast") {
            auto f = std::make_shared<filter::ContrastFilter>();
            f->setContrast(value);
            chain->addFilter(f);
        } else if (name == "saturation") {
            auto f = std::make_shared<filter::SaturationFilter>();
            f->setSaturation(value);
            chain->addFilter(f);
        } else if (name == "sharpness") {
            auto f = std::make_shared<filter::SharpenFilter>();
            f->setSharpness(value);
            chain->addFilter(f);
        } else if (name == "blur") {
            auto f = std::make_shared<filter::BlurFilter>();
            f->setBlurRadius(value);
            chain->addFilter(f);
        } else if (name == "vignette") {
            auto f = std::make_shared<filter::VignetteFilter>();
            f->setVignette(value);
            chain->addFilter(f);
        }
    }
    return chain;
}

} // namespace bridge
} // namespace fantasy
