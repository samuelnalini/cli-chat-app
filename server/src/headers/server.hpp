#pragma once

#include "network_session.hpp"

#include <string>
#include <unistd.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>

class Server
{
public:
    Server(std::string ip = "127.0.0.1", int port = 8080);
    ~Server();

    void Start();
    void Stop();

    const int MAX_MESSAGE_LEN{ 4096 };
    const int MAX_USERNAME_LEN{ 24 };

private:
    int m_listenfd{ -1 };

    std::unordered_map<std::string, std::unique_ptr<NetworkSession>> m_clientSessions;
    std::mutex m_clientsMutex;

    std::vector<std::thread> m_threadpool;

    std::string m_ip;
    int m_port;
    bool m_running{ false };

private:
    void ServerLoop();
    void HandleClientSession(std::unique_ptr<NetworkSession> session);
    void BroadcastMessage(const std::string& msg);
};
