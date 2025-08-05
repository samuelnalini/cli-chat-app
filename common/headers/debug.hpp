#pragma once

#include <string>
#include <sstream>
#include <queue>
#include <mutex>

class Debug
{
public:

    enum class LOG_LEVEL
    {
        INFO,
        WARNING,
        ERROR
    };

    template<typename T>
    static bool Log(const T& msg, LOG_LEVEL level = LOG_LEVEL::INFO)
    {
        std::ostringstream oss;
        oss << '[' << GetTimestamp() << "] "
            << '[' << ToString(level) << "] "
            << msg;
        return m_log(oss.str());
    }

    static bool DumpToFile(const std::string& path);
    static bool DumpToFile(const char* path);
    
    static void Flush();

public:
    static constexpr size_t MAX_LOG_ENTRIES{ 10000 };

private:
    static std::string GetTimestamp();
    static std::string ToString(LOG_LEVEL level);

private:
    static std::mutex m_debugMutex;
    static bool m_log(const std::string& msg);
    static std::queue<std::string> m_logQueue;
};



template<>
inline bool Debug::Log<bool>(const bool& b, LOG_LEVEL level)
{
    std::ostringstream oss;
    oss << '[' << GetTimestamp() << "] "
        << '[' << ToString(level) << "] "
        << (b ? "true" : "false");

    return m_log(oss.str());
}
