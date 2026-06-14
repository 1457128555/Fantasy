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
static bool g_glInited = false;       // GL 资源（program/texture/VBO）随 context 只建一次
static bool g_engineInited = false;   // 引擎是否已初始化（幂等 init/destroy 用）

extern "C"
JNIEXPORT jstring JNICALL
Java_com_fan_engine_EngineBridge_nativeHello(JNIEnv *env, jobject thiz) {
    __android_log_print(ANDROID_LOG_INFO, "engine", "nativeHello called");
    return env->NewStringUTF("test");
}

extern "C" JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeInit(JNIEnv* env, jobject thiz) {
    if (g_engineInited) return;   // 幂等：已初始化则跳过（进程复用、Activity 重建都安全）

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
        delete Engine::Instance();   // 回滚，保持可重试（否则单例残留，下次 new 会 assert）
        return;
    }
    g_engineInited = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeDestroy(JNIEnv* env, jobject thiz) {
    if (!g_engineInited) return;   // 未初始化 / 已销毁 → 幂等返回

    System::Instance()->postAndWait([]() {   // ① 渲染线程上拆 GL/EGL（线程还活着）
        System::Instance()->releaseRenderer();
        System::Instance()->getContext()->detach();   // 解除 Context 对即将销毁的 eglContext 的引用
        g_eglContext.reset();                          // 毁 surface(若有)+context → 释放全部 GL 资源
    });
    g_glInited = false;
    if (g_window) { ANativeWindow_release(g_window); g_window = nullptr; }

    Engine::Instance()->deinit();   // ② 停渲染线程 + 各 deinit（此时已无 GL 活）
    delete Engine::Instance();      // ③ ~Engine 删 System/Logger，Singleton 复位 sInstance
    g_engineInited = false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fan_engine_EngineBridge_nativeSurfaceCreated(JNIEnv *env, jobject thiz, jobject surface) {
    g_window = ANativeWindow_fromSurface(env, surface);

    System::Instance()->post([win = g_window]() {
        if (!g_eglContext) {                              // context 只建一次（app 级）
            g_eglContext = std::make_unique<EglContext>();
            if (!g_eglContext->initialize()) {
                Logger::Instance()->logE("engine", "EglContext init fail");
                return;
            }
            System::Instance()->getContext()->attach(g_eglContext.get());
        }

        if (!g_eglContext->createSurface(win)) {          // EGLSurface 每个新 window 建一次
            Logger::Instance()->logE("engine", "createSurface fail");
            return;
        }
        if (!g_eglContext->makeCurrent()) {
            Logger::Instance()->logE("engine", "makeCurrent fail");
            return;
        }

        if (!g_glInited) {                                // GL 资源只建一次，surface 重建后复用（含已选纹理）
            if (!System::Instance()->initRenderer()) {
                Logger::Instance()->logE("engine", "initRenderer fail");
                return;
            }
            g_glInited = true;
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
    System::Instance()->postAndWait([]() {
        g_eglContext->destroySurface();   // 只毁 EGLSurface，留住 context + GL 资源（已选纹理还在）
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
        if (g_eglContext && g_eglContext->hasSurface()) {   // 有 surface 才渲染（无 surface 时 context 未 current）
            System::Instance()->renderFrame(g_eglContext->width(), g_eglContext->height());
            g_eglContext->swapBuffers();
        }
    });
}
