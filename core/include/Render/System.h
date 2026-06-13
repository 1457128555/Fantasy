#pragma once
#include "Common/Singleton.h"
#include <memory>

namespace Fantasy::Render
{
    class System : public Common::Singleton<System>
    {
    public:
        System();
        
        ~System();

        [[nodiscard]] bool initialize();
        
        void deinit();
    
    private:
        std::unique_ptr<class Context> mContext;
    };
}
