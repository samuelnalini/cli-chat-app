#pragma once

#include "network_session.hpp"
#include "debug.hpp"

#include <string>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>

class Server
{
public:
    Server(uint16_t port = 8080);
    ~Server();

    void Start();
    void Stop();

private:
    struct ClientInfo
    {
        std::unique_ptr<NetworkSession> session;
        std::string username;
        bool registered{ false };
    };

    std::unordered_map<int, ClientInfo> m_clients;
    std::unordered_set<std::string> m_usernames;
    std::mutex m_clientsMutex;

    int m_listenfd{ -1 };
    int m_epollfd{ -1 };

    uint16_t m_port;
    Debugger m_debugger;

    bool m_running{ false };
private:
    void SetupListener();
    void SetNonBlocking(int fd);
    void EventLoop();
    void HandleNewConnection();
    void HandleClientEvent(int fd, uint32_t events);
    void BroadcastMessage(const std::string& msg);
};
