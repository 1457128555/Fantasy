#include "GLFramebuffer.h"
#include "GLTexture.h"
#include "rhi/Log.h"
#include <stdexcept>

#define TAG "GLFramebuffer"

namespace fantasy {
namespace rhi {

GLFramebuffer::GLFramebuffer(int width, int height, TextureFormat format)
    : m_width(width), m_height(height) {

    m_colorAttachment = RHITexture::create(width, height, format);

    glGenFramebuffers(1, &m_fbo);
    if (m_fbo == 0) {
        throw std::runtime_error("Failed to create GL framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    auto glTex = std::dynamic_pointer_cast<GLTexture>(m_colorAttachment);
    if (!glTex) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &m_fbo);
        throw std::runtime_error("Color attachment is not a GLTexture");
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, glTex->getHandle(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
        throw std::runtime_error("Framebuffer is not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    FANTASY_LOGI(TAG, "Framebuffer created: %dx%d FBO=%u", width, height, m_fbo);
}

GLFramebuffer::~GLFramebuffer() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

void GLFramebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void GLFramebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 静态工厂
std::shared_ptr<RHIFramebuffer> RHIFramebuffer::create(int width, int height, TextureFormat format) {
    return std::make_shared<GLFramebuffer>(width, height, format);
}

} // namespace rhi
} // namespace fantasy
