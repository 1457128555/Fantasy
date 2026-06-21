#include "Render/System.h"
#if defined(__ANDROID__)
    #include "Render/EglContext.h"
#elif defined(__APPLE__)
    #include "Render/EaglContext.h"
#endif

#include "Render/GL.h"
#include "Render/Renderer.h"
#include "Engine.h"

namespace Fantasy::Render
{
    System::System()
    {
#if defined(__ANDROID__)
        mContext = std::make_unique<EglContext>();
#elif defined(__APPLE__)
        mContext = std::make_unique<EaglContext>();
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
        int winW = Engine::Instance()->getWidth();
        int winH = Engine::Instance()->getHeight();
        int imgW = Engine::Instance()->getImgW();
        int imgH = Engine::Instance()->getImgH();

        auto imgPtr = std::make_shared<std::vector<uint8_t>>(Engine::Instance()->getImage());
        mThread.post([=](){
            Renderer r;
            r.initGL();
            r.setImage(imgW, imgH, imgPtr->data());
            r.render(winW, winH);
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
        if(!mContext->onSurfaceCreated(win))
            return false;

        mThread.post([&](){
            mContext->makeCurrent();
        });

        renderOneFrame(0.f);

        return true;
    }

    void System::onSurfaceChanged(int w, int h)
    {       
        mThread.post([=](){
            glViewport(0, 0, w, h);
        });
    }

    void System::onSurfaceDestroyed()
    {
        mThread.postAndWait([&](){
            mContext->onSurfaceDestroyed();
        });
    }
}