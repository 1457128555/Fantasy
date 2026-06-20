#include "Render/Renderer.h"
#include "Common/Logger.h"

namespace Fantasy::Render
{
    namespace   
    {
        const char* kVertexSrc = R"(#version 300 es
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aUV;
            uniform vec2 uScale;          // CPU 算好的 fit 缩放(sx, sy)
            out vec2 vUV;
            void main() {
                vUV = aUV;
                gl_Position = vec4(aPos * uScale, 0.0, 1.0);
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
            -1.0f, -1.0f,   0.0f, 0.0f,
             1.0f, -1.0f,   1.0f, 0.0f,
            -1.0f,  1.0f,   0.0f, 1.0f,
             1.0f,  1.0f,   1.0f, 1.0f,
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

        // aspect-fit:把图片不变形地居中塞进 surface,短边留黑边
        float imgAspect  = (float)mImageW / (float)mImageH;
        float surfAspect = (float)width   / (float)height;
        float r = imgAspect / surfAspect;
        float sx, sy;
        if (r > 1.0f) { sx = 1.0f; sy = 1.0f / r; }   // 图更宽 → 撑满宽,上下黑边
        else          { sx = r;    sy = 1.0f;     }   // 图更高 → 撑满高,左右黑边
        glUniform2f(mProgram.uniformLocation("uScale"), sx, sy);

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
        mImageW = width;
        mImageH = height;
    }
}
