#pragma once

#include "rhi/RHIShader.h"
#include <GLES3/gl3.h>

namespace fantasy {
namespace rhi {

class GLShader : public RHIShader {
public:
    GLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~GLShader() override;

    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;

    void bind() const override;
    void unbind() const override;

    int getUniformLocation(const std::string& name) const override;

    GLuint getHandle() const { return m_program; }

private:
    GLuint m_program = 0;

    static GLuint compileShader(GLenum type, const std::string& source);
};

} // namespace rhi
} // namespace fantasy
