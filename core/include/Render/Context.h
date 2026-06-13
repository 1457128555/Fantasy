#pragma once

namespace Fantasy::Render
{
    class IGLContext;
    class Context 
    {
    public:
        Context();

        ~Context();

        void attach(IGLContext* context);
        void detach();

    private:
        IGLContext* mGLContext = nullptr;
    };
}