#pragma once
#if defined(__ANDROID__)

#include "Render/IGLContext.h"
#include "Common/Macros.h"

#include <EGL/egl.h> 

namespace Fantasy::Render
{
    class EglContext : public Render::IGLContext
    {
    public:
        EglContext();
        ~EglContext() override;

        virtual bool initialize() override;   

        virtual void destroy() override;

        virtual bool onSurfaceCreated(void* window) override;

        virtual void onSurfaceDestroyed() override;

        virtual bool makeCurrent() override;

        virtual void swapBuffers() override;

        virtual int width()  const override;

        virtual int height() const override;

    private:
        EGLDisplay mDisplay = EGL_NO_DISPLAY;
        EGLSurface mSurface = EGL_NO_SURFACE;
        EGLContext mContext = EGL_NO_CONTEXT;    
        EGLConfig  mConfig  = nullptr;
    };
}
#endif