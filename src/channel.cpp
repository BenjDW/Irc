#include "../include/irc.hpp"

Channel::Channel(std::string _name)
{
  this->name = _name;
  this->maxClients = 10;
  this->inviteOnly = false;
  this->topicOnlyOperator = true;
  this->topic = "";
  this->password = "";
}

Channel::~Channel() {}

std::string Channel::getName()
{
  return this->name;
}

void Channel::broadcast(std::string message)
{
  for (std::vector<Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
  {
    (*it)->sendMessage(message);
  }
}

void Channel::broadcast(std::string message, Client *client)
{
  for (std::vector<Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
  {
    if ((*it) != client)
      (*it)->sendMessage(message);
  }
}

void Channel::addClient(Client *client)
{
  this->clients.push_back(client);
}

void Channel::printClient()
{
  std::cout << "all client" << std::endl;
  for (std::vector<Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
  {
    std::cout << (*it)->getUsername() << std::endl;
  }
}

void Channel::removeClient(Client *client)
{
  this->clients.erase(std::remove(this->clients.begin(), this->clients.end(), client), this->clients.end());
}

void Channel::addOperator(Client *client)
{
  this->operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
  this->operators.erase(std::remove(this->operators.begin(), this->operators.end(), client), this->operators.end());
}

bool Channel::isClientOperator(Client *client)
{
  for (std::vector<Client *>::iterator it = this->operators.begin(); it != this->operators.end(); it++)
    if ((*it) == client)
      return true;
  return false;
}

bool Channel::hasClientJoined(Client *client)
{
  for (std::vector<Client *>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
    if ((*it) == client)
      return true;
  return false;
}

void Channel::setTopic(std::string _topic)
{
  this->topic = _topic;
}

void Channel::setInviteOnly(bool _inviteOnly)
{
  this->inviteOnly = _inviteOnly;
}

std::vector<Client *> &Channel::getClients()
{
  return this->clients;
}
bool Channel::isInviteOnly()
{
  return this->inviteOnly;
}

void Channel::setTopicOnlyOperator(bool _topicOnlyOperator)
{
  this->topicOnlyOperator = _topicOnlyOperator;
}

bool Channel::isTopicOnlyOperator()
{
  return this->topicOnlyOperator;
}

void Channel::setMaxClients(int _maxClients)
{
  this->maxClients = _maxClients;
}

size_t Channel::getMaxClients()
{
  return this->maxClients;
}

void Channel::setPassword(std::string _password)
{
  this->password = _password;
}

std::string Channel::getPassword()
{
  return this->password;
}

void Channel::AddInvited(Client *client)
{
  this->invited.push_back(client);
}

void Channel::removeInvited(Client *client)
{
  this->invited.erase(std::remove(this->invited.begin(), this->invited.end(), client), this->invited.end());
}

bool Channel::isInvited(Client *client)
{
  for (std::vector<Client *>::iterator it = this->invited.begin(); it != this->invited.end(); it++)
    if ((*it) == client)
      return true;
  return false;
}