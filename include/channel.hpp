#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "irc.hpp"
#include "client.hpp"

class Client;
class Channel
{
    private:
        std::string name;
        std::vector<Client *> clients;
        std::vector<Client *> operators;
        std::vector<Client *> invited;

        size_t maxClients;
        std::string topic;
        bool topicOnlyOperator;
        bool inviteOnly;
        std::string password;
        Channel();

    public:
        Channel(std::string name);
        ~Channel();

        void broadcast(std::string message);
        void broadcast(std::string message, Client *client);
        void addClient(Client *client);
        void removeClient(Client *client);
        void addOperator(Client *client);
        void setInviteOnly(bool inviteOnly);
        void setTopicOnlyOperator(bool topicOnlyOperator);
        bool isTopicOnlyOperator();
        bool isInviteOnly();
        void AddInvited(Client *client);
        void removeInvited(Client *client);
        void setMaxClients(int maxClients);
        bool isInvited(Client *client);
        void setPassword(std::string password);
        std::string getPassword();
        size_t getMaxClients();
        void setTopic(std::string topic);
        void removeOperator(Client *client);
        void printClient();
        bool isClientOperator(Client *client);
        bool hasClientJoined(Client *client);
        std::vector<Client *> &getClients();

        std::string getName();
};

#endif
