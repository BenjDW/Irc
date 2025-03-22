#include "../include/irc.hpp"

Serv::Serv() : port(0) {}

Serv::Serv(const int _port, const std::string _password) : port(_port), password(_password) {}

Serv::Serv(const Serv &origin) : port(origin.getPort()), password(origin.getPassword()) {}

Serv::~Serv() {}

Serv    &Serv::operator=(const Serv &origin)
{
    port = origin.getPort();
    return *this;
}

int Serv::getPort() const
{
    return port;
}

std::string Serv::getPassword() const
{
    return password;
}

int Serv::getSocket() const
{
    return fd;
}

bool Serv::isRunning() const
{
    return running;
}

bool Serv::start()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!fd)
    {
        std::cerr << "Error: Server: Socket failed." << std::endl;
        return false;
    }

    int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
		std::cerr << "Error: Server: Setsockopt failed." << std::endl;
		close(fd);
		return false;
	}

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Error: Server: Bind failed." << std::endl;
        close(fd);
        return false;
    }

    if (listen(fd, 3) < 0)
    {
        std::cerr << "Error: Server: Listen failed." << std::endl;
        close(fd);
        return false;
    }
    running = true;
    std::cout << "Server running on port " << port << "." << std::endl;
    return true;
}

void Serv::stop()
{
    running = false;
}