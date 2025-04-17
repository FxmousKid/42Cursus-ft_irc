#pragma once

# include <vector>
# include <string>

typedef std::string str;
typedef std::vector<str> vec_str;
class Client;
class Server;

class Channel
{
private:
	str _name;
	Client		*_admin;

	int 		_l; //max user in chan
	bool		_i; //invite-only chan
	str 		_k; //chan's key(paasword)

	std::vector<Client *> _clients;
	std::vector<Client *> _oper_clients;

	Server *_server;

	unsigned long	_clientIndex(std::vector<Client *> clients, Client *client);
public:
	Channel(str const &name, const str &password, Client *admin, Server *server);
	Channel&	operator=(const Channel &src);

	Channel(const Channel &src);
	~Channel();

	// GETTERS

	Client						*getAdmin() const;
	str const 					&getName() const;
	str const 					&getPassword() const;
	int							getMaxUsers() const;
	int							invitOnlyChan();

	Client*						getClient(const str &nickname);
	std::vector<Client *> 		getChanClients() const;
	std::vector<Client *> 		getChanOpers() const;

	int							getNbrClients() const;
	std::vector<str>			getNickNames();

	// SETTERS

	void						setAdmin(Client *client);
	void						setPassword(str k);
	void						setMaxClients(int l);
	void						setInviteOnly(bool active);

	// OTHER

	void 						broadcast(str const &message);
	void 						broadcast(const str &message, Client *exclude);
	void 						removeClient(Client *client, str reason);
	void 						removeOper(Client *client);
	void						addClient(Client *client);
	void						addOper(Client *client);
	void						kick(Client *client, Client *target, str reason);
	void						invit(Client *client, Client *target);
	int 						is_oper(Client *client);
	bool						isInChannel(Client *client);
};;
