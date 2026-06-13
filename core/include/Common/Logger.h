#pragma once
#include "Singleton.h"

#include <string>
#include <functional>

namespace Fantasy::Common
{
    class Logger : public Singleton<Logger>
    {
    public:
        Logger();

        ~Logger();

        [[nodiscard]] bool initialize();

        void deinit();

        enum Level
        {
            Normal,
            Warning,
            Error
        };

        // 平台输出注入点：胶水 setSink 注册具体后端
        // （Android __android_log_print / iOS os_log / host printf）。core 不知道用的是哪个。
        using Sink = std::function<void(Level level, const std::string& tag, const std::string& msg)>;
        void setSink(Sink sink);

        void log(Level level, const std::string& tag, const std::string& msg);
        void logN(const std::string& tag, const std::string& msg);
        void logW(const std::string& tag, const std::string& msg);
        void logE(const std::string& tag, const std::string& msg);

    private:
        Sink mSink;   // 未注册时静默
    };
} 

