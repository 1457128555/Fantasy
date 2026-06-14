#include "Render/Texture.h"

namespace Fantasy::Render
{
    Texture::~Texture()
    {
        if (mTex)
            glDeleteTextures(1, &mTex);
    }

    bool Texture::initRGBA(int width, int height, const uint8_t* pixels)
    {
        if (width <= 0 || height <= 0 || !pixels)
            return false;
        if (mTex) 
            glDeleteTextures(1, &mTex);  
        mWidth = width; mHeight = height;

        glGenTextures(1, &mTex);
        glBindTexture(GL_TEXTURE_2D, mTex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA
            , width, height, 0
            , GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }

    void Texture::bind(int unit) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);  
        glBindTexture(GL_TEXTURE_2D, mTex);    
    }
}
