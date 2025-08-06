#include "headers/client.hpp"
#include "debug.hpp"
#include "common.hpp"

#include <chrono>
#include <iostream>
#include <ncurses.h>
#include <sodium/crypto_box.h>
#include <sodium/crypto_secretbox.h>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sodium.h>

Client::Client(std::string ip, int port)
    : m_ip(std::move(ip))
    , m_port(port)
    , m_ui()
{}

Client::~Client()
{
    if (m_running)
    {
        m_exitReason = "Abrupt exit -> attempting graceful shutdown";
        Stop(true);
    }
};

void Client::Start()
{
    if (m_running)
        return;

    try
    {
        if (!CreateSession())
        {
            Debug::Log("Failed to create session -> stopping", Debug::LOG_LEVEL::ERROR);
            std::cout << "Failed to create session, stopping\n";
            exit(1);
        }
       
        m_uiActive = true;
        m_ui.Init();
       
        Debug::Log("[*] Client started...");

        // Encryption Handshake

        crypto_box_keypair(m_client_pk, m_client_sk);

        // Send PK
        m_session->SendPacket(std::string((char*) m_client_pk, crypto_box_PUBLICKEYBYTES));

        // Receive server PK
        auto srvpkPkt{ m_session->RecvPacket().value() };
        memcpy(m_server_pk, srvpkPkt.data(), crypto_box_PUBLICKEYBYTES);

        // Receive encrypted group key
        auto grpPkt{ m_session->RecvPacket().value() };
        
        const unsigned char* nonce{ (unsigned char*) grpPkt.data() };
        const unsigned char* ct{ nonce + crypto_box_NONCEBYTES };
        size_t ctLen{ grpPkt.size() - crypto_box_NONCEBYTES };

        if (crypto_box_open_easy(
            m_group_key,
            ct,
            ctLen,
            nonce,
            m_server_pk,
            m_client_sk
        ) != 0)
        {
            throw std::runtime_error("Failed to decrypt group key");
        }

        // Username

        auto usernameOpt{ m_ui.PromptInput("Username: ") };

        if (!usernameOpt.has_value() || usernameOpt->empty() || usernameOpt->length() > MAX_USERNAME_LEN)
        {
            std::string logStr{ "Invalid username " + *usernameOpt };
            throw std::runtime_error("Invalid username");
        }

        const std::string username{ *usernameOpt };
        
        if (!SendEncrypted(username))
        {
            std::string logStr{ "Invalid username " + username };
            throw std::runtime_error("Failed to send username");
        }

        m_username = username;
        m_running = true;

    } catch (const std::exception& e)
    {
        m_exitReason = e.what();
        Debug::Log(std::string("Startup exception: ") + e.what());
        if (m_uiActive)
            m_ui.Cleanup();

        CloseSession();

        Debug::DumpToFile("log.txt");
        std::cerr << "Error: " << e.what() << '\n';
        std::exit(1);

    }

    Debug::Log("Initializing threads...");

    // Threadhandling

    auto safe = [&](auto fn){
        return std::thread([this, fn](){
                try{
                    (this->*fn)();
                } catch (const std::exception& e)
                {
                    Debug::Log((std::string("Exception in thread: ") + e.what()).c_str());
                    Stop(true);
                } catch (...)
                {
                    Debug::Log("Unknown exception in thread, stopping");
                    Stop(true);
                }
                }
                );
    };

    m_threadPool.emplace_back(safe(&Client::HandleBroadcast));
    m_threadPool.emplace_back(safe(&Client::ClientLoop));
    m_threadPool.emplace_back(safe(&Client::UIUpdateLoop));

    Debug::Log("Threads started");

    for (auto &thr : m_threadPool)
    {
        if (thr.joinable())
            thr.join();
    }

    Debug::Log("Threads stopped");

    m_ui.Cleanup();

    Debug::Log("UI cleanup procedure completed");

    if (m_exitReason != "None")
    {

        Debug::Log("Client exited: " + m_exitReason, Debug::LOG_LEVEL::INFO);
        Debug::DumpToFile("log.txt");
        std::cout << "Exited: " << m_exitReason << '\n';
    }
}

