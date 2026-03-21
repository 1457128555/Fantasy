#pragma once

#include "rhi/RHITexture.h"
#include "rhi/RHIRenderer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIUniformSet.h"
#include "rhi/RHIFramebuffer.h"
#include "filter/FilterChain.h"
#include "filter/filters/BrightnessFilter.h"
#include "filter/filters/ContrastFilter.h"
#include "filter/filters/SaturationFilter.h"
#include "filter/filters/LUTFilter.h"
#include "filter/filters/SharpenFilter.h"
#include "filter/filters/BlurFilter.h"
#include "filter/filters/VignetteFilter.h"

#include <memory>
#include <string>
#include <mutex>
#include <vector>

namespace fantasy {
namespace bridge {

class RenderSession {
public:
    RenderSession() = default;
    ~RenderSession() = default;

    void init();
    void setImage(const uint8_t* data, int w, int h);
    void setLUT(const uint8_t* data, int w, int h);
    void setFilterConfig(const std::string& config);
    void drawFrame(int viewW, int viewH);
    std::vector<uint8_t> exportImage(int w, int h);
    void destroy();

    static std::shared_ptr<filter::FilterChain> parseFilterConfig(
        const std::string& config,
        std::shared_ptr<rhi::RHITexture> lutTexture = nullptr);

private:
    void rebuildChain();

    std::shared_ptr<rhi::RHIRenderer> m_renderer;
    std::shared_ptr<rhi::RHIVertexBuffer> m_quadVBO;
    std::shared_ptr<rhi::RHITexture> m_inputTexture;
    std::shared_ptr<rhi::RHITexture> m_lutTexture;

    int m_imageWidth = 0;
    int m_imageHeight = 0;

    std::mutex m_configMutex;
    std::string m_filterConfig;
    bool m_configDirty = false;

    std::shared_ptr<filter::FilterChain> m_chain;
};

} // namespace bridge
} // namespace fantasy
