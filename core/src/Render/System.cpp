#include "Render/System.h"
#if defined(__ANDROID__)
    #include "Render/EglContext.h"
#endif

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
        return mThread.initialize() 
            && mContext->initialize();
    }

    void System::renderOneFrame(float dt)
    {

    }

    void System::destroy()
    {
        mContext->destroy();
        mThread.destroy();
    }

    bool System::onSurfaceCreated(void* win)
    {
        return mContext->onSurfaceCreated(win);
    }

    void System::onSurfaceChanged(int w, int h)
    {
    }

    void System::onSurfaceDestroyed()
    {
        mContext->onSurfaceDestroyed();
    }
}