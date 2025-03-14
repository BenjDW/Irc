#include "../include/irc.hpp"

//il faut applique la norm pour le protocole, donc parse de façons a rajoute les indicatifs /r ' ' en fonction de se que sais etc
//gere les return d'erreur du serv avec les bon codes
//
//Here are some examples of messages and how the parameters would be represented as JSON lists:
// :irc.example.com CAP * LIST :         ->  ["*", "LIST", ""]
// CAP * LS :multi-prefix sasl           ->  ["*", "LS", "multi-prefix sasl"]
// CAP REQ :sasl message-tags foo        ->  ["REQ", "sasl message-tags foo"]
// :dan!d@localhost PRIVMSG #chan :Hey!  ->  ["#chan", "Hey!"]
// :dan!d@localhost PRIVMSG #chan Hey!   ->  ["#chan", "Hey!"]
// :dan!d@localhost PRIVMSG #chan ::-)   ->  ["#chan", ":-)"]
// As these examples show, a trailing parameter (a final parameter with a preceding ':') has the same semantics as any other parameter, and MUST NOT be treated specially or stored separately once the ':' is stripped.

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "Error: Usage: ./ircserv [port] [password]." << std::endl;
        return (-1);
    }
    for (int i = 0, av[1][i] != '\0'; i++)
    {
        if (av[1][i] < '0' || av[1][i] > '9')
        {
            std::cerr << "Error: Unvalid port." << std::endl;
            return (-1);
        }
    }

    Serv    server(atoi(av[1]), av[2]);
    if (server.getPort() < 1)
    {
        std::cerr << "Error: port number too low." << std::endl;
        return (-1);
    }
    if (server.getPort() > 65535)
    {
        std::cerr << "Error: port number too large." << std::endl;
        return (-1);
    }
    if (server.start() < 0)
        return (-1);

	//main boucle , qui va recupere l'user , parser et execute les commandes asssocier ?
    while (server.isRunning())//flag true ?
    {
        //loop a faire
    }
}