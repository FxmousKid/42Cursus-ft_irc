#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

typedef std::string str;
typedef std::vector<str> vec_str;
class Server;
class Command;
class Client;

class CommandHandler
{
private:
	Server *_server;
	std::map<std::string, Command *> _commands;
public:
	CommandHandler(Server *server);
	~CommandHandler();

	void invoke(Client *client, std::string const &message);
};
