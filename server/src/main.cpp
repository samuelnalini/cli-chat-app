#include "headers/server.hpp"
#include "debug.hpp"

#include <string>
#include <csignal>
#include <iostream>

void SignalHandler(int signal)
{
    Debug::DumpToFile("server_log.txt");
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    if (argc < 2)
    {
        Server server;
        server.Start();
    } else
    {

        if (!std::stoi(argv[1]))
        {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }

        Server server(std::stoi(argv[1]));
        server.Start();

    }

}
