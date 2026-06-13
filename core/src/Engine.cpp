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

    void Engine::deinit()
    {
        Render::System::Instance()->deinit();
        Common::Logger::Instance()->deinit();
    }

    Engine::~Engine()
    {
        delete Render::System::Instance();
        delete Common::Logger::Instance();
    }
} // namespace Fantasy
