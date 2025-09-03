#include "../include/ServerManager.hpp"

ServerManager::ServerManager(ConfigParse& config):_serverCount(config.getServerCount()) {
	for (size_t i = 0; i < _serverCount; ++i) {
		Server server(Config(config, i));
		server.init();
		_servers.push_back(server);
	}
}

ServerManager::~ServerManager() {}