#include "Render/Renderer.h"
#include "Common/Logger.h"

namespace Fantasy::Render
{
    namespace   
    {
        const char* kVertexSrc = R"(#version 300 es
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aUV;
            out vec2 vUV;
            void main() {
                vUV = aUV;
                gl_Position = vec4(aPos, 0.0, 1.0);
            }
        )";

        const char* kFragmentSrc = R"(#version 300 es
            precision mediump float;
            in vec2 vUV;
            uniform sampler2D uTex;
            out vec4 fragColor;
            void main() {
                fragColor = texture(uTex, vec2(vUV.x, 1.0 - vUV.y));  
            }
        )";

        const float kQuad[] = {
            -0.5f, -0.5f,   0.0f, 0.0f,
             0.5f, -0.5f,   1.0f, 0.0f,
            -0.5f,  0.5f,   0.0f, 1.0f,
             0.5f,  0.5f,   1.0f, 1.0f,
        };
    }

    Renderer::~Renderer()
    {
        if (mVBO) 
            glDeleteBuffers(1, &mVBO);
    }

    bool Renderer::initGL()
    {
        if (!mProgram.initFromSource(kVertexSrc, kFragmentSrc))
            return false;

        //  Texture
        const int N = 8;
        std::vector<uint8_t> pixels(N * N * 4);
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x)
            {
                uint8_t c = (((x + y) & 1) == 0) ? 235 : 40;
                int i = (y * N + x) * 4;
                pixels[i+0] = c; pixels[i+1] = c; pixels[i+2] = c; pixels[i+3] = 255;
            }
        if (!mTexture.initRGBA(N, N, pixels.data()))
            return false;

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kQuad), kQuad, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }

    void Renderer::render(int width, int height)
    {
        glViewport(0, 0, width, height);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mProgram.use();

        mTexture.bind(0);
        glUniform1i(mProgram.uniformLocation("uTex"), 0);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        const GLsizei stride = 4 * sizeof(float);
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const void*)0);
        glEnableVertexAttribArray(1);  
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(2 * sizeof(float)));

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Renderer::setImage(int width, int height, const uint8_t* pixels)
    {
        mTexture.initRGBA(width, height, pixels);   
    }
}
