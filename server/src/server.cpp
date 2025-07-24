#include "headers/server.hpp"
#include "../../common/common.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory.h>
#include <string>
#include <thread>

void Server::BroadcastMessage(const char *msg)
{
    for (std::pair<std::string, int> conn : m_clientpool)
        sendMessage(conn.second, msg);   
}

void Server::HandleClient(int clientSock)
{
    std::string message(MAX_MESSAGE_LEN, '\0');
    std::string username(MAX_USERNAME_LEN, '\0');
    
    if (!recvMessage(clientSock, username))
        perror("Username error");
    
    if (m_clientpool.contains(username))
    {
        sendMessage(clientSock, "SERVER::USERNAME_TAKEN");
        return;
    }

    m_clientpool[username] = clientSock;
    std::cout << username << " has connected.\n";

    while (clientSock && running)
    {
        if (!recvMessage(clientSock, message))
            perror("Message error");
        
        if (message.length() <= 0)
        {
            m_clientpool.erase(username);
            std::cout << username << " has disconnected.\n";
            break;
        }
        //std::cout << username << ": " << message << '\n';
        std::string bcMsg{ username + ": " + message };
        BroadcastMessage(bcMsg.data());
    }
}

void Server::ServerLoop()
{
    while (running)
    {

        int clientSock = accept(m_sock, nullptr, nullptr);
        m_threadpool.emplace_back(&Server::HandleClient, this, clientSock);
    }

    std::cout << "Connection closed\n"; 
    BroadcastMessage("SERVER::CLOSE");

    close(m_sock);
}

void Server::CreateSocket()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &server.sin_addr);
    
    if (bind(m_sock, (struct sockaddr *)&server, sizeof server) > 0)
    {
        perror("Bind failed");
        exit(1);
    }
 
    listen(m_sock, 5);
    std::cout << "Socket created\n";
}

void Server::Start()
{
    if (running) return;
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
