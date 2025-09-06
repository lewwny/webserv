
#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <signal.h>
#include "Request.hpp"  // depends on P2
#include "Response.hpp" // depends on P2
#include "Parser.hpp"   // depends on P2
#include "ConfigParse.hpp" // forward declaration removed, include the header
#include "Config.hpp"
#include "Router.hpp"   // depends on P2

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"

class Server
{
public:
	Server(const Config &config );
	~Server( void );

	void init( void ); // create listening socket
	int getListenFd() const { return _listenFd; }
	const Config& getConfig() const { return _config; }
	const std::string& getHost() const { return _host; }
	int getPort() const { return _port; }
private:
	int _listenFd;
	Config _config;
	int _port;
	std::string _host;
	Server(); // prevent default constructor
	Server(const Server &); // prevent copy constructor
	Server &operator=(const Server &); // prevent assignment operator
};

#endif
