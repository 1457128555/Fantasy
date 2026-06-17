#pragma once
#include "Common/Singleton.h"

namespace Fantasy
{
    class Engine : public Common::Singleton<Engine>
    {
    public:
        Engine();
        
        ~Engine();
        
        bool initialize();
        
        void destroy();

        void renderOneFrame(float dt);

        bool onSurfaceCreated(void* win);

        void onSurfaceChanged(int w, int h);

        void* onSurfaceDestroyed();

    private:
        void* mWinHandle = nullptr;
    };
} // namespace Fantasy
