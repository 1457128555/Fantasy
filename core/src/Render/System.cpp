#include "Render/System.h"
#include "Render/Context.h"
#include "Render/RenderThread.h"

#include <memory>
#include <utility>


namespace Fantasy::Render
{
    System::System()
    {

    }

    System::~System()
    {

    }

    bool System::initialize() 
    {
        mContext = std::make_unique<Context>();
        mThread = std::make_unique<RenderThread>();
        mThread->start();
        return true;
    }

    void System::deinit()
    {
        mThread->stop();
        mThread.reset();
        mContext.reset();
    }

    void System::post(CommandQueue::Task task)
    {
        mThread->post(std::move(task));
    }

    Context* System::getContext()
    {
        return mContext.get();
    }
}