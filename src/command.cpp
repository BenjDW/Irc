#include "../include/irc.hpp"
#include "../include/bot.hpp"
#include "../include/client.hpp"
#include "../include/channel.hpp"

void Commands::pass_command(Serv &server, Client &user, std::string command)
{
    std::string password;
    size_t password_index;
    size_t end_index;

    if (user.isLogged())
        return;

    if ((password_index = command.find(' ')) != std::string::npos)
    {
        if (command.find(' ', password_index + 1) != std::string::npos)
        {
            user.sendMessage(ERR_PASSWORD(user.getNickname()));
            std::cerr << "ERROR: Incorrect PASS command format" << std::endl;
            return;
        }
        end_index = command.find_first_of("\r\n", password_index + 1);
        password = command.substr(password_index + 1, end_index - (password_index + 1));

        if (password == server.getPassword())
        {
            std::cout << "User " << user.getId() << " logged in succesfully!" << std::endl;
            user.setLogged();
            user.sendMessage("Good password!\r\n");
        }
        else
        {
            user.sendMessage(ERR_PASSWORD(user.getNickname()));
            user.incrementTries();
            if (user.getTries() >= 3)
            {
                user.sendMessage("You typed too many wrong passwords!\r\n");
                server.close_client_connection(user.getId(), "Too many password tries!");
            }
        }
    }
    else
        user.sendMessage("No password given!\r\n");
}

void Commands::part_command(Serv &server, Client &user, std::string command)
{
    size_t spacePos = command.find(" ");
    if (spacePos == std::string::npos)
        return;
    std::string channelName = command.substr(spacePos + 1);
    if (channelName.empty() || channelName[0] != '#')
        return;
    channelName = channelName.substr(0, channelName.find_first_of(" \r\n"));
    Channel *channel = server.getChannel(channelName);
    if (!channel)
        return;

    std::cout << "[_REQUEST] :" << user.getNickname() << " PART " << channel->getName() << std::endl;
    channel->broadcast(":" + user.getNickname() + " PART " + channelName + "\r\n");
    channel->removeClient(&user);
}

void Commands::privmsg_command(Serv &server, Client &user, std::string command)
{
    // Extraction de la cible et du message
    std::string::size_type firstSpace = command.find(" ");
    if (firstSpace == std::string::npos)
        return;
    std::string remainder = command.substr(firstSpace + 1);
    std::string::size_type secondSpace = remainder.find(" ");
    if (secondSpace == std::string::npos)
        return;

    std::string target  = remainder.substr(0, secondSpace);
    std::string message = remainder.substr(secondSpace + 1);

    // Supprime le ':' éventuel devant le message
    if (!message.empty() && message[0] == ':')
        message.erase(0, 1);

    // --- BONUS DCC ---
    // Si message est de type CTCP: \x01 ... \x01
    if (!message.empty() && message[0] == '\x01' && message[message.size() - 1] == '\x01')
    {
        // On retire les \x01
        std::string ctcpMessage = message.substr(1, message.size() - 2);
        // Vérifie si c'est un DCC
        if (ctcpMessage.size() >= 4 && ctcpMessage.substr(0, 4) == "DCC ")
        {
            handleDCC(server, user, target, ctcpMessage);
            return;
        }
    }

    // Sinon comportement classique
    if (target[0] == '#')
    {
        Channel *channel = server.getChannel(target);
        if (!channel)
        {
            user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), target));
            return;
        }
        channel->broadcast(":" + user.getNickname() + " PRIVMSG " + channel->getName() + " :" + message + "\r\n", &user);
    }
    else
    {
        Client *targetClient = server.getClient(target);
        if (!targetClient)
        {
            user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), target));
            return;
        }
        targetClient->sendMessage(":" + user.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n");
    }
}

void Commands::handleDCC(Serv &server, Client &user, const std::string &target, const std::string &ctcpMessage)
{
    Client *targetClient = server.getClient(target);
    if (!targetClient)
    {
        user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), target));
        return;
    }
    // On relaie tel quel le CTCP à la cible
    std::string relayMessage = ":" + user.getNickname() + " PRIVMSG " + target
        + " :\x01" + ctcpMessage + "\x01\r\n";
    targetClient->sendMessage(relayMessage);
}

