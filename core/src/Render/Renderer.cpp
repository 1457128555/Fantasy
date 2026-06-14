#include "Render/Renderer.h"
#include "Common/Logger.h"

namespace Fantasy::Render
{
    namespace   
    {
        const char* kVertexSrc = R"(#version 300 es
            layout(location = 0) in vec2 aPos;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);   
            }
        )";

        const char* kFragmentSrc = R"(#version 300 es
            precision mediump float;                  
            out vec4 fragColor;
            void main() {
                fragColor = vec4(0.95, 0.55, 0.15, 1.0);  
            }
        )";

        const float kQuad[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
            -0.5f,  0.5f,
             0.5f,  0.5f,
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
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);   // 深灰背景，衬橙色 quad
        glClear(GL_COLOR_BUFFER_BIT);

        mProgram.use();

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glEnableVertexAttribArray(0);                                   // location 0 = aPos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
