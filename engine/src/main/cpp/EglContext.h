#pragma once

#include "Render/IGLContext.h"
#include "Common/Macros.h"

#include <EGL/egl.h> 

struct ANativeWindow; 

namespace Fantasy::Android
{
    class EglContext : public Render::IGLContext
    {
    public:
        explicit EglContext(ANativeWindow* window); 
        ~EglContext() override; 

        bool initialize();

        [[nodiscard]] 
        bool makeCurrent() override;
        void swapBuffers() override;
        int width()  const override;
        int height() const override;

        FANTASY_NON_COPYABLE(EglContext);  

    private:
        ANativeWindow* mWindow  = nullptr;           
        EGLDisplay     mDisplay = EGL_NO_DISPLAY;     
        EGLSurface     mSurface = EGL_NO_SURFACE;
        EGLContext     mContext = EGL_NO_CONTEXT;    
        EGLConfig      mConfig  = nullptr;
    };
}