#include "Render/System.h"
#if defined(__ANDROID__)
    #include "Render/EglContext.h"
#endif

#include "Render/GL.h"

namespace Fantasy::Render
{
    System::System()
    {
#if defined(__ANDROID__)
        mContext = std::make_unique<EglContext>();
#endif
    }

    System::~System()
    {

    }

    bool System::initialize() 
    {
        if(!mContext->initialize())
            return false;
        if(!mThread.initialize())
            return false;
        return true;
    }

    void System::renderOneFrame(float dt)
    {
        mThread.post([&](){
            // glViewport(0, 0, width, height);
            glClearColor(1.0f, 0.10f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            mContext->swapBuffers();
        });
    }

    void System::destroy()
    {
        mThread.destroy();
        mContext->destroy();
    }

    bool System::onSurfaceCreated(void* win)
    {
        bool ret = false;
        mThread.postAndWait([&](){
            ret = mContext->onSurfaceCreated(win);
        });

        mThread.post([&](){
            mContext->makeCurrent();
        });

        renderOneFrame(0.f);

        return ret;
    }

    void System::onSurfaceChanged(int w, int h)
    {       
        mThread.post([=](){
            glViewport(0, 0, w, h);
            // glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
            // glClear(GL_COLOR_BUFFER_BIT);
        });
    }

    void System::onSurfaceDestroyed()
    {
        mThread.postAndWait([&](){
            mContext->onSurfaceDestroyed();
        });
    }
}