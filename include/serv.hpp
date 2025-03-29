#ifndef SERVER_HPP
#define SERVER_HPP

#include "irc.hpp"

class Serv;
class Commands;

typedef void (*CommandFunction)(Serv &server, Client &user, std::string command);

class Serv
{
    private:
        int   port;
        const std::string   password;

        struct sockaddr_in address;
        int     fd;//socket fd
		int		epollfd;
		struct epoll_event	epevent;
        struct epoll_event  events[10];
        std::map<int, Client *> clients;
        std::map<std::string, CommandFunction> commands;
        std::vector<Channel *> channels;

    public:
        Serv();
        Serv(const int _port, const std::string _password);
        Serv(const Serv &origin);
        ~Serv();

        Serv &operator=(const Serv &origin);

        int getPort() const;
        std::string getPassword() const;
        int getSocket() const;
		int	get_epollfd() const;
        epoll_event getPevent() const;
        epoll_event getEvent(int index) const;
        Client *getClient(std::string nickname);
        Channel *getChannel(std::string name);
        //std::vector<Channel *> Serv::getChannels();

        void addChannel(Channel* channel);
        void processMessage(int user_fd, const char *message);
        std::vector<std::string> splitCommands(const std::string &msg);
        void	interpret_message(int user_id, std::string const& command);
        void	close_client_connection(int user_id, std::string reason = "");

        bool start();
        void loop();
        void stop();
};

#endif