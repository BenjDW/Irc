#include "irc.hpp"

Serv::Serv() : port(0) {}

Serv::Serv(const int _port, const std::string _password) : port(_port), password(_password) {}

Serv::Serv(const Serv &origin) : port(origin.getPort()), password(origin.getPassword()) {}

Serv::~Serv() {}

Serv    &Serv::operator=(const Serv &origin)
{
    port = origin.getPort();
}

int Serv::getPort()
{
    return port;
}

std::string Serv::getPassword()
{
    return password;
}

bool Serv::isRunning()
{
    return running;
}