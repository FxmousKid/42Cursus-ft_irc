#pragma once

# include <vector>
# include <iostream>
# include <stdio.h>
# include <string>
# include <fcntl.h>
# include <stdlib.h>
# include <ctime>
# include <errno.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <sys/poll.h>
# include <netinet/in.h>
# include <sys/time.h>

typedef std::string str;
typedef std::vector<str> vec_str;

# include "CommandHandler.hpp"

# define DEFAULT_PORT 6667
# define DEFAULT_SERVER_NAME "irc.42.fr"
# define MAX_CONN 1024

class Client;
class Channel;

class Server {
private:
	const int				_port;
	str			 			_password;
	std::vector<Client *>	_clients;
	std::vector<Channel *>	_channels;
	str						_server_name;
	str						_start_time;

	int						_server_socket;
	struct pollfd			*_clients_fds;
	CommandHandler			_handler;


	void					_waitActivity(void);
	void					_constructFds(void);
	void 					_setNonBlocking(int fd);
	void					_acceptConnection(void);
	void 					_receiveData(Client *client);
	void					_handleMessage(const str& message, Client *client);

public:
	Server(int port, str const &password);
	Server(const Server &src);
	Server &operator =(const Server &src);
	~Server(void);

	// Server
	void			listen(void);
	ssize_t			send(str message, int const client_fd) const;
	void			broadcast(const str& message) const;
	void			broadcast(const str& message, int const exclude_fd) const;
	void			broadcastChannel(const str& message, Channel const *channel) const;
	void			broadcastChannel(str message, int exclude_fd, Channel const *channel) const;

	str&			getPassword(void);
	str&			getServerName(void) ;
	str&			getStartTime(void);

	// Client
	vec_str						getNickNames(void);
	std::vector<Client *> 		getServClients() const { return _clients; };
	int							addClient(int const fd, str const ip, int const port);
	int							delClient(int fd);
	Client*						getClient(int fd);
	Client*						getClient(const str &nickname);

	// Channel
	Channel*					getChannel(str const &name);
	std::vector<Channel *>		getServChannels() const { return _channels; };
	Channel* 					createChannel(str const &name, str const &password, Client *client);
};
