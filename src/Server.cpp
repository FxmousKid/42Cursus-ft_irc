#include "Server.hpp"
#include "ft_irc.hpp"
#include <cerrno>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>

bool Server::_signal_recv = false;

Server::Server(int port, str const &password) :
	_port(port),
	_password(password),
	_server_name(DEFAULT_SERVER_NAME),
	_start_time(dateString()),
	_clients_fds(NULL),
	_handler(CommandHandler(this))
{
}

Server::~Server(void)
{
	for (unsigned long i = 0; i < this->_clients.size(); i++)
		delete this->_clients[i];
	for (unsigned long i = 0; i < this->_channels.size(); i++)
		delete this->_channels[i];
	delete[] this->_clients_fds;
}

void	Server::listen(void)
{
	struct sockaddr_in server_addr;

	// signal handler
	this->setSignalHandler();

	this->_server_socket = socket(AF_INET, SOCK_STREAM , 0);
	if(this->_server_socket == 0)
	{
		std::cout << "Error: Socket creation failed." << std::endl;
		return;
	}

	// server socket options
	int opt = 1;
	if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cout << "Error: Can't set socket options." << std::endl;
		return;
	}

	// sets socket as non-blocking with fcntl
	this->_setNonBlocking(this->_server_socket);
	server_addr.sin_family = AF_INET; // Address family IPv4
	server_addr.sin_port = htons(_port); // converts port to network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY; // any local machine address
	


	if (bind(_server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cout << "Error: Can't bind socket." << std::endl;
		close(this->_server_socket);
		return;
	}

	std::cout << GRN << "Starting ircserv on port " << _port << WHT << std::endl;

	// listen
	if (::listen(this->_server_socket, MAX_CONN) < 0)
	{
		std::cout << "Error: Can't listen on socket." << std::endl;
		return;
	}

	std::cout << "Waiting for connections ..." << std::endl;
	this->_constructFds();
	while (!Server::_signal_recv)
		this->_waitActivity();
}

#include <cerrno>
#include <cstring>


/*
void	Server::_waitActivity(void)
{
	// wait for an activity in a socket
	if (poll(this->_clients_fds, this->_clients.size() + 1, -1) < 0 && !Server::_signal_recv)
	{
		std::cout << std::strerror(errno) <<"\nError: Can't look for socket(s) activity." << std::endl;
		std::fflush(stdout);
		throw std::runtime_error("poll() failed");
	}

	// loop in every client socket for a connection
	for(unsigned long i=0; i < this->_clients.size() + 1; i++)
	{
		// revents will be != 0 if there is an activity on the socket
		if(this->_clients_fds[i].revents == 0)
			continue;

		// if server socket is active, accept the connection
		if (this->_clients_fds[i].fd == this->_server_socket)
			this->_acceptConnection();
		else if (i > 0)
		{		
			Client *client = this->_clients[i - 1];
			this->_receiveData(client);
		}
	}
}
*/

void Server::_waitActivity(void)
{
    // wait for an activity in a socket
    if (poll(this->_clients_fds, this->_clients.size() + 1, -1) < 0 && !Server::_signal_recv)
    {
        std::cout << std::strerror(errno) <<"\nError: Can't look for socket(s) activity." << std::endl;
        std::fflush(stdout);
        throw std::runtime_error("poll() failed");
    }

    // Check server socket first
    if (this->_clients_fds[0].revents != 0)
        this->_acceptConnection();
    
    // Now process client sockets - store indexes to process in a separate vector
    // This prevents issues with deleting clients during processing
    std::vector<unsigned long> active_clients;
    for (unsigned long i = 1; i < this->_clients.size() + 1; i++)
    {
        if (this->_clients_fds[i].revents != 0)
            active_clients.push_back(i - 1);  // Store the client index to process
    }
    
    // Now process active clients
    for (std::vector<unsigned long>::iterator it = active_clients.begin(); 
         it != active_clients.end(); ++it)
    {
        // Check if this client index is still valid
        if (*it < this->_clients.size())
        {
            Client *client = this->_clients[*it];
            int client_fd = client->getFD();
            
            // Process the client data safely
            this->_processClientData(client);
            
            // Check if this client was deleted during processing
            bool client_exists = false;
            for (unsigned long i = 0; i < this->_clients.size(); i++)
            {
                if (this->_clients[i]->getFD() == client_fd)
                {
                    client_exists = true;
                    break;
                }
            }
            
            if (!client_exists)
            {
                // Client was deleted, skip any further processing
                continue;
            }
        }
    }
}

