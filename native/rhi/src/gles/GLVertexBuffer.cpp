#include "GLVertexBuffer.h"
#include "rhi/Log.h"
#include <stdexcept>

#define TAG "GLVertexBuffer"

namespace fantasy {
namespace rhi {

GLVertexBuffer::GLVertexBuffer(const float* data, size_t size, const RHIVertexLayout& layout)
    : m_layout(layout) {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    if (m_vao == 0 || m_vbo == 0) {
        if (m_vbo != 0) glDeleteBuffers(1, &m_vbo);
        if (m_vao != 0) glDeleteVertexArrays(1, &m_vao);
        throw std::runtime_error("Failed to create GL vertex buffer");
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    setupVAO();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    FANTASY_LOGI(TAG, "VertexBuffer created: VAO=%u VBO=%u", m_vao, m_vbo);
}

GLVertexBuffer::~GLVertexBuffer() {
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void GLVertexBuffer::bind() const {
    glBindVertexArray(m_vao);
}

void GLVertexBuffer::unbind() const {
    glBindVertexArray(0);
}

void GLVertexBuffer::setupVAO() {
    const auto& attribs = m_layout.getAttribs();
    GLsizei stride = static_cast<GLsizei>(m_layout.getStride());

    for (const auto& attr : attribs) {
        glEnableVertexAttribArray(attr.location);
        glVertexAttribPointer(
            attr.location,
            VertexAttrib::componentCount(attr.type),
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<const void*>(attr.offset)
        );
    }
}

// 静态工厂
std::shared_ptr<RHIVertexBuffer> RHIVertexBuffer::create(const float* data, size_t size, const RHIVertexLayout& layout) {
    return std::make_shared<GLVertexBuffer>(data, size, layout);
}

} // namespace rhi
} // namespace fantasy
