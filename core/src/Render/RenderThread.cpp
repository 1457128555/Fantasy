#include "Render/RenderThread.h"
#include <utility>

namespace Fantasy::Render
{
    RenderThread::RenderThread()
    {

    }

    RenderThread::~RenderThread()
    {
        stop();
    }

    void RenderThread::start()
    {
        mThread = std::thread([this](){
            CommandQueue::Task task;
            while (mCommandQueue.waitAndPop(task))
            {
                task();
            }
        });
    }

    void RenderThread::post(CommandQueue::Task task)
    {
        mCommandQueue.post(std::move(task));
    }

    void RenderThread::stop()
    {
        mCommandQueue.stop();
        if(mThread.joinable())
            mThread.join();
    }
} // namespace Fantasy::Render

