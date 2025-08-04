#include "headers/server.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <mutex>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>

constexpr int MAX_EXENTS{ 64 };

Server::Server(uint16_t port)
    : m_port(port)
    , m_debugger("server_log.txt")
{}

Server::~Server()
{
    Stop();
}

void Server::SetNonBlocking(int fd)
{
    int flags{ fcntl(fd, F_GETFL, 0) };
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::SetupListener()
{
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0)
    {
        perror("socket()");
        exit(1);
    }

    int opt{ 1 };
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listenfd, (sockaddr*) &addr, sizeof addr) < 0)
    {
        perror("bind()");
        close(m_listenfd);
        exit(1);
    }

    if (listen(m_listenfd, SOMAXCONN) < 0)
    {
        perror("listen()");
        exit(1);
    }

    SetNonBlocking(m_listenfd);

    m_epollfd = epoll_create1(0);
    if (m_epollfd < 0)
    {
        perror("epoll_create1()");
        exit(1);
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_listenfd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &ev);
    
    std::cout << "Listening on port " << m_port << '\n';
}

void Server::Start()
{
    if (m_running)
        return;
    
    std::cout << "Starting server...\n";

    m_running = true;
    SetupListener();
    EventLoop();
}

void Server::Stop()
{
    if (!m_running)
        return;

    m_running = false;

    if (m_listenfd != -1)
    {
        shutdown(m_listenfd, SHUT_RDWR);
        close(m_listenfd);
        m_listenfd = -1;
    }

    if (m_epollfd != -1)
    {
        shutdown(m_epollfd, SHUT_RDWR);
        close(m_epollfd);
        m_epollfd = -1;
    }

    m_clients.clear();
    m_usernames.clear();
}

void Server::EventLoop()
{
    epoll_event events[MAX_EXENTS];

    while (m_running)
    {
        int n{ epoll_wait(m_epollfd, events, MAX_EXENTS, -1) };

        if (n < 0)
        {
            perror("epoll_wait()");
            break;
        }

        for (int i{ 0 }; i < n; i++)
        {
            int fd{ events[i].data.fd };

            if (fd == m_listenfd)
                HandleNewConnection();
            else
                HandleClientEvent(fd, events[i].events);
        }
    }
}

void Server::HandleNewConnection()
{
    while (m_running)
    {
        int clientfd{ accept(m_listenfd, nullptr, nullptr) };

        if (clientfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                perror("accept()");
                break;
            }
        }

        epoll_event ev{ EPOLLIN, {
            .fd = clientfd
        } };

        if( epoll_ctl(m_epollfd, EPOLL_CTL_ADD, clientfd, &ev) < 0)
        {
            perror("epoll_ctl(): add client");
            close(clientfd);
            continue;
        }

        ClientInfo info;
        info.session = std::make_unique<NetworkSession>(clientfd);
        m_clients.emplace(clientfd, std::move(info));
    }
}

void Server::HandleClientEvent(int fd, uint32_t events)
{
    auto it{ m_clients.find(fd) };

    if (it == m_clients.end())
        return;

    ClientInfo &client{ it->second };

    auto dataOpt{ client.session->RecvPacket() };
    if (!dataOpt)
    {
        // Disconnect
        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr);
        client.session->CloseSession();

        if (client.registered)
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_usernames.erase(client.username);
        }

        std::string user{ client.username };
        m_clients.erase(it);
        if (client.registered)
            BroadcastMessage(user + " has disconnected");
        return;
    }

    if (!client.registered)
    {
        // Get username
        std::string uname{ *dataOpt };
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);

            if (m_usernames.count(uname))
            {
                client.session->SendPacket("SERVER::USERNAME_TAKEN");
                epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr);
                client.session->CloseSession();
                m_clients.erase(fd);
                return;
            }

            m_usernames.insert(uname);
        }

        client.username = std::move(uname);
        client.registered = true;
        BroadcastMessage(client.username + " has connected");
    } else if (!dataOpt->empty())
    {
        BroadcastMessage(client.username + ": " + *dataOpt);
    }
}


void Server::BroadcastMessage(const std::string& msg)
{
    std::vector<int> removeList;

    for (auto& [fd, client] : m_clients)
    {
        if (!client.registered)
            continue;

        if (!client.session->SendPacket(msg))
            removeList.push_back(fd);
    }

    for (int fd : removeList)
    {
        epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, nullptr);
        m_clients[fd].session->CloseSession();
        m_clients.erase(fd);
        
    }
}
