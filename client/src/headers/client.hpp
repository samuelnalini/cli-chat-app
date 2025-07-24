#pragma once

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <thread>

class Client
{
public:
    Client() {};
    Client(int port, std::string ip)
    : m_port(port)
    , m_ip(ip)
    {}

    void Start();
public:
    bool running{ false };

private:
    std::vector<std::thread> m_threadpool;
    struct sockaddr_in server;
    const std::string m_ip{ "127.0.0.1"} ;
    const int m_port{ 8080 };
    int m_sock;
private:
    void ClientLoop();
    void CreateSocket();
    void ListenForBroadcast(std::string username);
};
