#include "EglContext.h"
#include "Common/Logger.h"

#if defined(__ANDROID__)
#include <android/native_window.h>   

namespace Fantasy::Render
{
    namespace
    {
        const std::string& _Tag = "EglContext";
        void _LogN(const std::string& msg)
        {
            Common::Logger::Instance()->logN(_Tag, msg);
        }

        void _LogE(const std::string& msg)
        {
            Common::Logger::Instance()->logE(_Tag, msg);
        }
    }

    EglContext::EglContext()
    {
        _LogN("Constructor");
    }

    EglContext::~EglContext()
    {
        _LogN("Destructor");
    }

    bool EglContext::initialize()
    {
        _LogN("initialize");
        mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (mDisplay == EGL_NO_DISPLAY) 
        {
            _LogE("getDisplay fail");
            return false;
        }

        if (eglInitialize(mDisplay, nullptr, nullptr) != EGL_TRUE) 
        {
            _LogE("init fail");
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
            _LogE("chooseConfig fail");
            return false;
        }

        const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, ctxAttribs);
        if (mContext == EGL_NO_CONTEXT)
        {
            _LogE("createContext fail");
            return false; 
        }
        return true;
    }

    void EglContext::destroy()
    {
        _LogN("destroy");
        if (mDisplay != EGL_NO_DISPLAY)
        {
            if(mContext != EGL_NO_CONTEXT)
                eglDestroyContext(mDisplay, mContext);
            eglTerminate(mDisplay);
        }
        mContext = EGL_NO_CONTEXT;
        mDisplay = EGL_NO_DISPLAY;
        mConfig  = nullptr;
    }

    bool EglContext::onSurfaceCreated(void* window)
    {
        _LogN("onSurfaceCreate");
        auto* win = static_cast<ANativeWindow*>(window);
        EGLint visualId = 0;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &visualId);
        ANativeWindow_setBuffersGeometry(win, 0, 0, visualId);

        mSurface = eglCreateWindowSurface(mDisplay, mConfig, win, nullptr);
        if (mSurface == EGL_NO_SURFACE)
        {
            _LogE("createWindowSurface fail");
            return false;
        }
        return true;
    }

    void EglContext::onSurfaceDestroyed()
    {
        _LogN("onSurfaceDestroy");
        if (mSurface != EGL_NO_SURFACE)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);  
            eglDestroySurface(mDisplay, mSurface);
            mSurface = EGL_NO_SURFACE;
        }
    }

    bool EglContext::makeCurrent()
    {
        _LogN("makeCurrent");
        return eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) == EGL_TRUE;
    }

    void EglContext::swapBuffers() 
    {
        _LogN("swapBuffers");
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
#endif