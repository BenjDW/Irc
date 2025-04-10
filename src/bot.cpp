/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bde-wits <bde-wits@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 12:23:16 by bde-wits          #+#    #+#             */
/*   Updated: 2025/04/10 02:36:25 by bde-wits         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/bot.hpp"
#include "../include/channel.hpp"
#include "../include/serv.hpp"

Bot::Bot(const std::string& host, Serv* server)
    : Client(-1, host), _server(server)
{
    // Dans ce cas, on dit explicitement que c'est un bot
    this->setIsBot(true);
}

Bot::~Bot()
{
}

void Bot::sendHelloMessage(const std::string& channelName, const std::string& userName)
{
    std::string msg = "PRIVMSG " + channelName + " :Bonjour " + userName + "!\r\n";
    this->sendMessage(msg);
}

void Bot::sendMessage(std::string message)
{
    // Ici, on parse "PRIVMSG #channel :contenu"
    // pour relayer aux gens du channel

    // Cherche la cible
    size_t pos = message.find("PRIVMSG ");
    if (pos == std::string::npos)
        return;
    pos += 8; // saute "PRIVMSG "
    size_t end = message.find(" :", pos);
    if (end == std::string::npos)
        return;

    std::string targetName    = message.substr(pos, end - pos);
    std::string actualMessage = message.substr(end + 2); // saute " :"

    // On envoie dans le channel si trouvé
    Channel* channel = _server->getChannel(targetName);
    if (channel)
    {
        // Broadcast un vrai "PRIVMSG" pour que tous les clients voient le message du bot
        channel->broadcast(":" + this->getNickname() + " PRIVMSG " 
                           + targetName + " :" + actualMessage + "\r\n");
    }
}