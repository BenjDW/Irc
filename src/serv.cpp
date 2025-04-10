#include "../include/irc.hpp"
#include "../include/bot.hpp"  // On inclut le header du Bot

Serv::Serv() : port(0), password(""), bot(NULL) {}

Serv::Serv(const int _port, const std::string _password)
    : port(_port), password(_password), bot(NULL)
{
    // On enregistre nos commandes "standards"
    this->commands["PASS"]    = Commands::pass_command;
    this->commands["JOIN"]    = Commands::join_command;
    this->commands["NICK"]    = Commands::nick_command;
    this->commands["USER"]    = Commands::user_command;
    this->commands["PART"]    = Commands::part_command;
    this->commands["PRIVMSG"] = Commands::privmsg_command;
    this->commands["KICK"]    = Commands::kick_command;
    this->commands["TOPIC"]   = Commands::topic_command;
    this->commands["INVITE"]  = Commands::invite_command;
    this->commands["MODE"]    = Commands::mode_command;

    // --- BONUS BOT : création du bot ---
    // On peut lui donner un hostname arbitraire, par ex. "localhost"
    this->bot = new Bot("localhost", this);
    this->bot->setNickname("Bot42");
    this->bot->setUsername("Bot42");
    this->bot->setLogged();
    this->bot->setIsBot(true);
}

Serv::Serv(const Serv &origin)
    : port(origin.getPort()),
      password(origin.getPassword()),
      bot(NULL) // On ne copie pas directement l'instance du bot
{}

Serv::~Serv() 
{
    close(fd); // Ferme le socket
    // On libère le bot
    if (this->bot)
        delete this->bot;
}

Serv &Serv::operator=(const Serv &origin)
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

int Serv::get_epollfd() const
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

// Récupérer un channel depuis son nom
Channel *Serv::getChannel(std::string name)
{
    for (std::vector<Channel *>::iterator it = this->channels.begin(); it != this->channels.end(); it++)
    {
        if ((*it)->getName() == name)
            return (*it);
    }
    return (NULL);
}

// Récupérer un client depuis son nickname
Client *Serv::getClient(std::string nickname)
{
    for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it->second->getNickname() == nickname)
            return (it->second);
    }
    return (NULL);
}

void Serv::addChannel(Channel *channel)
{
    this->channels.push_back(channel);
}

// Traite le buffer de messages d'un client
void Serv::processMessage(int user_fd, const char *message)
{
    std::string msg(message);
    std::string buffer;

    this->clients[user_fd]->appendToBuffer(msg);
    buffer = this->clients[user_fd]->getBuffer();
    if (buffer.find_first_of("\r\n") == std::string::npos)
        return;

    std::vector<std::string> cmds = splitCommands(buffer);
    for (std::vector<std::string>::iterator it = cmds.begin(); it != cmds.end(); ++it)
        this->interpret_message(user_fd, *it);

    // On clear le buffer du client
    if (this->clients[user_fd])
        this->clients[user_fd]->clearBuffer();
}

// Découpe un gros buffer en plusieurs commandes (\r\n)
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

// Interprète la commande
void Serv::interpret_message(int user_id, std::string const &command)
{
    Client *user = this->clients[user_id];
    if (!user)
    {
        std::cerr << "Error: can't find client with fd [" << user_id << "]" << std::endl;
        return;
    }
    // Extrait le nom de commande
    std::string cmdname = command.substr(0, command.find_first_of(" \r\n"));

    // Vérifie s'il n'est pas loggé et la cmd n'est pas PASS / CAP
    if (cmdname != "CAP" && cmdname != "PASS" && !user->isLogged())
    {
        std::cerr << "ERROR: Unauthorized connection, needs password!" << std::endl;
        user->sendMessage(ERR_NOTREGISTERED(user->getNickname()));
    }
    else
    {
        CommandFunction cmdf = this->commands[cmdname];
        if (cmdf)
            cmdf(*this, *user, command);
    }
}

void Serv::close_client_connection(int user_id, std::string reason)
{
    if (this->clients[user_id] != NULL)
    {
        // On le retire de tous les channels
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

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "fcntl F_GETFL error" << std::endl;
        close(fd);
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "fcntl F_SETFL error" << std::endl;
        close(fd);
        return false;
    }

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        std::cerr << "Error: epoll_create1 failed" << std::endl;
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

    epevent.data.fd = fd;
    epevent.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epevent) == -1)
    {
        perror("epoll_ctl");
        close(fd);
        close(epollfd);
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
                continue;
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