// New method to safely process client data
void Server::_processClientData(Client *client)
{
    char buffer[BUFFER_SIZE + 1];
    int client_fd = client->getFD();  // Store the fd for later checks
    
    while (!Server::_signal_recv) {
        int ret = recv(client_fd, buffer, sizeof(buffer), 0);
        if (ret < 0)
        {
            if (errno != EWOULDBLOCK)
            {
                std::cout << "Error: recv() failed for fd " << client_fd;
                this->delClient(client_fd);
            }
            // no more data to receive
            break;
        }
        else if (!ret)
        {
            if (DEBUG)
                std::cout << RED << "Error: Connection closed by client " << client_fd << WHT << std::endl;
            this->delClient(client_fd);
            break;
        }
        else
        {
            buffer[ret] = '\0';
            str buff = buffer;
            
            std::cout << YEL <<"recv(" << client_fd << "): " << WHT << buff;
            
            // Process complete lines
            if (buff.at(buff.size() - 1) == '\n') {
                vec_str cmds = ft_split(client->getPartialRecv() + buff, '\n');
                client->setPartialRecv("");
                
                // Process each command
                for (vec_str::iterator it = cmds.begin(); it != cmds.end(); it++)
                {
                    // Store commands to process
                    str cmd = *it;
                    
                    // Process the command
					if (cmd.empty())
						continue;
                    this->_handler.invoke(client, cmd);
                    
                    // Check if client still exists
                    bool client_exists = false;
                    for (unsigned long i = 0; i < this->_clients.size(); i++)
                    {
                        if (this->_clients[i]->getFD() == client_fd)
                        {
                            client_exists = true;
                            break;
                        }
                    }
                    
                    if (!client_exists)
                    {
                        // Client was deleted, exit the function
                        return;
                    }
                }
            }
            else
            {
                client->setPartialRecv(client->getPartialRecv() + buff);
                if (DEBUG)
                    std::cout << "partial recv(" << client_fd << "): " << buff << std::endl;
            }
        }
    }
}

void	Server::_acceptConnection(void)
{
	int		socket;
	char	ip[INET_ADDRSTRLEN];

	do {
		struct sockaddr_in server_addr;
		socklen_t addrlen = sizeof(server_addr);

		socket = accept(_server_socket, (struct sockaddr*)&server_addr, &addrlen);
		if (socket < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // no more connections
			else if (errno == EINTR)
				continue; // interrupted by signal
			else
			{
				socket = -1;
				break;
			}
			std::cout << "Error: Failed to accept connection." << std::endl;
			break;
		}
		
		if (inet_ntop(AF_INET, &server_addr.sin_addr, ip, sizeof(ip)) == NULL)
		{
			std::cout << "Error: Failed to convert address." << std::endl;
			close(socket);
			break;
		}
		this->addClient(socket, ip, ntohs(server_addr.sin_port));
	} while (socket != -1);
}

void	Server::_receiveData(Client *client)
{
	char	buffer[BUFFER_SIZE + 1];
	bool	client_del = false;

	while(!Server::_signal_recv && !client_del) {
		int ret = recv(client->getFD(), buffer, sizeof(buffer), 0);
		if (ret < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				std::cout << "Error: recv() failed for fd " << client->getFD();
				this->delClient(client->getFD());
				client_del = true;
			}
			// no more data to receive
			break;
		}
		else if (!ret)
		{
			if (DEBUG)
				std::cout << RED << "Error: Connection closed by client " << client->getFD() << WHT << std::endl;
			this->delClient(client->getFD());
			break;
		}
		else
		{
			buffer[ret] = '\0';
			str buff = buffer;


			std::cout << YEL <<"recv(" << client->getFD() << "): " << WHT << buff;
			if (buff.at(buff.size() - 1) == '\n') {
				vec_str cmds = ft_split(client->getPartialRecv() + buff, '\n');
				client->setPartialRecv("");

				for (vec_str::iterator it = cmds.begin(); it != cmds.end(); it++)
					this->_handleMessage(*it, client);
			}
			else
			{
				client->setPartialRecv(client->getPartialRecv() + buff);
				if (DEBUG)
					std::cout << "partial recv(" << client->getFD() << "): " << buff << std::endl;
			}
		}
	}
}

void	Server::_setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cout << "Error: Can't set socket to non-blocking." << std::endl;
		if (fd != this->_server_socket)
			this->delClient(fd);
	}
}

ssize_t	Server::send(str message, int client_fd) const
{
	if (message[message.size() - 1] != '\n')
		message += "\n";

	if (DEBUG)
		std::cout << "send(" << client_fd << "): " << message;

	ssize_t	sent_size = ::send(client_fd, message.c_str(), message.length(), 0);
	if(sent_size != (ssize_t) message.length())
		std::cout << "Error: The message has not been sent entirely." << std::endl;
	return (sent_size);
}

