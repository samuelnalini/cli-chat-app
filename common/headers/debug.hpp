#pragma once

#include <filesystem>
#include <queue>
#include <thread>
#include <mutex>

typedef class Debugger
{
public:
    explicit Debugger(const char* path);
    ~Debugger();

    bool Log(const char* msg);
    void Start();
    void Stop();
    bool FileWrite();

private:
    std::filesystem::path m_filePath{ "log.txt" };
    std::queue<const char*> m_logQueue;
    std::mutex m_logMutex;

    std::thread m_debugThread;
    bool m_running{ false };

private:
    void Loop();

} Debugger;
