#pragma once
#include "Render/IGLContext.h"

#if defined(__APPLE__)

namespace Fantasy::Render
{
    class EaglContext : public IGLContext
    {
    public:
        EaglContext();
        ~EaglContext() override;

        bool initialize() override;
        void destroy() override;

        bool onSurfaceCreated(void* window) override;   
        void onSurfaceDestroyed() override;

        bool makeCurrent() override;
        void swapBuffers() override;

        int width()  const override;
        int height() const override;

    private:
        void* mContext = nullptr;            
        void* mLayer   = nullptr;          
        unsigned int mFramebuffer       = 0; 
        unsigned int mColorRenderbuffer = 0; 
        int mWidth  = 0;
        int mHeight = 0;
    };
}
#endif
