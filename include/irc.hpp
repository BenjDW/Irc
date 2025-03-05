#pragma once

#include <iostream>

#include <stdlib.h>

class Serv
{
    private:
        int   port;
        const std::string   password;
        bool    running;

    public:
        Serv();
        Serv(const int _port, const std::string _password);
        Serv(const Serv &origin);
        ~Serv();

        Serv &operator=(const Serv &origin);

        int getPort();
        std::string getPassword();
        bool isRunning();
};