#include "Render/EaglContext.h"
#include "Common/Logger.h"

#if defined(__APPLE__)

#import <QuartzCore/CAEAGLLayer.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#include <string>

namespace Fantasy::Render
{
    namespace
    {
        const std::string _Tag = "EaglContext";
        void _LogN(const std::string& m) { Common::Logger::Instance()->logN(_Tag, m); }
        void _LogE(const std::string& m) { Common::Logger::Instance()->logE(_Tag, m); }
    }

    EaglContext::EaglContext()  { _LogN("Constructor"); }
    EaglContext::~EaglContext() { _LogN("Destructor"); }

    bool EaglContext::initialize()
    {
        _LogN("initialize");
        EAGLContext* ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if (!ctx) { _LogE("create EAGLContext fail"); return false; }
        mContext = (__bridge_retained void*)ctx;  
        return true;
    }

    void EaglContext::destroy()
    {
        _LogN("destroy");
        [EAGLContext setCurrentContext:nil];
        if (mContext)
        {
            EAGLContext* ctx = (__bridge_transfer EAGLContext*)mContext; 
            (void)ctx;
            mContext = nullptr;
        }
        mLayer = nullptr;
    }

    bool EaglContext::onSurfaceCreated(void* window)
    {
        _LogN("onSurfaceCreated");
        mLayer = window;                                  
        CAEAGLLayer*  layer = (__bridge CAEAGLLayer*)mLayer;
        EAGLContext*  ctx   = (__bridge EAGLContext*)mContext;

        if (![EAGLContext setCurrentContext:ctx]) { _LogE("setCurrentContext fail"); return false; }

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glGenRenderbuffers(1, &mColorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffer);
        [ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];  
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, mColorRenderbuffer);

        GLint w = 0, h = 0;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,  &w);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
        mWidth = w; mHeight = h;

        GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (st != GL_FRAMEBUFFER_COMPLETE) { _LogE("framebuffer incomplete"); return false; }

        _LogN("surface " + std::to_string(mWidth) + "x" + std::to_string(mHeight));

        [EAGLContext setCurrentContext:nil];
        return true;
    }

    void EaglContext::onSurfaceDestroyed()
    {
        _LogN("onSurfaceDestroyed");
        EAGLContext* ctx = (__bridge EAGLContext*)mContext;
        [EAGLContext setCurrentContext:ctx];
        if (mColorRenderbuffer) { glDeleteRenderbuffers(1, &mColorRenderbuffer); mColorRenderbuffer = 0; }
        if (mFramebuffer)       { glDeleteFramebuffers(1, &mFramebuffer);        mFramebuffer = 0; }
        mLayer = nullptr;
    }

    bool EaglContext::makeCurrent()
    {
        _LogN("makeCurrent");
        EAGLContext* ctx = (__bridge EAGLContext*)mContext;
        if (![EAGLContext setCurrentContext:ctx]) return false;
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer); 
        return true;
    }

    void EaglContext::swapBuffers()
    {
        _LogN("swapBuffers");
        EAGLContext* ctx = (__bridge EAGLContext*)mContext;
        glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffer);
        [ctx presentRenderbuffer:GL_RENDERBUFFER];         
    }

    int EaglContext::width()  const { return mWidth; }
    int EaglContext::height() const { return mHeight; }
}
#endif
