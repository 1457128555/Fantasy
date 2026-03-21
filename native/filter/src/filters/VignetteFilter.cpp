#include "filter/filters/VignetteFilter.h"
#include "rhi/RHIShader.h"

namespace fantasy {
namespace filter {

static const char* VIGNETTE_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char* VIGNETTE_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_vignette;
out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Distance from center (0,0 at center, ~0.707 at corners)
    vec2 uv = vTexCoord - 0.5;
    float dist = length(uv);

    // Smooth vignette falloff
    float vignette = smoothstep(0.8, 0.8 - u_vignette * 0.75, dist);

    color.rgb *= vignette;
    fragColor = color;
}
)";

void VignetteFilter::setVignette(float vignette) {
    m_param.setFloat("u_vignette", vignette);
}

std::string VignetteFilter::getShaderName() const {
    return "vignette";
}

void VignetteFilter::registerShader() const {
    rhi::RHIShader::registerShader("vignette", VIGNETTE_VERTEX, VIGNETTE_FRAGMENT);
}

} // namespace filter
} // namespace fantasy
