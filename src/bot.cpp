/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bde-wits <bde-wits@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 12:23:16 by bde-wits          #+#    #+#             */
/*   Updated: 2025/04/10 16:42:08 by bde-wits         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/bot.hpp"
#include "../include/serv.hpp"

Bot::Bot(const std::string &host, Serv *server) : Client(-1, host), _server(server)
{
	setNickname("coco_peroquet");
	setUsername("coco_peroquet");
	setLogged();
	setIsBot(true);
}

Bot::~Bot() {}

void Bot::sendHelloMessage(const std::string &channel, const std::string &user)
{
	std::string raw = "PRIVMSG " + channel + " :Bonjour " + user + " !\r\n";
	sendMessage(raw);
}

void Bot::sendMessage(std::string raw)
{
	size_t p = raw.find("PRIVMSG ");
	if (p == std::string::npos)
		return;
	p += 8;
	size_t e = raw.find(" :", p);
	if (e == std::string::npos)
		return;
	std::string target  = raw.substr(p, e - p);
	std::string message = raw.substr(e + 2);

	_server->sendMessageFromBot(this, target, message);
}