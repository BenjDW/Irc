#include "../include/irc.hpp"

Serv::Serv() : port(0) {}

Serv::Serv(const int _port, const std::string _password) : port(_port), password(_password)
{
	this->commands["PASS"] = Commands::pass_command;
	this->commands["JOIN"] = Commands::join_command;
	this->commands["NICK"] = Commands::nick_command;
	this->commands["USER"] = Commands::user_command;
	this->commands["PART"] = Commands::part_command;
	this->commands["PRIVMSG"] = Commands::privmsg_command;
	this->commands["KICK"] = Commands::kick_command;
	this->commands["TOPIC"] = Commands::topic_command;
	this->commands["INVITE"] = Commands::invite_command;
	this->commands["MODE"] = Commands::mode_command;
}

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

/// GETTER ///
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

int	Serv::get_epollfd() const
{
	return epollfd;
}

epoll_event Serv::getPevent() const
{
    return epevent;
}

epoll_event Serv::getEvent(int index) const
{
    return events[index];
}

Channel *Serv::getChannel(std::string name)
{
	for (std::vector<Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++)
	{
		if ((*it)->getName() == name)
			return (*it);
	}
	return (NULL);
}

Client *Serv::getClient(std::string nickname)
{
	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
	{
		if (it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}

/*std::vector<Channel *> Serv::getChannels()
{
	return (this->channels);
}*/

/// MESSAGE PROCESSING ///
void Serv::processMessage(int user_fd, const char *message)
{
	std::string msg(message);
	std::string buffer;

	this->clients[user_fd]->appendToBuffer(msg);
	buffer = this->clients[user_fd]->getBuffer();
	if (buffer.find_first_of("\r\n") == std::string::npos)
		return;

	std::vector<std::string> commands = splitCommands(buffer);
	for (std::vector<std::string>::iterator it = commands.begin(); it != commands.end(); ++it)
		this->interpret_message(user_fd, *it);
	if (this->clients[user_fd])
		this->clients[user_fd]->clearBuffer();
}

std::vector<std::string> Serv::splitCommands(const std::string &msg)
{
	std::vector<std::string> commands;
	std::istringstream stream(msg);
	std::string command;

	while (std::getline(stream, command))
	{
		if (command.find("\r\n") == std::string::npos)
			command += "\r\n";
		commands.push_back(command);
	}

	return commands;
}

void Serv::interpret_message(int user_id, std::string const &command)
{
	Client *user = this->clients[user_id];
	std::string cmdname;

	if (!user)
	{
		std::cerr << "Error: Interpret Message function can't get the client with id [" << user_id << "]" << std::endl;
		return;
	}

	CommandFunction cmdf = NULL;
	cmdname = command.substr(0, command.find_first_of(" \r\n"));

	if (cmdname != "CAP" && cmdname != "PASS" && !user->isLogged())
	{
		std::cerr << "ERROR: Unauthorized connection, needs password!" << std::endl;
		user->sendMessage(ERR_NOTREGISTERED(user->getNickname()));
	}
	else
	{
		cmdf = this->commands[cmdname];
		if (cmdf != NULL)
			cmdf(*this, *user, command);
	}
}

void Serv::addChannel(Channel *channel)
{
	this->channels.push_back(channel);
}

void Serv::close_client_connection(int user_id, std::string reason)
{
	if (this->clients[user_id] != NULL)
	{
		for (std::vector<Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++)
		{
			(*it)->removeClient(this->clients[user_id]);
			(*it)->removeOperator(this->clients[user_id]);
		}
		delete this->clients[user_id];
		this->clients.erase(user_id);
		close(user_id);
		if (!reason.empty())
			std::cout << "Kicked User " << user_id << " because " << reason << std::endl;
	}
}

/// THREE MAIN ///
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

    std::cout << "Server running on port " << port << "." << std::endl;
    return true;
}

void Serv::loop()
{
    int nfds = epoll_wait(this->epollfd, this->events, 10, -1);
    for (int i = 0; i < nfds; ++i)
    {
        if (this->events[i].data.fd == this->fd)
        {
            sockaddr_in client;
            socklen_t cLen = sizeof(client);
            int newsockfd = accept(fd, (struct sockaddr *)&client, &cLen);
            if (newsockfd < 0)
            {
                std::cerr << "Error: Can't accept client connection." << std::endl;
                std::cerr << strerror(errno) << std::endl;
                continue;
            }


            char host[NI_MAXHOST];
            if (getnameinfo((struct sockaddr *)&client, cLen, host, NI_MAXHOST, NULL, 0, NI_NUMERICSERV) != 0)
            {
                close(newsockfd);
                continue;
            }

            this->epevent.events = EPOLLIN;
            this->epevent.data.fd = newsockfd;
            if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, newsockfd, &epevent) < 0)
            {
                std::cerr << "Error: Can't add client to epoll." << std::endl;
                std::cerr << strerror(errno) << std::endl;
                close(newsockfd);
            }

            this->clients[newsockfd] = new Client(newsockfd, std::string(host));
        }
        else
        {
            char buffer[1024] = {0};
            int user_fd = this->events[i].data.fd;
            int bytes_received = recv(user_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received > 0)
                this->processMessage(user_fd, buffer);
            else
                this->close_client_connection(user_fd);
        }
    }
}

void Serv::stop()
{
	std::cout << "Server is stopping..." << std::endl;
	for (std::vector<Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++)
		delete *it;
	for (std::map<int, Client *>::iterator iterator = this->clients.begin(); iterator != this->clients.end(); iterator++)
	{
		close(iterator->first);
		delete iterator->second;
	}
	close(this->fd);
	close(this->epollfd);
	std::cout << "Server stopped!" << std::endl;
}
