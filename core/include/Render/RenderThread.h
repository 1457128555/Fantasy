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

        void start();

        void post(CommandQueue::Task task);

        void stop();

        FANTASY_NON_COPYABLE(RenderThread);

    private:
        std::thread mThread;    
        CommandQueue mCommandQueue;
    };
}