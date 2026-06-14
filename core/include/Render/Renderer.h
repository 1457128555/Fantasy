#pragma once

#include "Common/Macros.h"
#include "Render/GL.h"
#include "Render/ShaderProgram.h"

namespace Fantasy::Render
{
    class Renderer
    {
    public:
        Renderer()  = default; 
        ~Renderer();

        [[nodiscard]] 
        bool initGL();       

        void render(int width, int height); 

        FANTASY_NON_COPYABLE(Renderer);
    private:
        ShaderProgram mProgram; 
        GLuint        mVBO = 0;
    };
}