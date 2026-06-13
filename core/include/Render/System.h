#pragma once

#include "Common/Singleton.h"
#include "Render/CommandQueue.h"

#include <memory>

namespace Fantasy::Render
{
    class Context;
    class RenderThread;
    class System : public Common::Singleton<System>
    {
    public:
        System();
        
        ~System();

        [[nodiscard]] bool initialize();
        
        void deinit();

        void post(CommandQueue::Task task);

        Context* getContext();
    
    private:
        std::unique_ptr<Context>        mContext;
        std::unique_ptr<RenderThread>   mThread;
    };
}
