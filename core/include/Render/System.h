#pragma once

#include "Common/Singleton.h"
#include "Render/RenderThread.h"
#include <memory>

namespace Fantasy::Render
{
    class IGLContext;
    class System : public Common::Singleton<System>
    {
    public:
        System();
        
        ~System();

        bool initialize();
        
        void destroy();

        void renderOneFrame(float dt);

        bool onSurfaceCreated(void* win);

        void onSurfaceChanged(int w, int h);

        void onSurfaceDestroyed();

        void setContext(std::unique_ptr<IGLContext> ctx);

    private:
        RenderThread mThread;
        std::unique_ptr<IGLContext> mContext;
    };
}
