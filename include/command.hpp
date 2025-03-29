#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "irc.hpp"

class Serv;

class Commands
{
    private:
	    Commands();

    public:
	    static void pass_command(Serv &server, Client &user, std::string command);
	    static void join_command(Serv &server, Client &user, std::string command);
	    static void nick_command(Serv &server, Client &user, std::string command);
	    static void user_command(Serv &server, Client &user, std::string command);
	    static void part_command(Serv &server, Client &user, std::string command);
	    static void privmsg_command(Serv &server, Client &user, std::string command);
	    static void kick_command(Serv &server, Client &user, std::string command);
	    static void topic_command(Serv &server, Client &user, std::string command);
	    static void invite_command(Serv &server, Client &user, std::string command);
	    static void mode_command(Serv &server,  Client &user, std::string command);
	    static void handleDCC(Serv &server, Client &user, const std::string &target, const std::string &ctcpMessage);

};

#endif