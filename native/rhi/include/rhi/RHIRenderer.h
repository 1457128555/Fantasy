#pragma once

#include "rhi/RHIShader.h"
#include "rhi/RHITexture.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIUniformSet.h"
#include <memory>

namespace fantasy {
namespace rhi {

class RHIRenderer {
public:
    virtual ~RHIRenderer() = default;

    virtual void beginPass(std::shared_ptr<RHIFramebuffer> target) = 0;
    virtual void endPass() = 0;

    virtual void setShader(std::shared_ptr<RHIShader> shader) = 0;
    virtual void setTexture(std::shared_ptr<RHITexture> texture, int unit = 0) = 0;
    virtual void setVertexBuffer(std::shared_ptr<RHIVertexBuffer> vbo) = 0;
    virtual void setUniforms(const RHIUniformSet& uniforms) = 0;

    virtual void drawTriangleStrip(int vertexCount) = 0;

    static std::shared_ptr<RHIRenderer> create();
};

} // namespace rhi
} // namespace fantasy
