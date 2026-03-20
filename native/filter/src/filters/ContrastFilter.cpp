#include "filter/filters/ContrastFilter.h"
#include "rhi/RHIShader.h"

namespace fantasy {
namespace filter {

static const char* CONTRAST_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

// contrast 范围 [-1, 1]，映射到乘数：0 表示不变，+1 加倍对比度，-1 变为纯灰
static const char* CONTRAST_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_contrast;
out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, vTexCoord);
    // 将 [-1,1] 映射为乘数 [0, 2]
    float factor = u_contrast + 1.0;
    color.rgb = (color.rgb - 0.5) * factor + 0.5;
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    fragColor = color;
}
)";

void ContrastFilter::setContrast(float contrast) {
    m_param.setFloat("u_contrast", contrast);
}

std::string ContrastFilter::getShaderName() const {
    return "contrast";
}

void ContrastFilter::registerShader() const {
    rhi::RHIShader::registerShader("contrast", CONTRAST_VERTEX, CONTRAST_FRAGMENT);
}

} // namespace filter
} // namespace fantasy
