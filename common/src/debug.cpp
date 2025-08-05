#include "debug.hpp"

#include <fstream>
#include <chrono>
#include <iomanip>

std::mutex Debug::m_debugMutex;
std::queue<std::string> Debug::m_logQueue;

bool Debug::m_log(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_debugMutex);

    if (m_logQueue.size() >= MAX_LOG_ENTRIES)
        m_logQueue.pop();

    m_logQueue.push(msg);

    return true;
}

bool Debug::DumpToFile(const std::string& path)
{
    std::lock_guard<std::mutex> lock(m_debugMutex);

    if (m_logQueue.empty())
        return true;

    std::ofstream file(path, std::ios::app);
    
    if (!file.is_open())
        return false;

    while (!m_logQueue.empty())
    {
        file << m_logQueue.front() << '\n';
        m_logQueue.pop();
    }

    return true;
}

bool Debug::DumpToFile(const char* path)
{
    return DumpToFile(std::string(path));
}

void Debug::Flush()
{
    std::lock_guard<std::mutex> lock(m_debugMutex);

    while (!m_logQueue.empty())
        m_logQueue.pop();
}

std::string Debug::ToString(LOG_LEVEL level)
{
    switch (level)
    {
        case LOG_LEVEL::INFO:
            return "INFO";
        case LOG_LEVEL::WARNING:
            return "WARN";
        case LOG_LEVEL::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Debug::GetTimestamp()
{
    auto now{ std::chrono::system_clock::now() };
    std::time_t t{ std::chrono::system_clock::to_time_t(now) };
    std::tm tm{ *std::localtime(&t) };

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
