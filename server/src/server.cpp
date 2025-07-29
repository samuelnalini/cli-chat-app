#include "headers/server.hpp"
#include "../../common/common.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory.h>
#include <string>
#include <thread>
#include <cstring>

void Server::BroadcastMessage(const char *msg)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (const auto& conn : m_clientpool)
    {
        if (!sendMessage(conn.second, msg))
            std::cerr << "Failed to send message to client: " << conn.first << '\n';
    }   
}

void Server::HandleClient(int clientSock)
{
    std::string username;
   
    if (!recvMessage(clientSock, username))
        std::cerr << "Failed to receive username from client\n";
    
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        if (m_clientpool.find(username) != m_clientpool.end())
        {
            sendMessage(clientSock, "SERVER::USERNAME_TAKEN");
            close(clientSock);
            return;
        }

        m_clientpool[username] = clientSock;
    }

    std::cout << username << " has connected.\n";

    std::string message;
    while (running)
    {
        message.clear();

        if (!recvMessage(clientSock, message))
            break;
            
        if (message.empty())
        { 
            break;
        }

        std::string bcMsg{ username + ": " + message };
        BroadcastMessage(bcMsg.c_str());
    }

    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clientpool.erase(username);
    }

    std::cout << username << " has disconnected.\n";
    close(clientSock);
}

void Server::ServerLoop()
{
    while (running)
    {

        int clientSock = accept(m_sock, nullptr, nullptr);

        if (clientSock < 0)
        {
            if (running)
                perror("Accept failed");
        }

        m_threadpool.emplace_back(&Server::HandleClient, this, clientSock);
    }

    std::cout << "Connection closed\n"; 
    BroadcastMessage("SERVER::CLOSE");

    close(m_sock);
}

void Server::CreateSocket()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (m_sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(m_port);

    if (inet_pton(AF_INET, m_ip.c_str(), &server.sin_addr) <= 0)
    {
        std::cerr << "Invalid IP address: " << m_ip << '\n';
        exit(1);
    }

    if (bind(m_sock, (struct sockaddr *)&server, sizeof server) < 0)
    {
        perror("Bind failed");
        close(m_sock);
        exit(1);
    }
 

    if (listen(m_sock, 10) < 0)
    {
       perror("Listen failed");
       close(m_sock);
       exit(1);
    }

    std::cout << "Server listening on " << m_ip << ':' << m_port << '\n';
}

void Server::Start()
{
    if (running)
        return;

    std::cout << "Starting server...\n";
    running = true;
    

    CreateSocket(); 
    ServerLoop();

    for (auto& thread : m_threadpool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
