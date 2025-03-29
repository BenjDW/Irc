#pragma once

#include "allincl.hpp"

#include "client.hpp"
#include "channel.hpp"
#include "server.hpp"

class Client;
class Server;
class Channel;
typedef void (*CommandFunction)(Server &server, Client &user, std::string command);