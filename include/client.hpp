#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "irc.hpp"
#include "channel.hpp"
// #include "bot.hpp"

class Channel;
// class bot;

class Client
{
    private:
        int         id;
        bool        logged;
        int         pass_tries;
        std::string nickname;
        std::string user;
        std::string host;
        std::string buffer;

        Channel*    channel;

        bool        is_bot;

        Client();
        Client(const Client &origin);
        Client &operator=(const Client &origin);

    public:
        Client(int _id, std::string hostname);
        virtual ~Client();

        int const &           getId() const;
        std::string const &   getHostname() const;
        std::string const &   getNickname() const;
        std::string           getUsername() const;
        int const &           getTries() const;
        Channel*              getChannel();
        std::string           getBuffer() const;

        bool                  isLogged() const;
        bool                  isBot() const { return is_bot; }      // getter
        void                  setIsBot(bool val) { is_bot = val; }  // setter

        void setUsername(std::string username);
        void setNickname(std::string newNickname);
        void setHostname(std::string newHost);
        void setChannel(Channel *channel);
        void setLogged();

        virtual void sendMessage(std::string message);

        void appendToBuffer(const std::string& msg);
        void clearBuffer();
        void incrementTries();
};

#endif