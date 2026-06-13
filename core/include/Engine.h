#pragma once
#include "Common/Singleton.h"

namespace Fantasy
{
    class Engine : public Common::Singleton<Engine>
    {
    public:
        Engine();
        
        ~Engine();
        
        [[nodiscard]] bool initialize();
        
        void deinit();

    private:

    };
} // namespace Fantasy
