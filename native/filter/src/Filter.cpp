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
