#include "Render/Context.h"

namespace Fantasy::Render
{
    Context::Context()
    {

    }

    Context::~Context()
    {

    }

    void Context::attach(IGLContext* context) 
    {
        mGLContext = context; 
    }

    void Context::detach() 
    { 
        mGLContext = nullptr;
    }
}