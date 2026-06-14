#include "Render/ShaderProgram.h"
#include "Common/Logger.h"

#include <vector>

namespace Fantasy::Render
{
    ShaderProgram::~ShaderProgram()
    {
        if (mProgram)
            glDeleteProgram(mProgram);
    }

    GLuint ShaderProgram::compileShader(GLenum type, const std::string& src)
    {
        GLuint shader = glCreateShader(type);
        const char* c = src.c_str();
        glShaderSource(shader, 1, &c, nullptr);   
        glCompileShader(shader);

        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len > 1 ? len : 1);
            glGetShaderInfoLog(shader, (GLsizei)log.size(), nullptr, log.data());

            const char* kind = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
            Common::Logger::Instance()->logE("ShaderProgram",
                std::string("compile ") + kind + " fail: " + log.data());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    bool ShaderProgram::initFromSource(const std::string& vsSrc,
                                       const std::string& fsSrc)
    {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
        if (!vs) 
            return false;

        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
        if (!fs) 
        {
            glDeleteShader(vs); 
            return false; 
        }  

        mProgram = glCreateProgram();
        glAttachShader(mProgram, vs);
        glAttachShader(mProgram, fs);
        glLinkProgram(mProgram);

        glDeleteShader(vs);   
        glDeleteShader(fs);

        GLint ok = 0;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> log(len > 1 ? len : 1);
            glGetProgramInfoLog(mProgram, (GLsizei)log.size(), nullptr, log.data());
            Common::Logger::Instance()->logE("ShaderProgram",
                std::string("link fail: ") + log.data());
            glDeleteProgram(mProgram);
            mProgram = 0;
            return false;
        }
        return true;
    }

    void ShaderProgram::use() const 
    { 
        glUseProgram(mProgram);
    }

    GLint ShaderProgram::uniformLocation(const std::string& name) const
    {
        return glGetUniformLocation(mProgram, name.c_str());
    }

    GLint ShaderProgram::attribLocation(const std::string& name) const
    {
        return glGetAttribLocation(mProgram, name.c_str());
    }
}
