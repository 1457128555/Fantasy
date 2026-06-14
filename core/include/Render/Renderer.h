#pragma once

#include "Common/Macros.h"
#include "Render/GL.h"
#include "Render/ShaderProgram.h"
#include "Render/Texture.h"

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
        
        void setImage(int width, int height, const uint8_t* pixels);  

        FANTASY_NON_COPYABLE(Renderer);
    private:
        ShaderProgram mProgram; 
        Texture       mTexture;
        GLuint        mVBO = 0;
    };
}