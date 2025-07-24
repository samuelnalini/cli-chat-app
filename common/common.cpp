#include "common.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream> 

bool sendMessage(int fd, const std::string& msg)
{
    uint32_t msgLen{ htonl(msg.length()) };
    
    // Send the length of the message
    if (send_all(fd, reinterpret_cast<char *>(&msgLen), sizeof msgLen, 0) < 0)
        return false;

    // Send the message
    if (send_all(fd, const_cast<char *>(msg.data()), msg.size(), 0) < 0)
        return false;

    return true;
}

bool recvMessage(int fd, std::string& out)
{
    uint32_t outLen{ 0 };

    // receive the length

    if (recv_all(fd, reinterpret_cast<char *>(&outLen), sizeof outLen, 0) < 0)
        return false;

    outLen = ntohl(outLen);
    
    if (outLen <= 0)
    {
        return false;
    }

    // receive the string

    std::string buffer(outLen, '\0');
    if (recv_all(fd, buffer.data(), outLen, 0) < 0)
        return false;

    out = std::move(buffer);
    return true;
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
