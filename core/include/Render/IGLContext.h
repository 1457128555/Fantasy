#pragma once
#include "Common/Macros.h"

namespace Fantasy::Render
{
    class IGLContext
    {
    public:
        virtual ~IGLContext() = default;

        [[nodiscard]] virtual bool makeCurrent() = 0;

        virtual void swapBuffers() = 0;

        virtual int width()  const = 0;
        virtual int height() const = 0;

        FANTASY_NON_COPYABLE(IGLContext);

    protected:
        IGLContext() = default;
    };
}
