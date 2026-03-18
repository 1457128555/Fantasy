#pragma once

#include "rhi/RHIFramebuffer.h"
#include <GLES3/gl3.h>

namespace fantasy {
namespace rhi {

class GLFramebuffer : public RHIFramebuffer {
public:
    GLFramebuffer(int width, int height, TextureFormat format);
    ~GLFramebuffer() override;

    GLFramebuffer(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;

    void bind() const override;
    void unbind() const override;
    int getWidth() const override { return m_width; }
    int getHeight() const override { return m_height; }
    std::shared_ptr<RHITexture> getColorAttachment() const override { return m_colorAttachment; }

    GLuint getHandle() const { return m_fbo; }

private:
    GLuint m_fbo = 0;
    int m_width = 0;
    int m_height = 0;
    std::shared_ptr<RHITexture> m_colorAttachment;
};

} // namespace rhi
} // namespace fantasy
