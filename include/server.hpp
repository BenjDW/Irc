#ifndef SERVER_HPP
#define SERVER_HPP

#include "irc.hpp"

class Serv
{
    private:
        int   port;
        const std::string   password;
        bool    running;

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
        bool isRunning() const;
        epoll_event getPevent() const;
        epoll_event *getEvents() const;
        epoll_event getEvent(int index) const;
        Client *getClient(std::string nickname);
        Channel *getChannel(std::string name);


        bool start();
        void loop();
        void stop();
};

#endif