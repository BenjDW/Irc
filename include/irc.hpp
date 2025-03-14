#pragma once

#include <iostream>

#include <stdlib.h>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <fcntl.h>

// In addition to the nickname, all servers must have the following information about all clients: 
// the real name/address of the host that the client is connecting from, the username of the client on that host, and the server to which the client is connected.

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

		// std::vector	client_tab; // vector d'objet d'users ou struct ?
		// std::map 	// map qui contient tout les users/bind avec les sockets ?name/address ?

        Serv &operator=(const Serv &origin);

        int getPort();
        std::string getPassword();
        bool isRunning();
};

// class client contient les infos de l'user de façons a y accede depuis la database du serveur ?
// (Nickname, user, namechannel ?)
class Client
{
	private:
		
	public:
		Client(std::string user, std::string namechannel, std::string nicknam);
		Client(std::string user, std::string namechannel);
		~Client();
		std::string	Nickname;
		std::string User;
		std::string Channel; //witch channel is the clients ?
};

