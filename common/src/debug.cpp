#include "debug.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

Debugger::Debugger(const char* path)
    : m_filePath(path)
{}

Debugger::~Debugger()
{}

void Debugger::Start()
{
    m_running = true; 
    m_debugThread = std::thread(&Debugger::Loop, this); 
}

void Debugger::Stop()
{
    m_running = false;
    if (m_debugThread.joinable())
        m_debugThread.join();

    FileWrite();
}

bool Debugger::Log(const char* msg)
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logQueue.push(msg);

    return true;
}


bool Debugger::FileWrite()
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    std::ofstream file(m_filePath);

    if (!file)
    {
        return false;
    }

    while (!m_logQueue.empty())
    {
        file << m_logQueue.front() << '\n';
        m_logQueue.pop();
    }

    return true;
}

void Debugger::Loop()
{
    while (m_running)
    {
        if (!m_logQueue.empty() && m_running)
            FileWrite();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}
