#include "headers/client.hpp"
#include "debug.hpp"

#include <chrono>
#include <iostream>
#include <ncurses.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

Client::Client(std::string ip, int port)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_debugger("log.txt")
{}

Client::~Client() 
{}

void Client::Start()
{
    if (m_running)
        return;

    m_uiActive = true;
    m_ui.Init();
    m_debugger.Start();

    try
    {
        CreateSession();

        // Username

        auto usernameOpt{ m_ui.PromptInput("Username: ") };
        if (!usernameOpt.has_value() || usernameOpt->empty() || usernameOpt->length() > MAX_USERNAME_LEN)
        {
            std::string logStr{ "Invalid username " + *usernameOpt };
            m_debugger.Log(logStr.c_str());
            throw std::runtime_error("Invalid username");
        }

        const std::string username{ *usernameOpt };

        if (!m_session->SendPacket(username))
        {
            std::string logStr{ "Invalid username " + username };
            m_debugger.Log(logStr.c_str());
            throw std::runtime_error("Failed to send username");
        }

        m_username = username;
        m_running = true;

        // Threads

        m_threadPool.emplace_back(&Client::HandleBroadcast, this, username);
        m_threadPool.emplace_back(&Client::ClientLoop, this);
        m_uiThread = std::thread(&Client::UIUpdateLoop, this);

        for (auto& thread : m_threadPool)
        {
            if (thread.joinable())
                thread.join();
        }
    } catch (const std::exception& e)
    {
        m_debugger.Log(e.what());
        m_exitReason = e.what();
    }

    // Shutdown
    
    Stop();

    if (m_exitReason != "None")
    {
        m_debugger.Log(m_exitReason.c_str());
        std::cout << "Exited: " << m_exitReason << '\n';
    }
}

void Client::Stop()
{
    if (!m_running)
    {
        m_debugger.Log("WARNING: Client is already stopped.");
        return;
    }
   
    m_debugger.Stop();
    m_running = false;
    m_uiActive = false;

    if (m_uiThread.joinable())
        m_uiThread.join();

    m_ui.Cleanup();
    CloseSession();
}

void Client::CreateSession()
{
   int fd{ socket(AF_INET, SOCK_STREAM, 0) };

   if (fd < 0)
   {
        m_debugger.Log("Socket error");
        Stop();
        perror("socket()");
        throw std::runtime_error("Socket creation failed");
   }

   // Connect to the server
   sockaddr_in addr{};

   addr.sin_family = AF_INET;
   addr.sin_port = htons(m_port);

   if (inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr) <= 0)
   {
       std::string logStr{ "Invalid IP address: " + m_ip };
       m_debugger.Log(logStr.c_str());
       Stop();
       throw std::runtime_error("Invalid IP address");
   }

   if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0)
   {
        m_debugger.Log("Connection failed");   
        perror("connect()");
        Stop();
        throw std::runtime_error("Connection failed");
   }

   m_session = std::make_unique<NetworkSession>(fd);
}

void Client::CloseSession()
{
    m_session->CloseSession();
}

void Client::ClientLoop()
{
    std::string buffer;
    const std::string prompt{ " > " };

    while (m_running)
    {
        int ch;
        
        if (m_ui.GetInputChar(ch))
        {
            if (ch == '\n' && !buffer.empty())
            {
                if (buffer == "/exit")
                {
                    m_exitReason = "Client exited";
                    Stop();
                    break;
                }

                m_session->SendPacket(buffer);
                buffer.clear();
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
            {
                if (!buffer.empty())
                    buffer.pop_back();
            } else if (isprint(ch))
            {
                buffer.push_back(static_cast<char>(ch));
            }
        }

        m_ui.RedrawInputLine(m_username + prompt, buffer);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Client::UIUpdateLoop()
{
    while (m_running)
    {
        m_ui.PrintBufferedMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Client::HandleBroadcast(const std::string& username)
{
    while (m_running)
    {
        auto msgOpt{ m_session->RecvPacket() };
        if (!msgOpt.has_value())
        {
            m_debugger.Log("Server closed connection");   
            m_exitReason = "Server closed connection.";
            Stop();
            break;
        }

        const auto& msg{ *msgOpt };
        if (msg == "SERVER::USERNAME_TAKEN")
        {
            std::string logStr{ "Username " + username + " is already taken" };
            m_debugger.Log(logStr.c_str());
            m_exitReason = "Username already taken";
            Stop();
            break;
        }

        m_ui.PushMessage(msg);
    }
}
