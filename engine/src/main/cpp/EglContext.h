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
        EglContext();
        ~EglContext() override;

        bool initialize();                          // display + config + context（app 级，一次）

        bool createSurface(ANativeWindow* window);  // 建 EGLSurface（每个新 window）
        void destroySurface();                      // 毁 EGLSurface（surface 没了，留住 context）
        bool hasSurface() const { return mSurface != EGL_NO_SURFACE; }

        [[nodiscard]]
        bool makeCurrent() override;
        void swapBuffers() override;
        int width()  const override;
        int height() const override;

        FANTASY_NON_COPYABLE(EglContext);  

    private:
        EGLDisplay     mDisplay = EGL_NO_DISPLAY;
        EGLSurface     mSurface = EGL_NO_SURFACE;
        EGLContext     mContext = EGL_NO_CONTEXT;    
        EGLConfig      mConfig  = nullptr;
    };
}