#pragma once
#include "Common/Macros.h"

namespace Fantasy::Render
{
    class IGLContext
    {
    public:
        virtual ~IGLContext() = default;

        virtual bool initialize() = 0;   

        virtual void destroy() = 0;

        virtual bool onSurfaceCreated(void* window) = 0;

        virtual void onSurfaceDestroyed() = 0;

        virtual bool makeCurrent() = 0;

        virtual void swapBuffers() = 0;

        virtual int width()  const = 0;

        virtual int height() const = 0;

        FANTASY_NON_COPYABLE(IGLContext);

    protected:
        IGLContext() = default;
    };
}
