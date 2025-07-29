#pragma once

#include "ncursesUI.hpp"

#include <netinet/in.h>
#include <string>
#include <vector>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <atomic>

class Client
{
public:
    Client() {};
    Client(std::string ip, int port)
    : m_port(port)
    , m_ip(ip)
    {}

    void Start();
public:
    std::atomic<bool> running{ false };
    std::atomic<bool> uiActive{ false };
private:
    std::vector<std::thread> m_threadpool;
    std::thread m_uiThread;

    struct sockaddr_in server;

    const std::string m_ip{ "127.0.0.1"} ;
    const int m_port{ 8080 };

    int m_sock;
    std::string m_exitReason{ "None" };

    NcursesUI m_ui;
    std::mutex m_netMutex;
private:
    void ClientLoop();
    void CreateSocket();
    void CloseSocket();
    void UIUpdateLoop();
    void ListenForBroadcast(std::string username);
};
