/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:46:04 by lengarci          #+#    #+#             */
/*   Updated: 2025/09/03 16:03:26 by lengarci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include <sstream>

template<typename T>
static std::string toString(T n)
{
	std::ostringstream oss;
	oss << n;
	return oss.str();
}


Server::Server( const Config& config ) : _listenFd(-1), _config(config)
{
}

Server::~Server( void )
{
	if (_listenFd != -1)
		close(_listenFd);
}

void Server::init( void )
{
	struct sockaddr_in	addr;
	// struct pollfd		pfd;
	int					option = 1;

	std::string host = _config.getHost();
	int port = _config.getPort();
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
	{
		close(_listenFd);
		_listenFd = -1;
		std::cerr << "Bind failed on " << host << ":" << port << " - " << strerror(errno) << std::endl;
		return ;
	}
		// throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
	if (listen(_listenFd, SOMAXCONN) < 0)
		throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));

	// fcntl to make socket non-blocking
	if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed: " + std::string(strerror(errno)));
	// pfd.fd = _listenFd;
	// pfd.events = POLLIN;
	// _pfds.push_back(pfd);
	
	std::cout << "[Server] Listening on " << port << std::endl;
	_host = host;
	_port = port;
	std::cout << "[DEBUG] Host: " << _host << ", Port: " << _port << ", listen fd: " << _listenFd << std::endl;
}
