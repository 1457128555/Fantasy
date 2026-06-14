#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>   

#include <GLES3/gl3.h> 
#include <memory>
#include <cstring>         
#include <vector>

#include "Engine.h"             
#include "Render/System.h"
#include "Render/Context.h"
#include "Common/Logger.h"
#include "EglContext.h"       

using Fantasy::Engine;
using Fantasy::Render::System;
using Fantasy::Common::Logger;
using Fantasy::Android::EglContext;

static ANativeWindow* g_window = nullptr;
static std::unique_ptr<EglContext> g_eglContext;  

extern "C"
JNIEXPORT jstring JNICALL
Java_com_fan_engine_EngineBridge_nativeHello(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "engine", "nativeHello called");
    return env->NewStringUTF("test");
}

extern "C" JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeInit(JNIEnv* env, jobject thiz) {
    new Engine();   

    Logger::Instance()->setSink(
        [](Logger::Level lvl, const std::string& tag, const std::string& msg) {
            int prio = lvl == Logger::Error   ? ANDROID_LOG_ERROR
                     : lvl == Logger::Warning ? ANDROID_LOG_WARN
                                              : ANDROID_LOG_INFO;
            __android_log_print(prio, tag.c_str(), "%s", msg.c_str());  
        }
    );

    if (!Engine::Instance()->initialize()) {
        Logger::Instance()->logE("engine", "Engine initialize failed");
        return;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeSurfaceCreated(JNIEnv *env, jobject thiz, jobject surface) {
    g_window = ANativeWindow_fromSurface(env, surface);

    System::Instance()->post([win = g_window]() {        
        g_eglContext = std::make_unique<EglContext>(win);
        if (!g_eglContext->initialize()) {                
            Logger::Instance()->logE("engine", "EglContext init fail");
            return;
        }
        System::Instance()->getContext()->attach(g_eglContext.get());  

        if (!g_eglContext->makeCurrent()) {               
            Logger::Instance()->logE("engine", "makeCurrent fail");
            return;
        }

        if (!System::Instance()->initRenderer()) {       // 建 program+VBO（本 context 一次）
            Logger::Instance()->logE("engine", "initRenderer fail");
            return;
        }
        System::Instance()->renderFrame(g_eglContext->width(), g_eglContext->height());
        
        g_eglContext->swapBuffers();
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeSurfaceChanged(JNIEnv *env, jobject thiz, jint width,
                                                      jint height) {
    System::Instance()->post([width, height]() {
        glViewport(0, 0, width, height);   
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeSurfaceDestroyed(JNIEnv *env, jobject thiz) {
    System::Instance()->post([]() {
        System::Instance()->getContext()->detach();
        g_eglContext.reset();   
    });

    if (g_window) {
        ANativeWindow_release(g_window);  
        g_window = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeSetImage(JNIEnv* env, jobject thiz, jobject bitmap) {
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS) {
        __android_log_print(ANDROID_LOG_ERROR, "engine", "getInfo fail"); return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        __android_log_print(ANDROID_LOG_ERROR, "engine", "bitmap not RGBA8888"); return;
    }

    void* src = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap, &src) != ANDROID_BITMAP_RESULT_SUCCESS) {  
        __android_log_print(ANDROID_LOG_ERROR, "engine", "lockPixels fail"); return;
    }

    const int w = (int)info.width;
    const int h = (int)info.height;
    auto pixels = std::make_shared<std::vector<uint8_t>>((size_t)w * h * 4);
    const uint8_t* s = (const uint8_t*)src;
    for (int y = 0; y < h; ++y)
        memcpy(pixels->data() + (size_t)y * w * 4, s + (size_t)y * info.stride, (size_t)w * 4);

    AndroidBitmap_unlockPixels(env, bitmap);   

    System::Instance()->post([w, h, pixels]() {
        System::Instance()->setImage(w, h, pixels->data());
        if (g_eglContext) {
            System::Instance()->renderFrame(g_eglContext->width(), g_eglContext->height());
            g_eglContext->swapBuffers();
        }
    });
}
