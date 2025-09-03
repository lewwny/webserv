
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

// struct Connection
// {
// 	int fd;                     // socket fd
// 	// Parser parser;              // owned parser (P2)
// 	std::string inBuffer;       // raw data read
// 	std::string outBuffer;      // response to send
// 	bool closed;

// 	Connection() : fd(-1), closed(false) {}
// };

class Server
{
public:
	Server(const Config &config );
	~Server( void );

	void init( void ); // create listening socket
	// void run( void );                                   // main poll() loop
	int getListenFd() const { return _listenFd; }
	const Config& getConfig() const { return _config; }
	const std::string& getHost() const { return _host; }
	int getPort() const { return _port; }
private:
	// ConfigParse _config; // server configuration
	int _listenFd;
	Config _config;
	// std::vector<struct pollfd> _pfds;
	int _port;
	std::string _host;
	Server(); // prevent default constructor
	Server(const Server &); // prevent copy constructor
	Server &operator=(const Server &); // prevent assignment operator
	// std::map<int, Connection> _conns;

	// void acceptConnection( void );
	// void handleRead( int fd );
	// void handleWrite( int fd );
	// void closeConnection(int fd);
};

#endif
