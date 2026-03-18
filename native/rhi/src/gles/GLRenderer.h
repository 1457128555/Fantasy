#pragma once

#include "rhi/RHIRenderer.h"
#include <GLES3/gl3.h>

namespace fantasy {
namespace rhi {

class GLRenderer : public RHIRenderer {
public:
    GLRenderer() = default;
    ~GLRenderer() override = default;

    void beginPass(std::shared_ptr<RHIFramebuffer> target) override;
    void endPass() override;

    void setShader(std::shared_ptr<RHIShader> shader) override;
    void setTexture(std::shared_ptr<RHITexture> texture, int unit = 0) override;
    void setVertexBuffer(std::shared_ptr<RHIVertexBuffer> vbo) override;
    void setUniforms(const RHIUniformSet& uniforms) override;

    void drawTriangleStrip(int vertexCount) override;

private:
    std::shared_ptr<RHIFramebuffer> m_currentTarget;
    std::shared_ptr<RHIShader> m_currentShader;
    std::shared_ptr<RHITexture> m_currentTexture;
    std::shared_ptr<RHIVertexBuffer> m_currentVBO;

    void applyUniforms(const RHIUniformSet& uniforms);
};

} // namespace rhi
} // namespace fantasy