void Commands::join_command(Serv &server, Client &user, std::string command)
{
    std::stringstream ss(command);
    std::vector<std::string> args(3);
    Channel *channel;

    // Vérif si l'user a bien NICK et USER
    if (user.getNickname().empty() || user.getUsername().empty())
    {
        user.sendMessage(ERR_NOTREGISTERED(user.getNickname()));
        return;
    }

    // 1) Lire le mot-clé "JOIN" dans args[0]
    ss >> args[0];  // ex: "JOIN"

    // 2) Lire #channel dans args[1], puis éventuel password dans args[2]
    for (int i = 1; i < 3; i++)
    {
        if (!ss.eof())
        {
            ss >> args[i];  // par ex. "#ok", puis "secret" si présent
            // Vider s'il y a juste un \r\n
            if (args[i] == "\n" || args[i] == "\r\n")
                args[i].clear();
        }
        else
            args[i].clear();
    }

    // Désormais :
    //   args[0] = "JOIN"
    //   args[1] = "#ok" (ou autre channel)
    //   args[2] = "password?" (si fourni)

    if (args[1].empty() || args[1][0] != '#')
    {
        user.sendMessage(ERR_NOSUCHCHANNEL(user.getNickname(), args[1]));
        return;
    }

    // Récupère ou crée le channel
    channel = server.getChannel(args[1]);
    if (!channel)
    {
        channel = new Channel(args[1]);
        server.addChannel(channel);
        channel->addOperator(&user);

        // Ajout du bot si présent
        if (server.getBot())
        {
            channel->addClient(server.getBot());
            channel->broadcast(":" + server.getBot()->getNickname()
                               + " JOIN " + channel->getName() + "\r\n");
        }
    }

    // Vérifications inviteOnly, limite max, password, etc.
    if (channel->isInviteOnly() && !channel->isClientOperator(&user) && !channel->isInvited(&user))
    {
        user.sendMessage(ERR_INVITEONLYCHAN(user.getNickname(), channel->getName()));
        return;
    }
    else
        channel->removeInvited(&user);

    if (channel->getMaxClients() <= channel->getClients().size())
    {
        user.sendMessage(ERR_CHANNELISFULL(user.getNickname(), channel->getName()));
        return;
    }

    if (!channel->getPassword().empty() && channel->getPassword() != args[2])
    {
        user.sendMessage(ERR_BADCHANNELKEY(user.getNickname(), channel->getName()));
        return;
    }

    // Ajout effectif
    channel->addClient(&user);
    std::cout << "[_REQUEST] :" << user.getNickname() << " JOIN "
              << channel->getName() << std::endl;
    channel->broadcast(":" + user.getNickname()
                       + " JOIN " + channel->getName() + "\r\n");

    // Informe le user de la présence des autres dans ce channel
    for (std::vector<Client*>::iterator it = channel->getClients().begin();
         it != channel->getClients().end(); ++it)
    {
        if (*it != &user)
            user.sendMessage(":" + (*it)->getNickname()
                             + " JOIN " + channel->getName() + "\r\n");
    }

    // Message de bienvenue du bot, si défini
    // if (server.getBot())
    //     server.getBot()->sendHelloMessage(channel->getName(),
    //                                       user.getNickname());
}


void Commands::nick_command(Serv &server, Client &user, std::string command)
{
    if (!user.getNickname().empty())
    {
        user.sendMessage(ERR_ALREADY_REGISTERED(user.getNickname()));
        return;
    }

    size_t nick_pos = command.find(" ") + 1;
    size_t end_pos  = command.find_first_of("\r\n", nick_pos);
    std::string nick = command.substr(nick_pos, end_pos - nick_pos);

    if (!server.getClient(nick))
        user.setNickname(nick);
    else
        user.sendMessage(ERR_NICKNAMEINUSE(nick));
}

void Commands::user_command(Serv &server, Client &user, std::string command)
{
    (void)server;

    if (user.getNickname().empty())
    {
        user.sendMessage(ERR_NONICKNAMEGIVEN);
        return;
    }
    if (!user.getUsername().empty())
    {
        user.sendMessage(ERR_ALREADY_REGISTERED(user.getNickname()));
        return;
    }

    std::string name = command.substr(0, command.find(' '));
    std::vector<std::string> argv;
    std::stringstream line(command.substr(name.length(), command.length()));
    std::string buff;

    while (line >> buff)
        argv.push_back(buff);
    if (!argv.empty())
        user.setUsername(argv[0]);
}

