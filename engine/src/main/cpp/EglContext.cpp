#include "EglContext.h"
#include "Common/Logger.h"

#include <android/native_window.h>   

namespace Fantasy::Android
{
    EglContext::EglContext(ANativeWindow* window)
        : mWindow(window)
    {

    }

    EglContext::~EglContext()
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (mContext != EGL_NO_CONTEXT) eglDestroyContext(mDisplay, mContext);
            if (mSurface != EGL_NO_SURFACE) eglDestroySurface(mDisplay, mSurface);
        }
        mContext = EGL_NO_CONTEXT;
        mSurface = EGL_NO_SURFACE; 
        mDisplay = EGL_NO_DISPLAY;
    }

    bool EglContext::initialize()
    {
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (mDisplay == EGL_NO_DISPLAY) 
        {
            Common::Logger::Instance()->logE("EglContext", "getDisplay fail");
            return false;
        }

        if (eglInitialize(mDisplay, nullptr, nullptr) != EGL_TRUE) 
        {
            Common::Logger::Instance()->logE("EglContext", "init fail");
            return false; 
        }

        const EGLint cfgAttribs[] = {
            EGL_RENDERABLE_TYPE, 
            EGL_OPENGL_ES3_BIT,   
            EGL_SURFACE_TYPE, 
            EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
        EGLint num = 0;
        if (eglChooseConfig(mDisplay, cfgAttribs, &mConfig, 1, &num) != EGL_TRUE || num < 1)
        {
            Common::Logger::Instance()->logE("EglContext", "chooseConfig fail");
            return false;
        }

        EGLint visualId = 0;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &visualId);
        ANativeWindow_setBuffersGeometry(mWindow, 0, 0, visualId);   

        mSurface = eglCreateWindowSurface(mDisplay, mConfig, mWindow, nullptr);
        if (mSurface == EGL_NO_SURFACE) 
        { 
            Common::Logger::Instance()->logE("EglContext", "createWindowSurface fail");
            return false; 
        }

        const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, ctxAttribs);
        if (mContext == EGL_NO_CONTEXT)
        {
            Common::Logger::Instance()->logE("EglContext", "createContext fail");
            return false; 
        }

        return true;
    }

    bool EglContext::makeCurrent() 
    {
        return eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) == EGL_TRUE;
    }

    void EglContext::swapBuffers() 
    {
        eglSwapBuffers(mDisplay, mSurface);   
    }

    int EglContext::width()  const 
    {
        EGLint w = 0; 
        eglQuerySurface(mDisplay, mSurface, EGL_WIDTH, &w); 
        return w;
    }

    int EglContext::height() const 
    {
        EGLint h = 0;
        eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &h);
        return h;
    }
}