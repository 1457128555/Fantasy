#pragma once

#include "Common/Macros.h"
#include "Render/CommandQueue.h"

#include <thread>

namespace Fantasy::Render
{
    class RenderThread
    {
    public:
        RenderThread();
        
        ~RenderThread();

        bool initialize();
        
        void destroy();

        void post(CommandQueue::Task task);

        void postAndWait(CommandQueue::Task task);

        FANTASY_NON_COPYABLE(RenderThread);

    private:
        std::thread mThread;    
        CommandQueue mCommandQueue;
    };
}