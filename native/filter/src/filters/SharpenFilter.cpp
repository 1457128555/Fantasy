#include "filter/filters/SharpenFilter.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIUniformSet.h"

namespace fantasy {
namespace filter {

static const char* SHARPEN_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

// Laplacian sharpening: stronger and more perceptible
static const char* SHARPEN_FRAGMENT = R"(#version 300 es
precision highp float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_sharpness;
uniform vec2 u_texelSize;
out vec4 fragColor;
void main() {
    vec4 center = texture(uTexture, vTexCoord);

    // 8-neighbor Laplacian kernel for stronger edge detection
    vec4 n0 = texture(uTexture, vTexCoord + vec2(-u_texelSize.x, -u_texelSize.y));
    vec4 n1 = texture(uTexture, vTexCoord + vec2( 0.0,           -u_texelSize.y));
    vec4 n2 = texture(uTexture, vTexCoord + vec2( u_texelSize.x, -u_texelSize.y));
    vec4 n3 = texture(uTexture, vTexCoord + vec2(-u_texelSize.x,  0.0));
    vec4 n4 = texture(uTexture, vTexCoord + vec2( u_texelSize.x,  0.0));
    vec4 n5 = texture(uTexture, vTexCoord + vec2(-u_texelSize.x,  u_texelSize.y));
    vec4 n6 = texture(uTexture, vTexCoord + vec2( 0.0,            u_texelSize.y));
    vec4 n7 = texture(uTexture, vTexCoord + vec2( u_texelSize.x,  u_texelSize.y));

    vec4 laplacian = center * 8.0 - (n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7);

    // strength maps 0~1 to multiplier 0~3
    vec4 sharpened = center + laplacian * u_sharpness * 3.0;
    sharpened.rgb = clamp(sharpened.rgb, 0.0, 1.0);
    sharpened.a = center.a;
    fragColor = sharpened;
}
)";

void SharpenFilter::setSharpness(float sharpness) {
    m_param.setFloat("u_sharpness", sharpness);
}

std::string SharpenFilter::getShaderName() const {
    return "sharpen";
}

void SharpenFilter::registerShader() const {
    rhi::RHIShader::registerShader("sharpen", SHARPEN_VERTEX, SHARPEN_FRAGMENT);
}

std::shared_ptr<rhi::RHITexture> SharpenFilter::apply(
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
