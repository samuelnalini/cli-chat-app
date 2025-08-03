#pragma once

#include <filesystem>
#include <queue>
#include <mutex>

typedef class Debugger
{
public:
    explicit Debugger(const char* path);

    bool Log(const char* msg);
    bool Log(bool val);
    void Start();
    void Stop();
    bool FileWrite();

private:
    std::filesystem::path m_filePath{ "log.txt" };
    std::queue<const char*> m_logQueue;
    std::mutex m_logMutex;

    bool m_running{ false };

private:
    void Loop();

} Debugger;
