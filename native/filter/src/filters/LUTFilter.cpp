#include "filter/filters/LUTFilter.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIUniformSet.h"

namespace fantasy {
namespace filter {

static const char* LUT_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char* LUT_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform sampler2D uLUT;
uniform float u_lut_strength;
out vec4 fragColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    float blueColor = color.b * 63.0;

    vec2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);

    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);

    vec2 texPos1;
    texPos1.x = (quad1.x * 64.0 + color.r * 63.0 + 0.5) / 512.0;
    texPos1.y = (quad1.y * 64.0 + color.g * 63.0 + 0.5) / 512.0;

    vec2 texPos2;
    texPos2.x = (quad2.x * 64.0 + color.r * 63.0 + 0.5) / 512.0;
    texPos2.y = (quad2.y * 64.0 + color.g * 63.0 + 0.5) / 512.0;

    vec4 newColor1 = texture(uLUT, texPos1);
    vec4 newColor2 = texture(uLUT, texPos2);

    vec4 lutColor = mix(newColor1, newColor2, fract(blueColor));

    fragColor = vec4(mix(color.rgb, lutColor.rgb, u_lut_strength), color.a);
}
)";

void LUTFilter::setLUT(std::shared_ptr<rhi::RHITexture> lutTexture) {
    m_lutTexture = lutTexture;
}

void LUTFilter::setStrength(float strength) {
    m_param.setFloat("u_lut_strength", strength);
}

std::string LUTFilter::getShaderName() const {
    return "lut";
}

void LUTFilter::registerShader() const {
    rhi::RHIShader::registerShader("lut", LUT_VERTEX, LUT_FRAGMENT);
}

std::shared_ptr<rhi::RHITexture> LUTFilter::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    if (!m_lutTexture) {
        return input;
    }

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
    uniforms.setInt("uLUT", 1);

    renderer->beginPass(fbo);
    renderer->setShader(shader);
    renderer->setTexture(input, 0);
    renderer->setTexture(m_lutTexture, 1);
    renderer->setVertexBuffer(vbo);
    renderer->setUniforms(uniforms);
    renderer->drawTriangleStrip(4);
    renderer->endPass();

    return fbo->getColorAttachment();
}

} // namespace filter
} // namespace fantasy
