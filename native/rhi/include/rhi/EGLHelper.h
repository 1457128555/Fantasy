#pragma once

#include <EGL/egl.h>
#include <stdexcept>

namespace fantasy {
namespace rhi {

class EGLHelper {
public:
    EGLHelper() {
        m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (m_display == EGL_NO_DISPLAY) {
            throw std::runtime_error("eglGetDisplay failed");
        }

        if (!eglInitialize(m_display, nullptr, nullptr)) {
            m_display = EGL_NO_DISPLAY;
            throw std::runtime_error("eglInitialize failed");
        }

        EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };

        EGLint numConfigs;
        EGLConfig config;
        if (!eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
            cleanup();
            throw std::runtime_error("eglChooseConfig failed");
        }

        EGLint pbufferAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE
        };
        m_surface = eglCreatePbufferSurface(m_display, config, pbufferAttribs);
        if (m_surface == EGL_NO_SURFACE) {
            cleanup();
            throw std::runtime_error("eglCreatePbufferSurface failed");
        }

        EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
        };
        m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, contextAttribs);
        if (m_context == EGL_NO_CONTEXT) {
            cleanup();
            throw std::runtime_error("eglCreateContext failed");
        }

        if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
            cleanup();
            throw std::runtime_error("eglMakeCurrent failed");
        }
    }

    ~EGLHelper() {
        cleanup();
    }

    EGLHelper(const EGLHelper&) = delete;
    EGLHelper& operator=(const EGLHelper&) = delete;

private:
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLSurface m_surface = EGL_NO_SURFACE;
    EGLContext m_context = EGL_NO_CONTEXT;

    void cleanup() {
        if (m_display != EGL_NO_DISPLAY) {
            eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (m_context != EGL_NO_CONTEXT) {
                eglDestroyContext(m_display, m_context);
                m_context = EGL_NO_CONTEXT;
            }
            if (m_surface != EGL_NO_SURFACE) {
                eglDestroySurface(m_display, m_surface);
                m_surface = EGL_NO_SURFACE;
            }
            eglTerminate(m_display);
            m_display = EGL_NO_DISPLAY;
        }
    }
};

} // namespace rhi
} // namespace fantasy
