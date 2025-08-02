#include "debug.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

Debugger::Debugger(const char* path)
    : m_filePath(path)
{}

Debugger::~Debugger() = default;

void Debugger::Start()
{
    m_running = true; 
    m_debugThread = std::thread(&Debugger::Loop, this); 
    Log("[*] Debugger started");
}

void Debugger::Stop()
{
    if (!m_running)
        return;
    
    Log("[!] Debugger stopped");
    m_running = false;
    if (m_debugThread.joinable())
        m_debugThread.join();

    FileWrite();
}

bool Debugger::Log(const char* msg)
{
    if (!m_running)
        return false;

    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logQueue.push(msg);

    return true;
}

bool Debugger::Log(bool val)
{
    if (!m_running)
        return false;

    return Log(val ? "true" : "false");
}

bool Debugger::FileWrite()
{
    std::lock_guard<std::mutex> lock(m_logMutex);
    std::fstream file(m_filePath, std::ios::app | std::ios::trunc | std::ios::out);

    if (!file)
    {
        //std::cerr << "Failed to open file" << m_filePath << '\n';
        return false;
    }

    while (!m_logQueue.empty())
    {
        file << m_logQueue.front() << '\n';
        m_logQueue.pop();
    }

    //std::cout << "Logged\n";
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
