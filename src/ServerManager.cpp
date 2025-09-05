#include "../include/ServerManager.hpp"

template<typename T>
static std::string toString(T n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}

ServerManager::ServerManager(ConfigParse& config):_serverCount(config.getServerCount()) {
	_servers.reserve(_serverCount);
	for (size_t i = 0; i < _serverCount; ++i) {
		Server* server = new Server(Config(config, i));
		server->init();
		_servers.push_back(server);

		struct pollfd pfd;
		pfd.fd = server->getListenFd();
		if (pfd.fd != -1) {
			pfd.events = POLLIN;
			pfd.revents = 0;
			_pfds.push_back(pfd);
			_listenFdToServerIndex[pfd.fd] = i;
		} else {
			// std::cerr << "Warning: Server on " << server->getHost() << ":" << server->getPort() << " failed to initialize properly." << std::endl;
		}
	}
	_serverShutdown = false;
}

ServerManager::~ServerManager() {
	for (size_t i = 0; i < _servers.size(); ++i) {
		delete _servers[i];
	}
}

void ServerManager::run()
{
	while (!_serverShutdown)
	{
		int n = poll(&_pfds[0], _pfds.size(), -1);
		if ( n < 0 )
		{
			if (errno == EINTR)
				continue; // Interrupted by signal, retry
			throw std::runtime_error("Poll error: " + std::string(strerror(errno)));
		}
		for (size_t i = 0; i < _pfds.size(); i++)
		{
			if ( _pfds[i].revents & POLLIN )
			{
				bool isListenFd = false;
				for (size_t j = 0; j < _servers.size(); ++j) {
					if ( _listenFdToServerIndex.find(_pfds[i].fd) != _listenFdToServerIndex.end()) {
						acceptConnection(_pfds[i].fd);
						isListenFd = true;
						break;
					}
				}
				if (!isListenFd)
					handleRead(_pfds[i].fd);
			}
			if ( _pfds[i].revents & POLLOUT )
				handleWrite(_pfds[i].fd);	
		}
	}
	for (size_t i = 0; i < _pfds.size(); ++i) {
		if (_listenFdToServerIndex.find(_pfds[i].fd) == _listenFdToServerIndex.end()) {
			closeConnection(_pfds[i].fd);
		}
	}
	for (size_t i = 0; i < _conns.size(); ++i) {
		if (_conns[i].fd != -1) {
			close(_conns[i].fd);
		}
	}
	std::cout << "[ServerManager] Shutdown complete." << std::endl;
}


void ServerManager::acceptConnection( int fd )
{
	int clientFd = accept(fd, NULL, NULL);
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
	pfd.revents = 0;
	_pfds.push_back(pfd);
	Connection conn;
	conn.fd = clientFd;
	_conns[clientFd] = conn;
	std::map<int, size_t>::iterator it = _listenFdToServerIndex.find(fd);
	if (it != _listenFdToServerIndex.end()) {
		_connFdToServerIndex[clientFd] = it->second;
	}
	std::cout << "[Server] New connection accepted, fd: " << clientFd << std::endl;
}

void ServerManager::closeConnection(int fd)
{
	close(fd);
	_conns.erase(fd);
	_connFdToServerIndex.erase(fd);
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

void	ServerManager::handleWrite( int fd )
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

void	ServerManager::handleRead( int fd )
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
	// // For demonstration, we simply create a placeholder response
	// _conns[fd].outBuffer = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\n\r\nHello, World!\n";
	// // In a real server, you'd parse the request and generate an appropriate response

	_conns[fd].inBuffer.append(buffer, bytesRead);
	std::cout << "[Server] Received " << bytesRead << " bytes on fd " << fd << std::endl;
	Parser parser;
	parser.setLimits(8192, 1048576, 4096); // Example limits, should get them from server._config
	if (parser.feed(_conns[fd].inBuffer))
	{
		Request req = parser.getRequest();
		if (req.getError())
		{
			// Handle request error
			_conns[fd].outBuffer = "HTTP/1.1 " + toString(req.getErrorCode()) + " " + req.getErrorMessage() + "\r\nContent-Length: 0\r\n\r\n";
		}
		else
		{
			std::cerr << "[Server] Request parsed successfully, fd: " << fd << std::endl;
			// Simple echo response for demonstration (TODO: real routing/handling)
			std::string body = "You requested: " + req.getPath() + "\n";
			_conns[fd].outBuffer = "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.length()) + "\r\n\r\n" + body;
			std::cout << "[DEBUG] outBuffer: " << std::endl << _conns[fd].outBuffer << std::endl;
		}
		_conns[fd].inBuffer.clear(); // Clear input buffer after processing
		std::map<std::string, std::string> headers = req.getHeaders();
		// std::cout << "[DEBUG] Parsed Headers:" << std::endl;
		// for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
		// 	std::cout << it->first << ": " << it->second << std::endl;
		// }
		// std::cout << "[DEBUG] End of Headers" << std::endl;
	}
	else if (parser.isComplete() && !parser.getRequest().getError())
	{
		// Request is complete but no error, should not happen here
	}
	else
	{
		// Incomplete request, wait for more data
		return;
	}

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

volatile sig_atomic_t ServerManager::_serverShutdown = false;
ServerManager* ServerManager::_instance = NULL;

void ServerManager::handleSignal(int signum) {
	if (signum == SIGINT) {
		if (_instance) {
			_instance->requestShutdown();
		}
	}
}