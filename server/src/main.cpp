#include "headers/server.hpp"

#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    
    if (argc < 3)
    {
        Server server;
        server.Start();
    } else
    {

        if (!std::stoi(argv[2]))
        {
            std::cerr << "Usage: server <ip> <port>\n";
            return 1;
        }

        Server server(argv[1], std::stoi(argv[2]));
        server.Start();
    }

}
