#include "../headers/network_session.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

NetworkSession::NetworkSession(int socketfd)
    : m_socketfd(socketfd)
{}

NetworkSession::~NetworkSession() = default;

bool NetworkSession::SendPacket(const std::string& data)
{
    uint32_t len{ static_cast<uint32_t>(data.size()) };
    uint32_t netLen{ htonl(len) };

    std::string buffer(sizeof netLen + data.size(), '\0');

    memcpy(buffer.data(), &netLen, sizeof netLen);
    memcpy(buffer.data() + sizeof netLen, data.data(), data.size());

    if (send_all(m_socketfd, buffer.data(), buffer.size(), 0) < 0)
        return false;

    /*
    // Length header
    if (send_all(m_socketfd, reinterpret_cast<char*>(&netLen), sizeof netLen, 0) < 0)
    {
        return false;
    }

    // Data payload
    if (send_all(m_socketfd, const_cast<char*>(data.data()), data.size(), 0) < 0)
    {
        return false;
    }

    */

    return true;
}

std::optional<std::string> NetworkSession::RecvPacket()
{
    // Length header
    uint32_t netLen{ 0 };
    ssize_t received{ recv_all(m_socketfd, reinterpret_cast<char*>(&netLen), sizeof netLen, 0) };

    if (received == 0)
    {
        // Connection closed
        return std::nullopt;
    }

    if (received < 0)
    {
        // Error
        return std::nullopt;
    }

    // Payload
    
    uint32_t len{ ntohl(netLen) };
    if (len == 0)
    {
        // Empty payload
        return std::string{};
    }

    std::string buffer(len, '\0');
    received = recv_all(m_socketfd, buffer.data(), len, 0);
    if (received <= 0)
    {
        return std::nullopt;
    }

    return buffer;
}

void NetworkSession::CloseSession()
{
    if (m_socketfd >= 0)
    {
        shutdown(m_socketfd, SHUT_RDWR);
        close(m_socketfd);
        m_socketfd = -1;
    }
}

int NetworkSession::GetSocket() const
{
    return m_socketfd;
}

ssize_t recv_all(int fd, char* buf, size_t bufSize, int flags)
{
    size_t totalReceived{ 0 };

    while (totalReceived < bufSize)
    {
        ssize_t bytes{ recv(fd, buf + totalReceived, bufSize - totalReceived, flags) };
        
        if (bytes == 0)
        {
            break;
        }

        if (bytes < 0)
        {
            perror("recv error");
            return -1;
        }

        totalReceived += bytes;
    }

    return totalReceived;
}

ssize_t send_all(int fd, char* buf, size_t bufSize, int flags)
{
    size_t totalSent{ 0 };

    while (totalSent < bufSize)
    {
        ssize_t bytes{ send(fd, buf + totalSent, bufSize - totalSent, flags) };

        if (bytes <= 0)
        {
            if (bytes < 0)
                perror("send error");

            return -1;
        }

        totalSent += bytes;
    }

    return totalSent;
}
