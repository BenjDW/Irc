#include "../include/irc.hpp"

Serv::Serv() : port(0) {}

Serv::Serv(const int _port, const std::string _password) : port(_port), password(_password) {}

Serv::Serv(const Serv &origin) : port(origin.getPort()), password(origin.getPassword()) {}

Serv::~Serv() 
{
	close(fd);//ferme le socket
}

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

int	Serv::get_epollfd() const
{
	return epollfd;
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

	//fcntl dois etre en premier pour mettre le sockets en non bloquant
	int flags = fcntl(fd, F_GETFL, 0);// recupere le file socket fd
	if (flags < 0)
	{
		std::cerr << "fcntl F_GETFL" << std::endl;
		close(fd);//ferme le socket quand on a fini , mis aussi dans le destructeur peu etre plus besoin du coup
		return false;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)// change le socket fd en non bloquant
	{
    	std::cerr << "fcntl F_SETFL" << std::endl;
		close(fd);
		return false;
	}

	// cree le fd epoll
	epollfd = epoll_create1(0);
    if (epollfd == -1)
	{
        std::cerr << "Error: epoll create" << std::endl;
		return false;
    }

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

	// a voir ou on dois mettre se block dans le main ou ici
	epevent.data.fd = fd;
    epevent.events = EPOLLIN; // Surveiller les connexions entrantes

	// va ajoute le socketfd a epoll et ajoute la struct event a surveille
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epevent) == -1) 
	{
        perror("epoll_ctl");
        close(fd), close(epollfd);
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