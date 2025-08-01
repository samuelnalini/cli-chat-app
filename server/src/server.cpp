#include "headers/server.hpp"

#include <arpa/inet.h>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Server::Server(std::string ip, int port)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_debugger("server_log.txt")
{}

Server::~Server()
{
    Stop();
}

void Server::Start()
{
    if (m_running)
        return;
    
    m_debugger.Start();

    m_debugger.Log("Starting server...");
    std::cout << "Starting server...\n";

    m_running = true;

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0)
    {
        m_debugger.Log("SERVER: Failed to create socket");
        perror("socket()");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr);

    if (bind(m_listenfd, (sockaddr*) &addr, sizeof addr) < 0)
    {
        perror("bind()");
        close(m_listenfd);
        exit(1);
    }

    listen(m_listenfd, 10);

    m_debugger.Log("Server started");
    std::cout << "Server started\n";
    std::cout << "Listening on " << m_ip << ':' << m_port << '\n';

    ServerLoop();
}

void Server::Stop()
{
    if (!m_running)
        return;

    m_running = false;
    m_debugger.Stop();
    close(m_listenfd);

    for (auto& thread : m_threadpool)
    {
        if (thread.joinable())
            thread.join();
    }
}

void Server::ServerLoop()
{
    while (m_running)
    {
        int clientfd{ accept(m_listenfd, nullptr, nullptr) };

        if (clientfd < 0)
        {
            if (m_running)
            {
                m_debugger.Log("SERVER: Failed to accept connection.");
                perror("accept()");
            }
            break;    
        }

        auto session{ std::make_unique<NetworkSession>(clientfd) };
        m_threadpool.emplace_back(&Server::HandleClientSession, this, std::move(session));
    }
}

void Server::HandleClientSession(std::unique_ptr<NetworkSession> session)
{
   // Receive the username
   auto usernameOpt{ session->RecvPacket() };
   if (!usernameOpt)
       return;

   const std::string username{ *usernameOpt };

   // Check username uniqueness

   {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        if (m_clientSessions.count(username))
        {
            m_debugger.Log("SERVER: Attempt to login with taken username");
            session->SendPacket("SERVER::USERNAME_TAKEN");
            return;
        }

        m_clientSessions.emplace(username, std::move(session));
   }

   {
       std::string logStr{ username + " has connected" };
       m_debugger.Log(logStr.c_str());
       std::cout << logStr << '\n';
   }
   // Client main loop

   while (m_running)
   {
        std::unique_ptr<NetworkSession>& clientSession{ m_clientSessions[username] };
        auto msgOpt{ clientSession->RecvPacket() };

        if (!msgOpt || msgOpt->empty())
            break;

        std::string broadcast{ username + ": " + *msgOpt };
        BroadcastMessage(broadcast);
   }

   // Disconnect
   {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clientSessions.erase(username);
   }

   {
       std::string logStr{ username + " has disconnected" };
       m_debugger.Log(logStr.c_str());
       std::cout << logStr << '\n';
   }
}

void Server::BroadcastMessage(const std::string& msg)
{
    std::vector<NetworkSession*> sessions;

    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        
        // Copies session pointers

        for (auto& [user, sessionPtr] : m_clientSessions)
        {
            sessions.push_back(sessionPtr.get());
        }
    }

    for (auto* sess : sessions)
    {
        if (!sess->SendPacket(msg))
        {
            m_debugger.Log("SERVER: Failed to send msg to a client.");
            std::cerr << "Failed to send msg to a client.\n";
        }
    }
}
