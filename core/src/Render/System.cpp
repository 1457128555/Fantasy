#include "Render/System.h"
#include "Render/Context.h"
#include <memory>

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
        return true;
    }

    void System::deinit()
    {
        mContext.reset();
    }
}