#include "ft_irc.hpp"

PrivMsgCommand::PrivMsgCommand(Server *server) : Command(server) {};

PrivMsgCommand::~PrivMsgCommand() {};

void PrivMsgCommand::execute(Client *client, vec_str arguments) {

	if (arguments.size() < 2 || arguments[0].empty() || arguments[1].empty()) {
		client->reply(ERR_NEEDMOREPARAMS(client->getNickName(), "PRIVMSG"));
		return;
	}

	str target = arguments.at(0);
	str message;

	for (vec_str::iterator it = arguments.begin() + 1; it != arguments.end(); it++) {
		message.append(*it + " ");
	}

	message = message.at(0) == ':' ? message.substr(1) : message;

	if (target.at(0) == '#') {

		std::vector<Channel *> client_chans = client->getUserChans();
		std::vector<Channel *>:: iterator it = client_chans.begin();

		Channel *chan;
		str chan_name;
		while (it != client_chans.end())
		{
			chan = it.operator*();
			if (chan->getName() == target)
				break ;
			++it;
		}
		if (it == client_chans.end())
		{
			_server->broadcast(ERR_NOTONCHANNEL(client->getNickName(), target));
			return;
		}

		chan->broadcast(RPL_PRIVMSG(client->getPrefix(), target, message), client);
		return;
	}

	Client *dest = _server->getClient(target);
	if (!dest)
	{
		client->reply(ERR_NOSUCHNICK(client->getNickName(), target));
		return;
	}

	dest->write(RPL_PRIVMSG(client->getPrefix(), target, message));
}
