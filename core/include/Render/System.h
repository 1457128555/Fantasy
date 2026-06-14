#pragma once

#include "Common/Singleton.h"
#include "Render/CommandQueue.h"

#include <memory>
#include <cstdint>     

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
        void postAndWait(CommandQueue::Task task);


        Context* getContext();

        [[nodiscard]] bool initRenderer(); 
        void renderFrame(int width, int height); 

        void setImage(int width, int height, const uint8_t* pixels);

        void releaseRenderer();   // 在渲染线程销毁 GL 资源（关闭时调，须 context 仍 current）

    private:
        std::unique_ptr<Context>        mContext;
        std::unique_ptr<RenderThread>   mThread;
        std::unique_ptr<Renderer>       mRenderer; 
    };
}
