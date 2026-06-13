#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <GLES3/gl3.h> 
#include <memory>

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
        glClearColor(0.8f, 0.1f, 0.1f, 1.0f);             
        glClear(GL_COLOR_BUFFER_BIT);
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