#include "Engine.h"
#include "Common/Logger.h"
#include "Render/System.h"

namespace Fantasy
{
    Engine::Engine()
    {
        new Common::Logger;
        new Render::System;
    }

    bool Engine::initialize()
    {
        return Common::Logger::Instance()->initialize()
            && Render::System::Instance()->initialize();
    }

    void Engine::renderOneFrame(float dt)
    {
        Render::System::Instance()->renderOneFrame(dt);
    }

    void Engine::destroy()
    {
        Render::System::Instance()->destroy();
        Common::Logger::Instance()->destroy();
    }

    bool Engine::onSurfaceCreated(void* win)
    {
        mWinHandle = win;
        return Render::System::Instance()->onSurfaceCreated(win);
    }

    void Engine::onSurfaceChanged(int w, int h)
    {
        mWidth = w, mHeight = h;
        Render::System::Instance()->onSurfaceChanged(w, h);
    }

    void* Engine::onSurfaceDestroyed()
    {
        Render::System::Instance()->onSurfaceDestroyed();
        return mWinHandle;
    }

    void Engine::setImage(void* data, int w, int h)
    {
        mImgW = w, mImgH = h;
        mImage.resize((size_t)w * h * 4);
        memcpy(mImage.data(), data, (size_t)w * h* 4);
    }

    Engine::~Engine()
    {
        delete Render::System::Instance();
        delete Common::Logger::Instance();
    }
} // namespace Fantasy