void Commands::kick_command(Serv &server, Client &user, std::string command)
{
    std::stringstream ss(command);
    std::vector<std::string> args(3);
    ss >> args[0]; // "KICK"
    for (int i = 0; i < 3 && !ss.eof(); i++)
    {
        ss >> args[i];
        if (args[i] == "\n" || args[i] == "\r\n")
            args[i].clear();
    }

    Channel *channel = server.getChannel(args[0]);
    if (!channel)
    {
        user.sendMessage(ERR_NOSUCHCHANNEL(user.getNickname(), args[0]));
        return;
    }
    if (!channel->hasClientJoined(&user))
    {
        user.sendMessage(ERR_NOTONCHANNEL(user.getNickname(), channel->getName()));
        return;
    }
    if (!channel->isClientOperator(&user))
    {
        user.sendMessage(ERR_CHANOPRIVSNEEDED(user.getNickname(), channel->getName()));
        return;
    }

    Client *to_kick = server.getClient(args[1]);
    if (!to_kick)
    {
        user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), args[1]));
        return;
    }
    if (channel->isClientOperator(to_kick))
    {
        user.sendMessage(ERR_KICKOPERATOR(user.getNickname()));
        return;
    }
    if (!channel->hasClientJoined(to_kick))
    {
        user.sendMessage(ERR_NOTONCHANNEL(user.getNickname(), channel->getName()));
        return;
    }
    // Kick effectif
    channel->removeClient(to_kick);
    user.sendMessage(KICK_RPL(user.getNickname(), to_kick->getNickname(), channel->getName(), args[2]));
    to_kick->sendMessage(KICK_RPL(user.getNickname(), to_kick->getNickname(), channel->getName(), args[2]));
}

void Commands::topic_command(Serv &server, Client &user, std::string command)
{
    std::stringstream ss(command);
    std::vector<std::string> args(2);
    ss >> args[0]; // "TOPIC"
    for (int i = 0; i < 2 && !ss.eof(); i++)
    {
        ss >> args[i];
        if (args[i] == "\n" || args[i] == "\r\n")
            args[i].clear();
    }
    Channel *channel = server.getChannel(args[0]);
    if (!channel)
    {
        user.sendMessage(ERR_NOSUCHCHANNEL(user.getNickname(), args[0]));
        return;
    }
    if (!channel->isClientOperator(&user) && channel->isTopicOnlyOperator())
    {
        user.sendMessage(ERR_CHANOPRIVSNEEDED(user.getNickname(), channel->getName()));
        return;
    }
    channel->setTopic(args[1]);
    channel->broadcast(":" + user.getNickname() + " TOPIC " + channel->getName() + " :" + args[1] + "\r\n");
}

void Commands::invite_command(Serv &server, Client &user, std::string command)
{
    std::stringstream ss(command);
    std::vector<std::string> args(2);
    ss >> args[0]; // "INVITE"
    for (int i = 0; i < 2 && !ss.eof(); i++)
    {
        ss >> args[i];
        if (args[i] == "\n" || args[i] == "\r\n")
            args[i].clear();
    }
    Channel *channel = server.getChannel(args[1]);
    if (!channel)
    {
        user.sendMessage(ERR_NOSUCHCHANNEL(user.getNickname(), args[1]));
        return;
    }
    if (!channel->isClientOperator(&user))
    {
        user.sendMessage(ERR_CHANOPRIVSNEEDED(user.getNickname(), channel->getName()));
        return;
    }
    Client *to_invite = server.getClient(args[0]);
    if (!to_invite)
    {
        user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), args[0]));
        return;
    }
    if (channel->hasClientJoined(to_invite))
    {
        user.sendMessage(ERR_USERONCHANNEL(user.getNickname(), to_invite->getNickname(), channel->getName()));
        return;
    }
    if (channel->isInvited(to_invite))
    {
        // L'utilisateur est déjà dans la liste d'invite
        user.sendMessage("User is already invited.\r\n");
        return;
    }
    channel->AddInvited(to_invite);
    to_invite->sendMessage(":" + user.getNickname() + " INVITE " + to_invite->getNickname() + " " + channel->getName() + "\r\n");
    user.sendMessage(":" + user.getNickname() + " INVITE " + to_invite->getNickname() + " " + channel->getName() + "\r\n");
}

