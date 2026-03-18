#pragma once

#include "rhi/RHITexture.h"
#include <GLES3/gl3.h>

namespace fantasy {
namespace rhi {

class GLTexture : public RHITexture {
public:
    GLTexture(int width, int height, TextureFormat format);
    ~GLTexture() override;

    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;

    int getWidth() const override { return m_width; }
    int getHeight() const override { return m_height; }
    TextureFormat getFormat() const override { return m_format; }

    void upload(const uint8_t* data, int width, int height, TextureFormat format) override;
    void readPixels(uint8_t* outData) const override;
    void bind(int unit = 0) const override;
    void unbind() const override;

    GLuint getHandle() const { return m_textureId; }

private:
    GLuint m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
    mutable int m_boundUnit = 0;
    TextureFormat m_format = TextureFormat::RGBA8;

    static GLenum toGLFormat(TextureFormat format);
    static GLenum toGLInternalFormat(TextureFormat format);
    static int bytesPerPixel(TextureFormat format);
};

} // namespace rhi
} // namespace fantasy
