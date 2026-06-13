#pragma once
#include "Singleton.h"

namespace Fantasy::Common
{
    class Logger : public Singleton<Logger>
    {
    public:
        Logger();

        ~Logger();

        [[nodiscard]] bool initialize();

        void deinit();
    };
} 

