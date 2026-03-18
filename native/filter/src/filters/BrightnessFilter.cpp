#include "filter/filters/BrightnessFilter.h"
#include "rhi/RHIShader.h"

namespace fantasy {
namespace filter {

static const char* BRIGHTNESS_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char* BRIGHTNESS_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float u_brightness;
out vec4 fragColor;
void main() {
    vec4 color = texture(uTexture, vTexCoord);
    color.rgb += u_brightness;
    color.rgb = clamp(color.rgb, 0.0, 1.0);
    fragColor = color;
}
)";

void BrightnessFilter::setBrightness(float brightness) {
    m_param.setFloat("u_brightness", brightness);
}

std::string BrightnessFilter::getShaderName() const {
    return "brightness";
}

void BrightnessFilter::registerShader() const {
    rhi::RHIShader::registerShader("brightness", BRIGHTNESS_VERTEX, BRIGHTNESS_FRAGMENT);
}

} // namespace filter
} // namespace fantasy
