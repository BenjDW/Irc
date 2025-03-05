#include "../include/irc.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "Error: Usage: ./ircserv [port] [password]." << std::endl;
        return (-1);
    }
    for (int i = 0, av[1][i] != '\0'; i++)
    {
        if (av[1][i] < '0' || av[1][i] > '9')
        {
            std::cerr << "Error: Unvalid port." << std::endl;
            return (-1);
        }
    }

    Serv    server(atoi(av[1]), av[2]);
    if (server.getPort() < 1)
    {
        std::cerr << "Error: port number too low." << std::endl;
        return (-1);
    }
    if (server.getPort() > 65535)
    {
        std::cerr << "Error: port number too large." << std::endl;
        return (-1);
    }
    if (server.start() < 0)
        return (-1);

    while (server.isRunning())
    {
        //loop a faire
    }
}