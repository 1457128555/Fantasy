#include "filter/filters/CropRotateFilter.h"
#include "rhi/RHIShader.h"
#include "rhi/RHIFramebuffer.h"
#include "rhi/RHIVertexBuffer.h"
#include "rhi/RHIVertexLayout.h"
#include "rhi/RHIUniformSet.h"
#include <cmath>

namespace fantasy {
namespace filter {

static const char* CROP_ROTATE_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

// Fragment shader: inverse transform from output UV to source texture UV
// Transform order (forward): 90° rotate → free rotate → crop
// Inverse (in shader):  crop⁻¹ → free rotate⁻¹ → 90° rotate⁻¹
static const char* CROP_ROTATE_FRAGMENT = R"(#version 300 es
precision highp float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uCropRect;      // x, y, w, h in normalized coords (after 90° rotation space)
uniform float uFreeRotation;  // radians
uniform int uRotation90;      // 0,1,2,3
out vec4 fragColor;

void main() {
    // Step 1: Map output UV [0,1] to crop region
    vec2 uv = uCropRect.xy + vTexCoord * uCropRect.zw;

    // Step 2: Inverse free rotation around center (0.5, 0.5)
    if (uFreeRotation != 0.0) {
        vec2 center = vec2(0.5);
        uv -= center;
        float c = cos(-uFreeRotation);
        float s = sin(-uFreeRotation);
        uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
        uv += center;
    }

    // Step 3: Inverse 90° rotation
    // Forward 90°CW: (x,y) -> (1-y, x), so inverse: (x,y) -> (y, 1-x)
    if (uRotation90 == 1) {
        uv = vec2(uv.y, 1.0 - uv.x);
    } else if (uRotation90 == 2) {
        uv = vec2(1.0 - uv.x, 1.0 - uv.y);
    } else if (uRotation90 == 3) {
        uv = vec2(1.0 - uv.y, uv.x);
    }

    // Out-of-bounds: black
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        fragColor = texture(uTexture, uv);
    }
}
)";

void CropRotateFilter::setCropRect(float x, float y, float w, float h) {
    m_cropX = x;
    m_cropY = y;
    m_cropW = w;
    m_cropH = h;
}

void CropRotateFilter::setRotation90(int steps) {
    m_rotation90 = steps % 4;
}

void CropRotateFilter::setFreeRotation(float radians) {
    m_freeRotation = radians;
}

std::string CropRotateFilter::getShaderName() const {
    return "crop_rotate";
}

void CropRotateFilter::registerShader() const {
    rhi::RHIShader::registerShader("crop_rotate", CROP_ROTATE_VERTEX, CROP_ROTATE_FRAGMENT);
}

std::shared_ptr<rhi::RHITexture> CropRotateFilter::apply(
    std::shared_ptr<rhi::RHITexture> input,
    std::shared_ptr<rhi::RHIRenderer> renderer) {

    registerShader();

    int srcW = input->getWidth();
    int srcH = input->getHeight();

    // After 90° rotation, the effective dimensions swap
    int effectiveW = srcW;
    int effectiveH = srcH;
    if (m_rotation90 == 1 || m_rotation90 == 3) {
        effectiveW = srcH;
        effectiveH = srcW;
    }

    // Output dimensions = cropped region of the rotated image
    int outW = static_cast<int>(effectiveW * m_cropW);
    int outH = static_cast<int>(effectiveH * m_cropH);
    if (outW < 1) outW = 1;
    if (outH < 1) outH = 1;

    auto fbo = rhi::RHIFramebuffer::create(outW, outH, input->getFormat());

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

    rhi::RHIUniformSet uniforms;
    uniforms.setInt("uTexture", 0);
    uniforms.setFloat4("uCropRect", m_cropX, m_cropY, m_cropW, m_cropH);
    uniforms.setFloat("uFreeRotation", m_freeRotation);
    uniforms.setInt("uRotation90", m_rotation90);

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
