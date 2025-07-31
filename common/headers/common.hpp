#pragma once

#include <unistd.h>
#include <string>

const int MAX_MESSAGE_LEN{ 4096 };
const int MAX_USERNAME_LEN{ 24 };

ssize_t recv_all(int fd, char* buf, size_t bufSize, int flags);
ssize_t send_all(int fd, char* buf, size_t bufSize, int flags);
bool sendMessage(int fd, const std::string& msg);
bool recvMessage(int fd, std::string& out);
