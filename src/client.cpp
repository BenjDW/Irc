Client::Client(std::string user, std::string namechannel, std::string nicknam) : Nickname(nicknam), User(user), Channel(namechannel)
{
}

Client::Client(std::string user, std::string namechannel) : User(user), Channel(namechannel), Nickname(NULL)
{
}

Client::~Client()
{
}