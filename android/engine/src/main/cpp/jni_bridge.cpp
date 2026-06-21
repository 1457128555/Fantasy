

#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>

#include <vector>

#include "Common/Logger.h"
#include "Engine.h"
#include "EglContext.h"

#include <memory>

using namespace Fantasy;
extern "C"
{
    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_initialize(JNIEnv* env, jobject thiz)
    {
        auto engine = new Engine();
        
        Common::Logger::Instance()->setSink(
            [](Common::Logger::Level lvl, const std::string& tag, const std::string& msg) {
                int prio = lvl == Common::Logger::Error   ? ANDROID_LOG_ERROR
                         : lvl == Common::Logger::Warning ? ANDROID_LOG_WARN
                                                          : ANDROID_LOG_INFO;
                __android_log_print(prio, tag.c_str(), "%s", msg.c_str());
            }
        );

        Engine::Instance()->setContext(std::make_unique<Render::EglContext>());

        if (!Engine::Instance()->initialize()) {
            Common::Logger::Instance()->logE("JNI_Bridge", "Engine initialize failed");
            engine->destroy();
            delete engine;
        }
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_destroy(JNIEnv* env, jobject thiz) {
        Engine::Instance()->destroy();
        delete Engine::Instance();
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_renderOneFrame(JNIEnv* env, jobject thiz) {
        Engine::Instance()->renderOneFrame(0.f);
    }

    JNIEXPORT bool JNICALL
    Java_com_fan_engine_EngineBridge_onSurfaceCreated(JNIEnv *env, jobject thiz, jobject surface) {
        ANativeWindow* win = ANativeWindow_fromSurface(env, surface);
        return Engine::Instance()->onSurfaceCreated(win);
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_onSurfaceChanged(JNIEnv *env, jobject thiz, jint width, jint height) {
        Engine::Instance()->onSurfaceChanged(width, height);
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_onSurfaceDestroyed(JNIEnv *env, jobject thiz) {
        void* win = Engine::Instance()->onSurfaceDestroyed();
        ANativeWindow_release(static_cast<ANativeWindow*>(win)); 
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_setImage(JNIEnv* env, jobject thiz, jobject bitmap) {

         AndroidBitmapInfo info;
         if (AndroidBitmap_getInfo(env, bitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS) {
             Common::Logger::Instance()->logE("engine", "getInfo fail"); return;
         }
         if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
             Common::Logger::Instance()->logE("engine", "bitmap not RGBA8888"); return;
         }

         void* src = nullptr;
         if (AndroidBitmap_lockPixels(env, bitmap, &src) != ANDROID_BITMAP_RESULT_SUCCESS) {
             Common::Logger::Instance()->logE("engine", "lockPixels fail"); return;
         }

         const int w = (int)info.width;
         const int h = (int)info.height;
         std::vector<uint8_t>pixels ((size_t)w * h * 4);
         const uint8_t* s = (const uint8_t*)src;
         for (int y = 0; y < h; ++y)
             memcpy(pixels.data() + (size_t)y * w * 4, s + (size_t)y * info.stride, (size_t)w * 4);

         AndroidBitmap_unlockPixels(env, bitmap);

         Engine::Instance()->setImage(pixels.data(), w, h);
    }

    JNIEXPORT void JNICALL
    Java_com_fan_engine_EngineBridge_renderOneFrame(JNIEnv *env, jobject thiz) {
        Engine::Instance()->renderOneFrame(0.f);
    }
}