void Client::Stop(bool dumpLog)
{
    if (m_stopping)
    {
        return;
    }

    Debug::Log("Stop called");

    m_stopping = true;
    m_running = false;
    m_ui.running = false;
    m_uiActive = false;

    CloseSession();

    Debug::Log("Session closed");
}

bool Client::CreateSession()
{
    try
    {
        int fd{ socket(AF_INET, SOCK_STREAM, 0) };

        if (fd < 0)
        {
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
            throw std::runtime_error(logStr);    
        }

        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0)
        {
            throw std::runtime_error("Connection failed");
        }

        m_session = std::make_unique<NetworkSession>(fd);
        return true;
    } catch (const std::runtime_error& e)
    {
        Debug::Log(e.what());
        m_exitReason = e.what();
        return false;
    }
}

void Client::CloseSession()
{
    if (m_session)
        m_session->CloseSession();
}

bool Client::SendEncrypted(const std::string& plaintext)
{
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    std::vector<unsigned char> cipher(crypto_secretbox_MACBYTES + plaintext.size());
    
    if (crypto_secretbox_easy(
        cipher.data(),
        (unsigned char*) plaintext.data(),
        plaintext.size(),
        nonce,
        m_group_key
    ) != 0)
    {
        // Dropped
        return false;
    }

    std::string payload;
    payload.append((char*) nonce, crypto_secretbox_NONCEBYTES);
    payload.append((char*) cipher.data(), cipher.size());
    
    return m_session->SendPacket(payload);
}

std::optional<std::string> Client::RecvDecrypted()
{
    auto encOpt{ m_session->RecvPacket() };

    if (!encOpt)
        return std::nullopt;

    auto &enc{ *encOpt };

    if (enc.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)
        return std::nullopt;

    const unsigned char* nonce{ (unsigned char*) enc.data() };
    const unsigned char* ct{ nonce + crypto_secretbox_NONCEBYTES };
    size_t ctLen{ enc.size() - crypto_secretbox_NONCEBYTES };
    size_t ptLen{ ctLen - crypto_secretbox_MACBYTES };

    std::vector<unsigned char> pt(ptLen);

    if (ctLen <= crypto_secretbox_MACBYTES)
    {
        Debug::Log("Encrypted message too short for valid decryption");
        return std::nullopt;
    }

    if (crypto_secretbox_open_easy(
        pt.data(),
        ct,
        ctLen,
        nonce,
        m_group_key
    ) != 0)
    {
        Debug::Log("Decryption failed: invalid ciphertext or tampering");
        return std::nullopt;
    }

    return std::string((char*) pt.data(), pt.size());
}

void Client::ClientLoop()
{
   const std::string prompt{ " > " };

   while (m_running)
   {
        auto inputOpt{ m_ui.PromptInput(m_username + prompt) };

        if (!inputOpt.has_value())
            break;

        std::string input{ inputOpt.value() };
        if (input.empty())
            continue;

        if (input == "/exit")
        {
            Stop();
            break;
        }

        SendEncrypted(m_username + ": " + input);
   }

   Stop();
}

void Client::UIUpdateLoop()
{
    while (m_running)
    {
        m_ui.PrintBufferedMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    Stop();
}

void Client::HandleBroadcast()
{
    while (m_running)
    {
        auto msgOpt{ RecvDecrypted() };
        if (!msgOpt.has_value())
        {
            if (!m_stopping)
            {
                m_exitReason = "Server closed connection.";
                Debug::Log("Server closed the connection");
            }
            break;
        }

        const auto& msg{ *msgOpt };
        if (msg == "SERVER::USERNAME_TAKEN")
        {
            std::string logStr{ "Username " + m_username + " is already taken" };
            m_exitReason = logStr;
            Stop();
            break;
        }

        m_ui.PushMessage(msg);
    }

    Stop();
}
