#include "headers/server.hpp"
#include "debug.hpp"

#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
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
