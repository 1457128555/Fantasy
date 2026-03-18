#include "GLRenderer.h"
#include "rhi/Log.h"
#include <stdexcept>

#define TAG "GLRenderer"

namespace fantasy {
namespace rhi {

void GLRenderer::beginPass(std::shared_ptr<RHIFramebuffer> target) {
    m_currentTarget = target;
    if (m_currentTarget) {
        m_currentTarget->bind();
    }
}

void GLRenderer::endPass() {
    if (m_currentTarget) {
        m_currentTarget->unbind();
    }
    m_currentTarget = nullptr;
    m_currentShader = nullptr;
    m_currentTexture = nullptr;
    m_currentVBO = nullptr;
}

void GLRenderer::setShader(std::shared_ptr<RHIShader> shader) {
    m_currentShader = shader;
    if (m_currentShader) {
        m_currentShader->bind();
    }
}

void GLRenderer::setTexture(std::shared_ptr<RHITexture> texture, int unit) {
    m_currentTexture = texture;
    if (m_currentTexture) {
        m_currentTexture->bind(unit);
    }
}

void GLRenderer::setVertexBuffer(std::shared_ptr<RHIVertexBuffer> vbo) {
    m_currentVBO = vbo;
    if (m_currentVBO) {
        m_currentVBO->bind();
    }
}

void GLRenderer::setUniforms(const RHIUniformSet& uniforms) {
    if (!m_currentShader) {
        FANTASY_LOGW(TAG, "setUniforms called without a bound shader, uniforms will be dropped");
        return;
    }
    applyUniforms(uniforms);
}

void GLRenderer::drawTriangleStrip(int vertexCount) {
    if (!m_currentShader) {
        FANTASY_LOGE(TAG, "drawTriangleStrip called without a bound shader");
        return;
    }
    if (!m_currentVBO) {
        FANTASY_LOGE(TAG, "drawTriangleStrip called without a bound VBO");
        return;
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
}

void GLRenderer::applyUniforms(const RHIUniformSet& uniforms) {
    for (const auto& [name, value] : uniforms.getAll()) {
        int loc = m_currentShader->getUniformLocation(name);
        if (loc < 0) continue;

        std::visit([loc](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>) {
                glUniform1i(loc, v);
            } else if constexpr (std::is_same_v<T, float>) {
                glUniform1f(loc, v);
            } else if constexpr (std::is_same_v<T, std::array<float, 2>>) {
                glUniform2f(loc, v[0], v[1]);
            } else if constexpr (std::is_same_v<T, std::array<float, 3>>) {
                glUniform3f(loc, v[0], v[1], v[2]);
            } else if constexpr (std::is_same_v<T, std::array<float, 4>>) {
                glUniform4f(loc, v[0], v[1], v[2], v[3]);
            }
        }, value);
    }
}

// 静态工厂
std::shared_ptr<RHIRenderer> RHIRenderer::create() {
    return std::make_shared<GLRenderer>();
}

} // namespace rhi
} // namespace fantasy
