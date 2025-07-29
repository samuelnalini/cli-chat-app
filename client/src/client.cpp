#include "headers/client.hpp"
#include "../../common/common.hpp"

#include <chrono>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>

void Client::ListenForBroadcast(std::string username)
{
    std::string receivedMessage;
    while (running && uiActive)
    {
        if (!recvMessage(m_sock, receivedMessage))
        {
            m_exitReason = "Connection closed by server.";
            running = false;
            break;
        }      

        if (receivedMessage == "SERVER::USERNAME_TAKEN")
        {
            m_exitReason = "Invalid username: already taken";
            running = false;
            break;
        }
       
        if (running && uiActive) {
            m_ui.PushMessage(receivedMessage);
        }
    }
}

void Client::ClientLoop()
{
    auto username = m_ui.PromptInput("Username: ");

    if (!username.has_value())
    {
        m_exitReason = "Invalid username";
        return;
    }

    if (username.value().empty() || username.value().length() > MAX_USERNAME_LEN)
    {
        m_exitReason = "Invalid username";
        return;
    }
  
    if (!sendMessage(m_sock, username.value()))
    {
        m_exitReason = "Failed to send username to server";
        return;
    }

    m_threadpool.emplace_back(&Client::ListenForBroadcast, this, username.value());

    const std::string prompt = username.value() + "> ";

    std::string inputBuffer;

    while (running)
    {
        m_ui.RedrawInputLine(prompt, inputBuffer);

        int ch;

        if (!m_ui.GetInputChar(ch))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (ch == '\n')
        {
            if (!running)
                break;

            if (inputBuffer.empty())
                continue;

            if (inputBuffer == "/exit")
            {
                shutdown(m_sock, SHUT_RD);
                m_exitReason = "Client exited";
                
                running = false;
                
                break;
            }

            if ((inputBuffer.length() > MAX_MESSAGE_LEN) && running)
                m_ui.PushMessage("Message too long.");
            else
            {
                if (!sendMessage(m_sock, inputBuffer))
                {
                    m_ui.PushMessage("Error sending message");
                }
            }

            inputBuffer.clear();
        }
        else if ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && running)
            inputBuffer.pop_back();
        else if (isprint(ch) && running)
            inputBuffer.push_back((char)ch);
    }
}

void Client::CreateSocket()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (m_sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(m_port);

    if (inet_pton(AF_INET, m_ip.c_str(), &serverAddress.sin_addr) <= 0)
    {
        perror("Invalid IP address");
        exit(1);
    }

    if (connect(m_sock, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof serverAddress) < 0)
    {
        perror("Connection error");
        exit(1);
    }
    
    std::cout << "Connected to " << m_ip << ':' << m_port << '\n';
}

void Client::UIUpdateLoop()
{
    while (running && uiActive)
    {
        m_ui.PrintBufferedMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Client::CloseSocket()
{
    if (m_sock >= 0)
    {
        shutdown(m_sock, SHUT_RDWR);
        close(m_sock);
        m_sock = -1;
    }
}

void Client::Start()
{
    if (running)
        return;

    running = true;
    m_ui.Init();
    uiActive = true;

    CreateSocket();

    m_uiThread = std::thread(&Client::UIUpdateLoop, this);

    ClientLoop();

    running = false;
    uiActive = false;

    for (auto& t : m_threadpool)
    {
        if (t.joinable())
            t.join();
    }

    if (m_uiThread.joinable()) {
        m_uiThread.join();
    }


    m_ui.Cleanup();
    CloseSocket();
    if (m_exitReason != "None")
        std::cout << "Exited: " << m_exitReason << '\n';
}
