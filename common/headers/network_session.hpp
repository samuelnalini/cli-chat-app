/*

   network_session.hpp

   This class is meant to be a network wrapper for packet sending and receiving. It was created for further separation of concerns when adding complexity, such as encryption.

*/

#pragma once

#include <string>
#include <optional>

ssize_t send_all(int fd, char* buf, size_t bufSize, int flags);
ssize_t recv_all(int fd, char* buf, size_t bufSize, int flags);

class NetworkSession
{
public:
    explicit NetworkSession(int socketfd);
    ~NetworkSession();

    // I don't want this to be copyable
    NetworkSession(const NetworkSession&) = delete;
    NetworkSession& operator = (const NetworkSession&) = delete;

    // Send a raw packet with length-prefix framing
    bool SendPacket(const std::string& data);

    // Receive a raw packet with length-prefix framing
    std::optional<std::string> RecvPacket();

    void CloseSession();
    int GetSocket() const;

private:
    int m_socketfd;

};
