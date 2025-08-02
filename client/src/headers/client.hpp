#pragma once

#include "ncursesUI.hpp"
#include "network_session.hpp"
#include "debug.hpp"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

class Client
{
public:
    Client(std::string ip = "127.0.0.1", int port = 8080);
    ~Client();

    void Start();
    void Stop();

private:
    std::unique_ptr<NetworkSession> m_session;

    NcursesUI m_ui;
    Debugger m_debugger;

    std::vector<std::thread> m_threadPool;

    std::string m_ip;
    int         m_port;

    std::string m_username;

    std::atomic<bool> m_stopping{ false };
    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_uiActive{ false };
    std::string       m_exitReason{ "None" };
    std::mutex        m_netMutex;

private:
    bool CreateSession();
    void CloseSession();

    void ClientLoop();
    void UIUpdateLoop();
    void HandleBroadcast();
};
