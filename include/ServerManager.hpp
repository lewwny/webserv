#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Config.hpp"
#include "Server.hpp"
#include "ConfigParse.hpp"

class ServerManager {
public:
	ServerManager(ConfigParse& config);
	~ServerManager();

	// void runServers();
private:
	std::vector<Server> _servers;
	size_t _serverCount;
};

#endif