#pragma once

#include <iostream>

#include <stdlib.h>

#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <fcntl.h>
#include <cerrno>

// In addition to the nickname, all servers must have the following information about all clients: 
// the real name/address of the host that the client is connecting from, the username of the client on that host, and the server to which the client is connected.

class Client
{
	private:
        int id;

        bool logged;

        int pass_tries;

        std::string	nickname;
        std::string user;
        std::string host;
        std::string buffer;

        Channel *channel; //witch channel is the clients ?

        Client();
        Client(const Client &origin);
        Client &operator=(const Client &origin);

	public:
		Client(int _id, std::string hostname);
		virtual ~Client();


        int const &getId() const;
        std::string const &getHostname() const;
        std::string const &getNickname() const;
        std::string getUsername() const;
        int const& getTries() const;
        Channel *getChannel();
        std::string getBuffer() const;

        void setUsername(std::string username);
        void setNickname(std::string newNickname);
        void setHostname(std::string newHost);
        void setChannel(Channel *channel);
        void setLogged();

        bool isLogged() const;

        virtual void sendMessage(std::string message);
        void appendToBuffer(const std::string& msg);
        void clearBuffer();
        void incrementTries();
};

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
		//

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

        bool start();
        void loop();
        void stop();
};
