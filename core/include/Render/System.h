#pragma once

#include "Common/Singleton.h"
#include "Render/CommandQueue.h"

#include <memory>

namespace Fantasy::Render
{
    class Context;
    class RenderThread;
    class Renderer; 
    class System : public Common::Singleton<System>
    {
    public:
        System();
        
        ~System();

        [[nodiscard]] bool initialize();
        
        void deinit();

        void post(CommandQueue::Task task);

        Context* getContext();

        [[nodiscard]] bool initRenderer(); 
        void renderFrame(int width, int height); 
    private:
        std::unique_ptr<Context>        mContext;
        std::unique_ptr<RenderThread>   mThread;
        std::unique_ptr<Renderer>       mRenderer; 
    };
}
