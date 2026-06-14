#pragma once

#include "Common/Macros.h"
#include "Render/GL.h"

#include <cstdint>

namespace Fantasy::Render
{
    class Texture
    {
    public:
        Texture()  = default;
        ~Texture();

        [[nodiscard]] 
        bool initRGBA(int width, int height, const uint8_t* pixels);

        void bind(int unit) const;    
        
        [[nodiscard]]
        GLuint id() const { return mTex; }

        FANTASY_NON_COPYABLE(Texture);
    private:
        GLuint mTex    = 0;
        int    mWidth  = 0;
        int    mHeight = 0;
    };
}
