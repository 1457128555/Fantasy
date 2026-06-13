#pragma once
#include "Singleton.h"

namespace Fantasy::Common
{
    class Logger : public Singleton<Logger>
    {
    public:
        Logger();

        ~Logger();

        bool initialize();

        void deinit();
    };
} 

