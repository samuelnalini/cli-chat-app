#pragma once

#include "network_session.hpp"
#include "debug.hpp"

#include <string>
#include <unistd.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>

class Server
{
public:
    Server(std::string ip = "127.0.0.1", uint16_t port = 8080);
    ~Server();

    void Start();
    void Stop();

private:
    int m_listenfd{ -1 };

    std::unordered_map<std::string, std::unique_ptr<NetworkSession>> m_clientSessions;
    std::mutex m_clientsMutex;

    std::vector<std::thread> m_threadpool;
    Debugger m_debugger;

    std::string m_ip;
    uint16_t m_port;
    bool m_running{ false };

private:
    void ServerLoop();
    void HandleClientSession(std::unique_ptr<NetworkSession> session);
    void BroadcastMessage(const std::string& msg);
};
