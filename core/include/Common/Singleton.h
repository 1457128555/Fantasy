#pragma once
#include "Macros.h"
#include <cassert>

namespace Fantasy::Common
{
    template <typename T>
    class Singleton 
    {
        friend T; 
    public:
        static T* Instance() 
        {
            assert(nullptr != sInstance);
            return sInstance;
        }
        
        FANTASY_NON_COPYABLE(Singleton);

    private:
        Singleton()
        {
            assert(nullptr == sInstance);
            sInstance = static_cast<T*>(this);
        }

        ~Singleton()
        {
            assert(nullptr != sInstance);
            sInstance = nullptr;
        }

    private:
        inline static T* sInstance = nullptr;
    };
}
