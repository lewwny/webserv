#include "../include/Server.hpp"

Server::Server( void ) : _listenFd(-1)
{
}

Server::~Server( void )
{
	if (_listenFd != -1)
		close(_listenFd);
}

void Server::init( const std::string &host, int port )
{
	struct sockaddr_in	addr;
	struct pollfd		pfd;
	int					option = 1;

	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd == -1)
		throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
		throw std::runtime_error("Setsockopt failed: " + std::string(strerror(errno)));
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	addr.sin_port = htons(port);
	if (bind(_listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
	if (listen(_listenFd, SOMAXCONN) < 0)
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));

	// fcntl to make socket non-blocking
	if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed: " + std::string(strerror(errno)));
	
	pfd.fd = _listenFd;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);

	std::cout << "[Server] Listening on " << port << std::endl;
}

void	Server::run( void )
{
	while (true)
	{
		int n = poll(&_pfds[0], _pfds.size(), -1);
		if ( n < 0 )
		{
			if (errno == EINTR)
				continue; // Interrupted by signal, retry
			throw std::runtime_error("Poll error: " + std::string(strerror(errno)));
		}
		for (long unsigned int i = 0; i < _pfds.size(); i++)
		{
			if ( _pfds[i].revents & POLLIN )
			{
				if ( _pfds[i].fd == _listenFd )
					acceptConnection();
				else
					handleRead(_pfds[i].fd);
			}
			if ( _pfds[i].revents & POLLOUT )
				handleWrite(_pfds[i].fd);	
		}
	}
}

void Server::acceptConnection( void )
{
	int clientFd = accept(_listenFd, NULL, NULL);
	if (clientFd < 0)
		throw std::runtime_error("Accept failed: " + std::string(strerror(errno)));
	// fcntl to make socket non-blocking
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		// do we need to throw here ?
		close(clientFd);
		// throw std::runtime_error("Fcntl failed: " + std::string(strerror(errno)));
	}
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	_pfds.push_back(pfd);
	_conns[clientFd] = Connection();
	_conns[clientFd].fd = clientFd;
	std::cout << "[Server] New connection accepted, fd: " << clientFd << std::endl;
}

void	Server::handleRead( int fd )
{
	char	buffer[4096];
	int		bytesRead;

	bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "Recv error on fd " << fd << ": " << strerror(errno) << std::endl;
			closeConnection(fd);
		}
		return;
	}
	if (bytesRead == 0)
	{
		std::cout << "[Server] Connection closed by peer, fd: " << fd << std::endl;
		closeConnection(fd);
		return;
	}
	// For demonstration, we simply create a placeholder response
	_conns[fd].outBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nHello, World!\n";
	// In a real server, you'd parse the request and generate an appropriate response

	// Modify state to monitor for write readiness
	for (long unsigned int i = 0; i < _pfds.size(); i++)
	{
		if (_pfds[i].fd == fd)
		{
			_pfds[i].events = POLLOUT;
			break ;
		}
	}
}

void	Server::handleWrite( int fd )
{
	int	bytesSent;

	bytesSent = send(fd, _conns[fd].outBuffer.c_str(), _conns[fd].outBuffer.size(), 0);
	if ( bytesSent < 0 )
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "Send error on fd " << fd << ": " << strerror(errno) << std::endl;
			closeConnection(fd);
		}
		return;
	}
	if (bytesSent == 0)
	{
		std::cout << "[Server] Connection closed by peer, fd: " << fd << std::endl;
		closeConnection(fd);
		return;
	}
	_conns[fd].outBuffer.erase(0, bytesSent);
	if (_conns[fd].outBuffer.empty())
	{
		// If the output buffer is empty, switch back to read mode
		for (long unsigned int i = 0; i < _pfds.size(); i++)
		{
			if (_pfds[i].fd == fd)
			{
				_pfds[i].events = POLLIN;
				break ;
			}
		}
	}
}

void Server::closeConnection(int fd)
{
	close(fd);
	_conns.erase(fd);
	for (std::vector<struct pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pfds.erase(it);
			break ;
		}
	}
	std::cout << "[Server] Connection closed, fd: " << fd << std::endl;
}

