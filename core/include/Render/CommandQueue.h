#pragma once

#include "Common/Macros.h"

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Fantasy::Render
{
    class CommandQueue 
    {
    public:
        CommandQueue()  = default;
        ~CommandQueue() = default;
        
        using Task = std::function<void()>;
        void post(Task task);
        
        bool waitAndPop(Task& out);
        
        void destroy();

        FANTASY_NON_COPYABLE(CommandQueue);
    private:
        std::queue<Task>        mQueue;
        std::mutex              mMutex;
        std::condition_variable mCV;
        bool                    mStopped = false;
    };
}