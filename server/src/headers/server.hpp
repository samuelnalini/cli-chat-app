#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>

class Server
{
public:
    Server() = default;
    Server(std::string ip, int port)
        : m_port(port)
        , m_ip(ip)
    {}

    void Start(); 

    const int MAX_MESSAGE_LEN{ 4096 };
    const int MAX_USERNAME_LEN{ 24 };
    bool running{ false };

private:
    struct sockaddr_in server;
    std::string m_ip{ "127.0.0.1" };
    int m_sock{ -1 };
    int m_port{ 8080 };
    std::vector<std::thread> m_threadpool;
    std::unordered_map<std::string, int> m_clientpool;
    std::mutex m_clientsMutex;
private:
    void ServerLoop();
    void CreateSocket();
    void HandleClient(int clientSock);
    void BroadcastMessage(const char *msg);
};
