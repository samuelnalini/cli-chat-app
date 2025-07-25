#include "headers/client.hpp"

#include <string>
#include <iostream>

int main(int argc, char* argv[])
{

    if (argc < 3)
    {
        Client client;
        client.Start();
    } else
    {

        if (!std::stoi(argv[2]))
        {
            std::cerr << "Usage: client <ip> <port>";
            return 1;
        }

        Client client(argv[1], std::stoi(argv[2]));
        client.Start();
    }
}
