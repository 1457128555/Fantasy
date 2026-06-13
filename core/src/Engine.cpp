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
        return Common::Logger::Instance()->initialize();
        return Render::System::Instance()->initialize();
    }

    void Engine::deinit()
    {
        return Render::System::Instance()->deinit();
        return Common::Logger::Instance()->deinit();
    }

    Engine::~Engine()
    {
        delete Render::System::Instance();
        delete Common::Logger::Instance();
    }
} // namespace Fantasy