void	Server::broadcast(const str& message) const
{
	for (unsigned long i = 0; i < this->_clients.size(); i++)
	{
		this->send(message, this->_clients[i]->getFD());
	}
}

void	Server::broadcast(const str& message, int exclude_fd) const
{
	for (unsigned long i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getFD() != exclude_fd)
			this->send(message, this->_clients[i]->getFD());
	}
}

int	Server::addClient(int socket, str ip, int port)
{
	str newip = ip;
	if (newip.empty() || newip == "1")
		newip = "127.0.0.1";

	this->_clients.push_back(new Client(this, socket, newip, port));
	this->_setNonBlocking(socket);
	this->_constructFds();
	if (DEBUG)
		std::cout << "* New connection {fd: " << socket
			<< ", ip: " << ip
			<< ", port: " << port
			<< "}" << std::endl;
	return this->_clients.size();
}

int	Server::delClient(int socket)
{
	for (size_t client = 0; client < this->_clients.size(); client++)
	{
		if (this->_clients[client]->getFD() == socket)
		{
			if (DEBUG)
				std::cout << "* Closed connection {fd: " << this->_clients[client]->getFD()
					<< ", ip: " << this->_clients[client]->getHostName()
					<< ", port: " << this->_clients[client]->getPort()
					<< "}" << std::endl;

			// Remove from channels
			for (unsigned long chan = 0; chan < this->_channels.size(); chan++)
			{
				if (this->_channels[chan]->isInChannel(this->_clients[client]))
					this->_channels[chan]->removeClient(this->_clients[client], "");
			}
			delete this->_clients[client];
			this->_clients.erase(this->_clients.begin() + client);
			break;
		}
	}
	this->_constructFds();
	close(socket);
	return this->_clients.size();
}

Client	*Server::getClient(int fd)
{
	for (unsigned long i = 0; i < this->_clients.size(); i++)
	{
		if (this->_clients[i]->getFD() == fd)
			return this->_clients[i];
	}
	return (NULL);
}

Client *Server::getClient(const str &nickname)
{
	std::vector<Client *>::iterator it = _clients.begin();

	while (it != _clients.end())
	{
		if ((*it)->getNickName() == nickname)
			return *it;
		it++;
	}
	return NULL;
}

void	Server::_constructFds(void)
{
	if (this->_clients_fds)
		delete [] this->_clients_fds;
	this->_clients_fds = new struct pollfd[this->_clients.size() + 1];

	this->_clients_fds[0].fd = this->_server_socket;
	this->_clients_fds[0].events = POLLIN;

	for (unsigned long i = 0; i < this->_clients.size(); i++)
	{
		this->_clients_fds[i + 1].fd = this->_clients[i]->getFD();
		this->_clients_fds[i + 1].events = POLLIN;
	}
}

void	Server::_signalHandler(int signum)
{
	std::cout << "Signal " << signum << " caught !" << std::endl; 
	Server::_signal_recv = true;
}

void	Server::setSignalHandler(void)
{
	signal(SIGINT, _signalHandler);
	signal(SIGQUIT, _signalHandler);
	signal(SIGTERM, _signalHandler);
}

Channel *Server::getChannel(const str &name)
{
	std::vector<Channel *>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
		if (it.operator*()->getName() == name)
			return it.operator*();

	return NULL;
}

Channel *Server::createChannel(const str &name, str const &password, Client *client)
{
	Channel *channel = new Channel(name, password, client, this);
	_channels.push_back(channel);

	return channel;
}

void	Server::broadcastChannel(const str& message, Channel const *channel) const
{
	std::vector<Client *> clients = channel->getChanClients();

	for (unsigned long i = 0; i < clients.size(); i++)
		this->send(message, clients[i]->getFD());
}

void	Server::broadcastChannel(str message, int exclude_fd, Channel const *channel) const
{
	std::vector<Client *> clients = channel->getChanClients();

	for (unsigned long i = 0; i < clients.size(); i++)
		if (clients[i]->getFD() != exclude_fd)
			this->send(message, clients[i]->getFD());
}


void	Server::_handleMessage(const str& message, Client *client)
{
	if (DEBUG)
		std::cout << YEL <<"recv(" << client->getFD() << "): " << WHT << message;


	this->_handler.invoke(client, message);
}

vec_str	Server::getNickNames()
{
	vec_str nicknames;
	std::vector<Client *>::iterator it = _clients.begin();

	while (it != _clients.end())
	{
		nicknames.push_back((*it)->getNickName());
		it++;
	}
	return nicknames;
}



str&	Server::getPassword(void) 
{ 
	return _password; 
}

str&	Server::getServerName(void)
{ 
	return _server_name; 
}

str&	Server::getStartTime(void)
{ 
	return _start_time; 
}

