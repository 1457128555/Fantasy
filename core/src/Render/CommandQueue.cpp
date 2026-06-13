#include "Render/CommandQueue.h"
#include <utility>

namespace Fantasy::Render
{
    void CommandQueue::post(Task task)
    {
        {
            std::lock_guard<std::mutex> lk(mMutex);
            mQueue.push(std::move(task));
        }
        mCV.notify_one();
    }
    
    bool CommandQueue::waitAndPop(Task& out)
    {
        std::unique_lock<std::mutex> lk(mMutex);
        mCV.wait(lk, [this](){
            return !mQueue.empty() || mStopped;
        });

        if(!mQueue.empty())        
        {
            out = std::move(mQueue.front());
            mQueue.pop();
            return true;
        }
        return false;
    }
    
    void CommandQueue::stop()
    {
        {
            std::lock_guard<std::mutex> lk(mMutex);
            mStopped = true;
        }
        mCV.notify_all();
    }
}