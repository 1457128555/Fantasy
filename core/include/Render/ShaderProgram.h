#pragma once

#include "Common/Macros.h"
#include "Render/GL.h"       

#include <string>

namespace Fantasy::Render
{
    class ShaderProgram
    {
    public:
        ShaderProgram() = default;    
        ~ShaderProgram();

        [[nodiscard]] 
        bool initFromSource(const std::string& vsSrc, const std::string& fsSrc);

        void use() const;          

        [[nodiscard]] 
        GLuint id() const { return mProgram; }

        [[nodiscard]] 
        GLint uniformLocation(const std::string& name) const;

        [[nodiscard]] 
        GLint attribLocation (const std::string& name) const;

        FANTASY_NON_COPYABLE(ShaderProgram);    

    private:
        [[nodiscard]] 
        GLuint compileShader(GLenum type, const std::string& src);  
        
    private:
        GLuint mProgram = 0;
    };
}