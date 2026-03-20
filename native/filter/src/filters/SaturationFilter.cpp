#include "filter/filters/SaturationFilter.h"
#include "rhi/RHIShader.h"

namespace fantasy {
namespace filter {

static const char* SATURATION_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

// saturation 范围 [-1, 1]，0 不变，-1 完全灰度，+1 饱和度加倍
// 使用 luminance 权重做灰度混合
static const char* SATURATION_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_saturation;
out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, vTexCoord);
    float luminance = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 gray = vec3(luminance);
    // 将 [-1,1] 映射为混合系数 [0, 2]：0=灰度，1=原色，2=超饱和
    float factor = u_saturation + 1.0;
    color.rgb = mix(gray, color.rgb, factor);
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    fragColor = color;
}
)";

void SaturationFilter::setSaturation(float saturation) {
    m_param.setFloat("u_saturation", saturation);
}

std::string SaturationFilter::getShaderName() const {
    return "saturation";
}

void SaturationFilter::registerShader() const {
    rhi::RHIShader::registerShader("saturation", SATURATION_VERTEX, SATURATION_FRAGMENT);
}

} // namespace filter
} // namespace fantasy
