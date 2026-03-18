#pragma once

#include "rhi/RHIVertexBuffer.h"
#include <GLES3/gl3.h>

namespace fantasy {
namespace rhi {

class GLVertexBuffer : public RHIVertexBuffer {
public:
    GLVertexBuffer(const float* data, size_t size, const RHIVertexLayout& layout);
    ~GLVertexBuffer() override;

    GLVertexBuffer(const GLVertexBuffer&) = delete;
    GLVertexBuffer& operator=(const GLVertexBuffer&) = delete;

    void bind() const override;
    void unbind() const override;
    const RHIVertexLayout& getLayout() const override { return m_layout; }

private:
    GLuint m_vbo = 0;
    GLuint m_vao = 0;
    RHIVertexLayout m_layout;

    void setupVAO();
};

} // namespace rhi
} // namespace fantasy
