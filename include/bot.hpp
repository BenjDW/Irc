/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bde-wits <bde-wits@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 12:21:43 by bde-wits          #+#    #+#             */
/*   Updated: 2025/04/10 10:55:08 by bde-wits         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP

#include "client.hpp"
// #include "serv.hpp"
#include "irc.hpp"

class Client;
class Serv;

class Bot : public Client
{
	private:
		Serv* _server;
		Bot();

	public:
		Bot(const std::string& host, Serv* server);
		~Bot();

		void sendHelloMessage(const std::string& channelName, const std::string& userName);

		void sendMessage(std::string message);
};

#endif