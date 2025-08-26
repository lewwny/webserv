/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:45:51 by lengarci          #+#    #+#             */
/*   Updated: 2025/08/26 10:25:46 by lengarci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
// #include "Parser.hpp"   // depends on P2
// #include "Response.hpp" // depends on P2

struct Connection
{
	int fd;                     // socket fd
	// Parser parser;              // owned parser (P2)
	std::string inBuffer;       // raw data read
	std::string outBuffer;      // response to send
	bool closed;

	Connection() : fd(-1), closed(false) {}
};

class Server
{
public:
	Server( void );
	~Server( void );

	void init( const std::string& host, int port ); // create listening socket
	void run( void );                                   // main poll() loop

private:
	int _listenFd;
	std::vector<struct pollfd> _pfds;
	std::map<int, Connection> _conns;

	void acceptConnection( void );
	void handleRead( int fd );
	void handleWrite( int fd );
	void closeConnection(int fd);
};

#endif
