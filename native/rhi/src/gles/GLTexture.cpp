#include "GLTexture.h"
#include "rhi/Log.h"
#include <stdexcept>

#define TAG "GLTexture"

namespace fantasy {
namespace rhi {

GLTexture::GLTexture(int width, int height, TextureFormat format)
    : m_width(width), m_height(height), m_format(format) {
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid texture dimensions");
    }

    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        throw std::runtime_error("Failed to generate GL texture");
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 toGLInternalFormat(format),
                 width, height, 0,
                 toGLFormat(format),
                 GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLTexture::~GLTexture() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}

void GLTexture::upload(const uint8_t* data, int width, int height, TextureFormat format) {
    if (!data) {
        throw std::runtime_error("Null data passed to GLTexture::upload");
    }
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid dimensions passed to GLTexture::upload");
    }

    m_width = width;
    m_height = height;
    m_format = format;

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0,
                 toGLInternalFormat(format),
                 width, height, 0,
                 toGLFormat(format),
                 GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::readPixels(uint8_t* outData) const {
    if (!outData) {
        throw std::runtime_error("Null output buffer passed to GLTexture::readPixels");
    }

    // 保存当前绑定的 FBO
    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);

    // 创建临时 FBO 读取纹理像素
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
        glDeleteFramebuffers(1, &fbo);
        throw std::runtime_error("Framebuffer incomplete in readPixels");
    }

    glReadPixels(0, 0, m_width, m_height,
                 toGLFormat(m_format), GL_UNSIGNED_BYTE, outData);

    // 恢复之前的 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glDeleteFramebuffers(1, &fbo);
}

void GLTexture::bind(int unit) const {
    m_boundUnit = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void GLTexture::unbind() const {
    glActiveTexture(GL_TEXTURE0 + m_boundUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum GLTexture::toGLFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8: return GL_RGBA;
        case TextureFormat::RGB8:  return GL_RGB;
        default: return GL_RGBA;
    }
}

GLenum GLTexture::toGLInternalFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGB8:  return GL_RGB8;
        default: return GL_RGBA8;
    }
}

int GLTexture::bytesPerPixel(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8: return 4;
        case TextureFormat::RGB8:  return 3;
        default: return 4;
    }
}

// 静态工厂方法
std::shared_ptr<RHITexture> RHITexture::create(int width, int height, TextureFormat format) {
    return std::make_shared<GLTexture>(width, height, format);
}

std::shared_ptr<RHITexture> RHITexture::create(const uint8_t* data, int width, int height, TextureFormat format) {
    auto texture = std::make_shared<GLTexture>(width, height, format);
    texture->upload(data, width, height, format);
    return texture;
}

} // namespace rhi
} // namespace fantasy
