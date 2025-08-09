#include "headers/server.hpp"
#include "style.hpp"
#include "debug.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <mutex>
#include <sodium/crypto_box.h>
#include <sodium/crypto_secretbox.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <vector>

constexpr int MAX_EXENTS{ 64 };

Server::Server(const uint16_t port)
    : m_port(port)
{
    if (sodium_init() < 0)
    {
        std::cerr << "Unable to initialize libsodium\n";
        exit(1);
    }

    crypto_box_keypair(m_server_pk, m_server_sk);
    randombytes_buf(m_group_key, sizeof m_group_key);
}

Server::~Server()
{
    Stop(true);
}

void Server::SetNonBlocking(int fd)
{
    int flags{ fcntl(fd, F_GETFL, 0) };
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::SetupListener()
{

    std::cout << "==> Creating socket... ";
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0)
    {
        Debug::Log("Failed to create socket", Debug::LOG_LEVEL::ERROR);
        perror("socket()");
        Stop(true);
    }

    std::cout << Style::style("PASS\n", {Style::STYLE_TYPE::GREEN, Style::STYLE_TYPE::BOLD});

    int opt{ 1 };
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;


    std::cout << "==> Binding... ";
    if (bind(m_listenfd, (sockaddr*) &addr, sizeof addr) < 0)
    {
        Debug::Log("Failed to bind socket", Debug::LOG_LEVEL::ERROR);
        perror("bind()");
        close(m_listenfd);
        Stop(true);
    }

    if (listen(m_listenfd, SOMAXCONN) < 0)
    {
        perror("listen()");
        Stop(true);
    }
    
    std::cout << Style::style("PASS\n", {Style::STYLE_TYPE::GREEN, Style::STYLE_TYPE::BOLD});

    SetNonBlocking(m_listenfd);


    std::cout << Style::style("STARTING EVENT HANDLER\n", {Style::STYLE_TYPE::RED});

    std::cout << "==> Setting up epoll... ";
    m_epollfd = epoll_create1(0);
    if (m_epollfd < 0)
    {
        Debug::Log("epoll() error", Debug::LOG_LEVEL::ERROR);
        perror("epoll_create1()");
        Stop(true);
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_listenfd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &ev);

    std::cout << Style::style("PASS\n", {Style::STYLE_TYPE::GREEN, Style::STYLE_TYPE::BOLD});
}

void Server::Start()
{
    if (m_running)
        return;
    
    Debug::Log("Starting server...");
    std::cout << Style::style("STARTING SERVER\n", {Style::STYLE_TYPE::RED});
    m_running = true;
    SetupListener();
    EventLoop();
}

void Server::Stop(bool dumpLog)
{
    if (!m_running)
        return;


    Debug::Log("Stopping server...");
    std::cout << Style::style("STOPPING SERVER\n", {Style::STYLE_TYPE::BRIGHT_RED, Style::STYLE_TYPE::BOLD});
    m_running = false;

    if (m_listenfd != -1)
    {
        shutdown(m_listenfd, SHUT_RDWR);
        close(m_listenfd);
        m_listenfd = -1;
        Debug::Log("Client sock: closed");
        std::cout << "Client sock: closed\n";
    }

    if (m_epollfd != -1)
    {
        shutdown(m_epollfd, SHUT_RDWR);
        close(m_epollfd);
        m_epollfd = -1;
        Debug::Log("Event handler: stopped");
        std::cout << "Event handler: stopped\n";
    }

    m_clients.clear();
    m_usernames.clear();

    if (dumpLog)
        Debug::DumpToFile("serverlog.txt");
}

void Server::EventLoop()
{
    epoll_event events[MAX_EXENTS];
   
    std::cout << Style::style("SERVER STARTED\n", {Style::STYLE_TYPE::GREEN, Style::STYLE_TYPE::BOLD});
    std::cout << "RUNNING ON PORT ";
    std::cout << Style::style(std::to_string(m_port) + '\n', {Style::STYLE_TYPE::BOLD});

    std::cout << Style::style("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", {Style::STYLE_TYPE::STRIKETHROUGH});

    while (m_running)
    {
        int n{ epoll_wait(m_epollfd, events, MAX_EXENTS, -1) };

        if (n < 0)
        {
            Debug::Log("epoll_wait() error", Debug::LOG_LEVEL::ERROR);
            perror("epoll_wait()");
            Stop(true);
            break;
        }

        for (int i{ 0 }; i < n; i++)
        {
            int fd{ events[i].data.fd };

            if (fd == m_listenfd)
                HandleNewConnection();
            else
            {
                auto it = m_clients.find(fd);

                if (it == m_clients.end())
                    return;

                HandleClientEvent(it->second, events[i].events);
            }
        }
    }
}

void Server::HandleNewConnection()
{
    while (m_running)
    {
        int clientfd = accept(m_listenfd, nullptr, nullptr);

        if (clientfd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                Debug::Log("accept() error", Debug::LOG_LEVEL::ERROR);
                perror("accept()");
                break;
            }
        }

        epoll_event ev{ EPOLLIN, {
            .fd = clientfd
        } };

        if( epoll_ctl(m_epollfd, EPOLL_CTL_ADD, clientfd, &ev) < 0)
        {
            Debug::Log("epoll_ctl() error", Debug::LOG_LEVEL::ERROR);
            perror("epoll_ctl(): add client");
            close(clientfd);
            continue;
        }

        ClientInfo info;
        info.session = std::make_unique<NetworkSession>(clientfd);
        info.fd = clientfd;
        m_clients.emplace(clientfd, std::move(info));
    }
}