void operator_command(Channel *channel, Serv &server, Client &user, std::vector<std::string> args)
{
    if (args[1] == "+o")
    {
        Client *to_op = server.getClient(args[2]);
        if (!to_op)
        {
            user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), args[2]));
            return;
        }
        if (!channel->isClientOperator(to_op))
        {
            channel->addOperator(to_op);
            user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " +o " + to_op->getNickname() + "\r\n");
        }
        else
            user.sendMessage("This user is already an operator!\r\n");
    }
    else if (args[1] == "-o")
    {
        Client *to_deop = server.getClient(args[2]);
        if (!to_deop)
        {
            user.sendMessage(ERR_NOSUCHNICK(user.getNickname(), args[2]));
            return;
        }
        if (channel->isClientOperator(to_deop))
        {
            channel->removeOperator(to_deop);
            user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " -o " + to_deop->getNickname() + "\r\n");
        }
        else
            user.sendMessage("This user is not an operator!\r\n");
    }
}

void invite_mode_command(Channel *channel, Client &user, std::vector<std::string> args)
{
    if (args[1] == "+i")
    {
        channel->setInviteOnly(true);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " +i\r\n");
    }
    else if (args[1] == "-i")
    {
        channel->setInviteOnly(false);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " -i\r\n");
    }
}

void limit_mode_command(Channel *channel, Client &user, std::vector<std::string> args)
{
    if (args[1] == "+l")
    {
        channel->setMaxClients(std::atoi(args[2].c_str()));
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " +l " + args[2] + "\r\n");
    }
    else if (args[1] == "-l")
    {
        channel->setMaxClients(10);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " -l\r\n");
    }
}

void pass_mode_command(Channel *channel, Client &user, std::vector<std::string> args)
{
    if (args[1] == "+k")
    {
        channel->setPassword(args[2]);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " +k " + args[2] + "\r\n");
    }
    else if (args[1] == "-k")
    {
        channel->setPassword("");
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " -k\r\n");
    }
}

void topic_mode_command(Channel *channel, Client &user, std::vector<std::string> args)
{
    if (args[1] == "+t")
    {
        channel->setTopicOnlyOperator(true);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " +t\r\n");
    }
    else if (args[1] == "-t")
    {
        channel->setTopicOnlyOperator(false);
        user.sendMessage(":" + user.getNickname() + " MODE " + channel->getName() + " -t\r\n");
    }
}

void Commands::mode_command(Serv &server, Client &user, std::string command)
{
    std::stringstream ss(command);
    std::vector<std::string> args(3);
    ss >> args[0];
    for (int i = 0; i < 3 && !ss.eof(); i++)
    {
        ss >> args[i];
        if (args[i] == "\n" || args[i] == "\r\n")
            args[i].clear();
    }
    Channel *channel = server.getChannel(args[0]);
    if (!channel)
    {
        user.sendMessage(ERR_NOSUCHCHANNEL(user.getNickname(), args[0]));
        return;
    }
    if (!channel->isClientOperator(&user))
    {
        user.sendMessage(ERR_CHANOPRIVSNEEDED(user.getNickname(), channel->getName()));
        return;
    }
    if (args.size() < 2 || args[1].empty())
    {
        user.sendMessage("Unknown mode!\r\n");
        return;
    }
    // Dispatch
    if (args[1] == "+o" || args[1] == "-o")
        operator_command(channel, server, user, args);
    else if (args[1] == "+i" || args[1] == "-i")
        invite_mode_command(channel, user, args);
    else if (args[1] == "+l" || args[1] == "-l")
        limit_mode_command(channel, user, args);
    else if (args[1] == "+k" || args[1] == "-k")
        pass_mode_command(channel, user, args);
    else if (args[1] == "+t" || args[1] == "-t")
        topic_mode_command(channel, user, args);
    else
        user.sendMessage("Unknown mode!\r\n");
}
