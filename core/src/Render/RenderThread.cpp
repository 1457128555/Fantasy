#include "Render/RenderThread.h"
#include <utility>
#include <future>

namespace Fantasy::Render
{
    RenderThread::RenderThread()
    {

    }

    RenderThread::~RenderThread()
    {

    }

    bool RenderThread::initialize()
    {
        mThread = std::thread([this](){
            CommandQueue::Task task;
            while (mCommandQueue.waitAndPop(task))
                task();
        });
        return true;
    }

    void RenderThread::destroy()
    {
        mCommandQueue.destroy();
        if(mThread.joinable())
            mThread.join();
    }

    void RenderThread::post(CommandQueue::Task task)
    {
        mCommandQueue.post(std::move(task));
    }

    void RenderThread::postAndWait(CommandQueue::Task task)
    {
        auto done = std::make_shared<std::promise<void>>();
        std::future<void> fut = done->get_future(); 
        post([task = std::move(task), done]() {          
            task();                                     
            done->set_value();                          
        });
        fut.get(); 
    }
} // namespace Fantasy::Render

