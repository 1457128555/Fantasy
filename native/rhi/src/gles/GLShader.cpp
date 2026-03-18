#include "GLShader.h"
#include "rhi/Log.h"
#include <stdexcept>
#include <vector>

#define TAG "GLShader"

namespace fantasy {
namespace rhi {

// ==================== 内置 Shader 源码 ====================

static const char* PASSTHROUGH_VERTEX = R"(#version 300 es
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char* PASSTHROUGH_FRAGMENT = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    fragColor = texture(uTexture, vTexCoord);
}
)";

// ==================== GLShader 实现 ====================

GLShader::GLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    m_program = glCreateProgram();
    if (m_program == 0) {
        glDeleteShader(vs);
        glDeleteShader(fs);
        throw std::runtime_error("Failed to create GL program");
    }

    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint linkStatus = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        GLint logLen = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
        FANTASY_LOGE(TAG, "Program link failed: %s", log.data());

        glDeleteProgram(m_program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        m_program = 0;
        throw std::runtime_error("Shader program link failed: " + std::string(log.data()));
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    FANTASY_LOGI(TAG, "Shader program created: %u", m_program);
}

GLShader::~GLShader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void GLShader::bind() const {
    glUseProgram(m_program);
}

void GLShader::unbind() const {
    glUseProgram(0);
}

int GLShader::getUniformLocation(const std::string& name) const {
    int loc = glGetUniformLocation(m_program, name.c_str());
    return loc;
}

GLuint GLShader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error("Failed to create shader");
    }

    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compileStatus = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        FANTASY_LOGE(TAG, "Shader compile failed: %s", log.data());

        glDeleteShader(shader);
        throw std::runtime_error("Shader compile failed: " + std::string(log.data()));
    }

    return shader;
}

// ==================== 静态注册表 ====================
// 注意：注册表是全局静态的，切换 EGL 上下文时必须先调用 clearRegistry()
// 再调用 registerBuiltins()，否则会持有已销毁上下文的 GL handle。
// 当前设计为单线程使用，不做线程同步。

std::unordered_map<std::string, std::shared_ptr<RHIShader>> RHIShader::s_shaderRegistry;

void RHIShader::registerBuiltins() {
    if (s_shaderRegistry.find("passthrough") != s_shaderRegistry.end()) {
        return;
    }
    s_shaderRegistry["passthrough"] = std::make_shared<GLShader>(
        PASSTHROUGH_VERTEX, PASSTHROUGH_FRAGMENT);
    FANTASY_LOGI(TAG, "Built-in shaders registered");
}

void RHIShader::registerShader(const std::string& name,
                               const std::string& vertexSrc,
                               const std::string& fragmentSrc) {
    if (s_shaderRegistry.find(name) != s_shaderRegistry.end()) {
        return; // 已注册则跳过
    }
    s_shaderRegistry[name] = std::make_shared<GLShader>(vertexSrc, fragmentSrc);
    FANTASY_LOGI(TAG, "Custom shader registered: %s", name.c_str());
}

void RHIShader::clearRegistry() {
    s_shaderRegistry.clear();
}

std::shared_ptr<RHIShader> RHIShader::get(const std::string& name) {
    auto it = s_shaderRegistry.find(name);
    if (it == s_shaderRegistry.end()) {
        throw std::runtime_error("Shader not found: " + name);
    }
    return it->second;
}

} // namespace rhi
} // namespace fantasy
