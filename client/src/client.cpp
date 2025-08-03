#include "headers/client.hpp"
#include "debug.hpp"
#include "common.hpp"

#include <chrono>
#include <iostream>
#include <ncurses.h>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

Client::Client(std::string ip, int port)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_debugger("log.txt")
    , m_ui(m_debugger)
{}

Client::~Client() = default;

void Client::Start()
{
    if (m_running)
        return;


    try
    {
        if (!CreateSession())
        {
            std::cout << "Failed to create session, stopping\n";
            Stop();
        }
        
        m_debugger.Start();

        m_uiActive = true;
        m_ui.Init();
        
        m_debugger.Log("[*] Client started...");
        // Username

        auto usernameOpt{ m_ui.PromptInput("Username: ") };

        m_debugger.Log("Username prompted");

        if (!usernameOpt.has_value() || usernameOpt->empty() || usernameOpt->length() > MAX_USERNAME_LEN)
        {
            std::string logStr{ "Invalid username " + *usernameOpt };
            m_debugger.Log(logStr.c_str());
            throw std::runtime_error("Invalid username");
        }

        const std::string username{ *usernameOpt };
        
        m_debugger.Log(username.c_str());

        if (!m_session->SendPacket(username))
        {
            std::string logStr{ "Invalid username " + username };
            m_debugger.Log(logStr.c_str());
            throw std::runtime_error("Failed to send username");
        }

        m_username = username;
        m_running = true;

        m_debugger.Log("Initializing threads...");

        // Threadhandling
        
        auto safe = [&](auto fn){
            return std::thread([this, fn](){
                try{
                    (this->*fn)();
                } catch (const std::exception& e)
                {
                    m_debugger.Log((std::string("Exception in thread: ") + e.what()).c_str());
                    Stop();
                } catch (...)
                {
                    m_debugger.Log("Unknown exception in thread, stopping");
                    Stop();
                }
            }
            );
        };

        m_threadPool.emplace_back(safe(&Client::HandleBroadcast));
        m_threadPool.emplace_back(safe(&Client::ClientLoop));
        m_threadPool.emplace_back(safe(&Client::UIUpdateLoop));
        
        m_debugger.Log("Threads started");

        for (auto &thr : m_threadPool)
        {
            if (thr.joinable())
                thr.join();
        }

    } catch (const std::runtime_error& e)
    {
        m_debugger.Log(e.what());
        m_exitReason = e.what();
        Stop();
    }
}

void Client::Stop()
{
    if (m_stopping)
    {
        return;
    }

    m_stopping = true;
    m_running = false;
    m_uiActive = false;

    m_debugger.Log("[!] Client stopped");
 
    m_debugger.Stop();
    m_ui.Cleanup();
    CloseSession();


    if (m_exitReason != "None")
    {
        std::cout << "Exited: " << m_exitReason << '\n';
    }

    exit((m_exitReason == "None") ? 0 : 1);
}

bool Client::CreateSession()
{
    try
    {
        int fd{ socket(AF_INET, SOCK_STREAM, 0) };

        if (fd < 0)
        {
            m_debugger.Log("Socket error");
            throw std::runtime_error("Socket creation failed");
            return false;
        }

        // Connect to the server
        sockaddr_in addr{};

        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port);

        if (inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr) <= 0)
        {
            std::string logStr{ "Invalid IP address: " + m_ip };
            m_debugger.Log(logStr.c_str());
            throw std::runtime_error(logStr);    
        }

        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0)
        {
            m_debugger.Log("Connection failed");   
            throw std::runtime_error("Connection failed");
        }

        m_session = std::make_unique<NetworkSession>(fd);
        std::cout << "Session created!\n";
        return true;
    } catch (const std::runtime_error& e)
    {
        m_exitReason = e.what();
        return false;
    }
}

void Client::CloseSession()
{
    if (m_session)
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

    m_debugger.Log("Exited client loop, stopping");
    Stop();
}

void Client::UIUpdateLoop()
{
    while (m_running)
    {
        m_ui.PrintBufferedMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    m_debugger.Log("Exited UI loop");
}

void Client::HandleBroadcast()
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
            std::string logStr{ "Username " + m_username + " is already taken" };
            m_debugger.Log(logStr.c_str());
            m_exitReason = "Username already taken";
            Stop();
            break;
        }

        m_ui.PushMessage(msg);
    }

    m_debugger.Log("Exited broadcast loop");
}
