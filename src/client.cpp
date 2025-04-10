#include "../include/irc.hpp"
#include "../include/client.hpp"
#include "../include/channel.hpp"

Client::Client(int _id, std::string hostname)
    : id(_id), host(hostname), channel(NULL)
{
    this->logged = false;
    this->pass_tries = 0;

    this->is_bot = false;  // par défaut
}

Client::~Client() {}

int const &Client::getId() const
{
    return this->id;
}

std::string const &Client::getHostname() const
{
    return this->host;
}

std::string const &Client::getNickname() const
{
    return this->nickname;
}

void Client::setNickname(std::string newNickname)
{
    // On autorise un setNickname même si vide ?
    // Ici, on l'interdit simplement si déjà set:
    if (this->nickname.empty())
        this->nickname = newNickname;
}

void Client::sendMessage(std::string message)
{
	if (this->isBot() == true || this->id < 0)
		return ;
    if (send(this->id, message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error while sending message to client " << this->id
                  << " : " << strerror(errno) << std::endl;
    }
}

void Client::setHostname(std::string newHost)
{
    this->host = newHost;
}

void Client::setChannel(Channel *_channel)
{
    this->channel = _channel;
}

Channel *Client::getChannel()
{
    return this->channel;
}

bool Client::isLogged() const
{
    return this->logged;
}

void Client::setUsername(std::string username)
{
    this->user = username;
}

std::string Client::getUsername() const
{
    return this->user;
}

void Client::appendToBuffer(const std::string &msg)
{
    buffer += msg;
}

std::string Client::getBuffer() const
{
    return buffer;
}

void Client::clearBuffer()
{
    buffer.clear();
}

void Client::setLogged()
{
    this->logged = true;
}

int const &Client::getTries() const
{
    return this->pass_tries;
}

void Client::incrementTries()
{
    this->pass_tries++;
}