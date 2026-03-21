#include "filter/filters/BlurFilter.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIUniformSet.h"

namespace fantasy {
namespace filter {

static const char* BLUR_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

// Box blur with large sampling radius for visible effect
static const char* BLUR_FRAGMENT = R"(#version 300 es
precision highp float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_blurRadius;
uniform vec2 u_texelSize;
out vec4 fragColor;
void main() {
    // radius in pixels: 0 ~ 20
    float radius = u_blurRadius * 20.0;

    vec4 sum = vec4(0.0);
    float count = 0.0;

    // 5x5 sampling grid scaled by radius
    for (float x = -2.0; x <= 2.0; x += 1.0) {
        for (float y = -2.0; y <= 2.0; y += 1.0) {
            vec2 offset = vec2(x, y) * u_texelSize * radius * 0.5;
            sum += texture(uTexture, vTexCoord + offset);
            count += 1.0;
        }
    }

    fragColor = sum / count;
}
)";

void BlurFilter::setBlurRadius(float radius) {
    m_param.setFloat("u_blurRadius", radius);
}

std::string BlurFilter::getShaderName() const {
    return "blur";
}

void BlurFilter::registerShader() const {
    rhi::RHIShader::registerShader("blur", BLUR_VERTEX, BLUR_FRAGMENT);
}

std::shared_ptr<rhi::RHITexture> BlurFilter::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    registerShader();

    int w = input->getWidth();
    int h = input->getHeight();

    auto fbo = rhi::RHIFramebuffer::create(w, h, input->getFormat());

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

    auto shader = rhi::RHIShader::get(getShaderName());

    auto uniforms = buildUniforms();
    uniforms.setInt("uTexture", 0);
    uniforms.setFloat2("u_texelSize", 1.0f / w, 1.0f / h);

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