void Server::DisconnectClient(ClientInfo& client)
{
    std::unique_lock<std::mutex> lock(m_clientsMutex);

    auto it{ m_clients.find(client.fd) };

    if (it == m_clients.end())
        return;

    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, client.fd, nullptr);
    client.session->CloseSession();

    std::string user{ client.username };
    bool was_registered = client.registered;

    if (was_registered)
    {
        m_usernames.erase(client.username);
    }

    m_clients.erase(it);
    lock.unlock();

    if (was_registered)
    {
        BroadcastEncrypted(user + " has disconnected");
        std::cout << user + " has " + Style::red("disconnected\n");
    }
}

void Server::HandleClientEvent(ClientInfo& client, uint32_t events)
{
    auto it{ m_clients.find(client.fd) };

    if (it == m_clients.end())
        return;

    // Key exchange

    if (!client.key_exchanged)
    {
        // Receive client public key
        auto pubkeyPkt{ client.session->RecvPacket() };

        if (!pubkeyPkt)
        {
            Debug::Log("Invalid public key", Debug::LOG_LEVEL::ERROR);
            return;
        }

        if (pubkeyPkt->size() != crypto_box_PUBLICKEYBYTES)
        {
            Debug::Log("Public key has an invalid size", Debug::LOG_LEVEL::ERROR);
            return;
        }

        memcpy(client.client_pk, pubkeyPkt->data(), crypto_box_PUBLICKEYBYTES);
        
        // Send server public key
        std::string srvpk((char*) m_server_pk, crypto_box_PUBLICKEYBYTES);
        client.session->SendPacket(srvpk);

        // Encrypt the group key w/ client's pk
        unsigned char nonce[crypto_box_NONCEBYTES];
        randombytes_buf(nonce, sizeof nonce);

        std::vector<unsigned char> cipher(crypto_box_MACBYTES + sizeof m_group_key);

        if (crypto_box_easy(cipher.data(),
            m_group_key,
            sizeof m_group_key,
            nonce,
            client.client_pk,
            m_server_sk
        ) != 0)
        {
            // Bad handshake
            Debug::Log("Bad handshake -> Disconnecting client", Debug::LOG_LEVEL::ERROR);
            DisconnectClient(client);
            return;
        }

        std::string payload;
        payload.append((char*) nonce, crypto_box_NONCEBYTES);
        payload.append((char*) cipher.data(), cipher.size());
        client.session->SendPacket(payload);

        client.key_exchanged = true;
        return;
    }


    // Receive the next packet as raw encrypted blob

    auto rawPkt{ client.session->RecvPacket() };
    if (!rawPkt)
    {
        DisconnectClient(client);
        return;
    }

    // Username

    if (!client.registered)
    {
        if (rawPkt->size() < crypto_secretbox_NONCEBYTES)
        {
            Debug::Log("Username packet too small -> Disconnecting client", Debug::LOG_LEVEL::WARNING);
            DisconnectClient(client);
            return;
        }

        const unsigned char* np{ (unsigned char*) rawPkt->data() };
        const unsigned char* ct{ np + crypto_secretbox_NONCEBYTES };
        size_t ctLen{ rawPkt->size() - crypto_secretbox_NONCEBYTES };
        std::vector<unsigned char> pt(ctLen - crypto_secretbox_MACBYTES);

        if (crypto_secretbox_open_easy(
            pt.data(),
            ct,
            ctLen,
            np,
            m_group_key
        ) != 0)
        {
            // Bad username packet
            Debug::Log("Invalid username packet -> Disconnecting client", Debug::LOG_LEVEL::WARNING);
            DisconnectClient(client);
            return;
        }

        std::string uname{ (char*) pt.data(), pt.size() };

        // Enforce uniqueness

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);

            if (m_usernames.count(uname))
            {
                Debug::Log("Client attempted to login with a username that is already taken. Disconnecting", Debug::LOG_LEVEL::INFO);
                SendSecretbox(client.session.get(), "SERVER::USERNAME_TAKEN");
                DisconnectClient(client);
                return;
            }
            m_usernames.insert(uname);
        }

        client.username = std::move(uname);
        client.registered = true;

        BroadcastEncrypted(client.username + " has connected");
        std::cout << client.username + " has " + Style::green("connected\n");
        return;
    }

    // Forward the packet
    
    for (auto& [otherFd, otherClient] : m_clients)
    {
        if (!otherClient.registered)
            continue;

        otherClient.session->SendPacket(*rawPkt);
    }
}


bool Server::SendSecretbox(NetworkSession* sess, const std::string& msg)
{
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    std::vector<unsigned char> cipher(crypto_secretbox_MACBYTES + msg.size());

    if (crypto_secretbox_easy(
        cipher.data(),
        (unsigned char*) msg.data(),
        msg.size(),
        nonce,
        m_group_key
    ) != 0)
    {
        Debug::Log("Failed to encrypt packet -> Dropping", Debug::LOG_LEVEL::WARNING);
        return false;
    }

    std::string payload;
    payload.append((char*) nonce, crypto_secretbox_NONCEBYTES);
    payload.append((char*) cipher.data(), cipher.size());
    if (!sess->SendPacket(payload))
    {
        Debug::Log("Failed to send message", Debug::LOG_LEVEL::WARNING);
        return false;
    }

    return true;
}

void Server::BroadcastEncrypted(const std::string& msg)
{
    std::vector<int> removeList;

    for (auto& [fd, client] : m_clients)
    {
        if (!client.registered)
            continue;
        
        if (!SendSecretbox(client.session.get(), msg))
            removeList.push_back(fd);
    }

    for (int fd : removeList)
    {
        auto it = m_clients.find(fd);

        if (it == m_clients.end())
            continue;

        DisconnectClient(it->second);
    }
}
