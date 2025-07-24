#include "headers/client.hpp"
#include "../../common/common.hpp"

#include <string>

std::string getInput(const char* msg)
{
    std::string input;
    std::cout << msg;
    std::getline(std::cin, input);

    return input;
}

void Client::ListenForBroadcast(std::string username)
{
    std::string receivedMessage;
    while (running)
    {
        if (!recvMessage(m_sock, receivedMessage))
        {
            std::cout << "Server closed the connection.\n";
            running = false;
            break;
        }
        
        if (receivedMessage == "SERVER::USERNAME_TAKEN")
        {
            std::cout << "Invalid username: Name taken\n";
            running = false;
            break;
        }

        std::cout << '\n' << receivedMessage << '\n';
        std::cout << username << "> " << std::flush;
    }
}

void Client::ClientLoop()
{  
    std::string username{ getInput("Username: ") };
    const std::string messageInputDisplay{ username + "> " };

    if (username.empty() || username.length() > MAX_USERNAME_LEN)
    {
        std::cerr << "Invalid username";
        exit(1);
    }
    
    if (!sendMessage(m_sock, username))
        perror("Username error");

    m_threadpool.emplace_back(&Client::ListenForBroadcast, this, username);

    while (running)
    {
        
        std::string input{ getInput(messageInputDisplay.c_str()) };
        
        if (input.empty())
            continue;

        if (input == "/exit")
        {
            running = false;
            break;
        }

        if (input.length() > MAX_MESSAGE_LEN)
        {
            std::cout << "Message too long.\n";
            continue;
        }
        
        if (!sendMessage(m_sock, input))
            perror("Message error");
    }
   
    std::cout << "Connection closed.\n";
    close(m_sock);
}

void Client::CreateSocket()
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(m_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(m_sock, (struct sockaddr *)&serverAddress, sizeof serverAddress) > 0)
    {
        perror("Connection error");
        exit(1);
    }
    
    std::cout << "Connected\n";
}

void Client::Start()
{
    if (running) return;
    std::cout << "Starting client...\n";
    running = true;

    CreateSocket();
    ClientLoop();
}
