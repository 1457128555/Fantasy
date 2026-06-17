#include "Common/Logger.h"

#include <utility>

namespace Fantasy::Common
{
    Logger::Logger()
    {

    }

    Logger::~Logger()
    {

    }

    bool Logger::initialize() 
    {
        return true;
    }

    void Logger::destroy()
    {

    }

    void Logger::setSink(Sink sink)
    {
        mSink = std::move(sink);
    }

    void Logger::log(Level level
        , const std::string& tag
        , const std::string& msg)
    {
        if (mSink) 
            mSink(level, tag, msg);   
    }

    void Logger::logN(const std::string& tag, const std::string& msg)
    {
        log(Normal, tag, msg);
    }

    void Logger::logW(const std::string& tag, const std::string& msg)
    {
        log(Warning, tag, msg);
    }
    
    void Logger::logE(const std::string& tag, const std::string& msg) 
    {
        log(Error, tag, msg);
    }
}