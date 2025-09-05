#include "../include/Config.hpp"

Config::Config(const ConfigParse& configParse, size_t index) {
	std::map<std::string, std::string> serverConfig = configParse.getServerConfig(index);
	_host = serverConfig["host"];
	_port = std::atoi(serverConfig["listen"].c_str());
	std::string serverNamesStr = serverConfig["server_name"];
	std::istringstream ss(serverNamesStr);
	std::string serverName;
	while (ss >> serverName) {
		_serverNames.push_back(serverName);
	}
	if (serverConfig.find("index") == serverConfig.end())
		_index = "index.html"; // default index
	else
		_index = serverConfig["index"];
	if (serverConfig.find("root") == serverConfig.end())
		_root = "/www"; // default root
	else
		_root = serverConfig["root"];
	_clientMaxBodySize = 1024 * 1024; // default 1MB
	if (serverConfig.find("client_max_body_size") != serverConfig.end()) {
		std::string sizeStr = serverConfig["client_max_body_size"];
		char unit = sizeStr.empty() ? '\0' : sizeStr[sizeStr.size() - 1];
		long size = std::atol(sizeStr.c_str());
		if (unit == 'K')
			size *= 1024;
		else if (unit == 'M')
			size *= 1024 * 1024;
		else if (unit == 'G')
			size *= 1024 * 1024 * 1024;
		_clientMaxBodySize = size;
	}
	for (std::map<std::string, std::string>::const_iterator it = serverConfig.begin(); it != serverConfig.end(); ++it) {
		if (it->first.find("error_page") == 0) {
			std::string codeStr = it->first.substr(11);
			int code = std::atoi(codeStr.c_str());
			_errorPages[code] = it->second;
		}
	}
	std::vector<std::map<std::string, std::string> > locationConfigs = configParse.getLocationConfig(index);
	for (size_t i = 0; i < locationConfigs.size(); ++i) {
		const std::map<std::string, std::string>& locConf = locationConfigs[i];
		Location loc(locConf);
		_locations.push_back(loc);
	}
}

Config::~Config() {}

const Location* Config::getLocationByPath(const std::string& path) const {
	for (size_t i = 0; i < _locations.size(); ++i) {
		if (_locations[i].getPath() == path) {
			return &_locations[i];
		}
	}
	return NULL;
}

void Config::printConfig() const {
	std::cout << "Server Configuration:" << std::endl;
	std::cout << "Host: " << _host << std::endl;
	std::cout << "Port: " << _port << std::endl;
	std::cout << "Server Names: ";
	for (size_t i = 0; i < _serverNames.size(); ++i) {
		std::cout << _serverNames[i];
		if (i < _serverNames.size() - 1)
			std::cout << ", ";
	}
	std::cout << std::endl;
	std::cout << "Index: " << _index << std::endl;
	std::cout << "Root: " << _root << std::endl;
	std::cout << "Client Max Body Size: " << _clientMaxBodySize << std::endl;
	std::cout << "Error Pages:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Locations:" << std::endl;
	for (size_t i = 0; i < _locations.size(); ++i) {
		std::cout << "Location " << i + 1 << ":" << std::endl;
		_locations[i].printLocation();
	}
}

Location::Location(const std::map<std::string, std::string>& locConf) {
	std::map<std::string, std::string>::const_iterator it;

	it = locConf.find("path");
	if (it == locConf.end())
		throw std::runtime_error("Missing 'path' directive in location block");
	_path = it->second;

	it = locConf.find("allow_methods");
	if (it == locConf.end())
		_methods.push_back("GET"); // default method
	else {
		std::istringstream ss(it->second);
		std::string method;
		while (ss >> method) {
			_methods.push_back(method);
		}
	}

	it = locConf.find("root");
	if (it == locConf.end())
		_root = "/"; // default root
	else
		_root = it->second;

	it = locConf.find("index");
	if (it == locConf.end())
		_index = ""; // default index
	else
		_index = it->second;

	it = locConf.find("autoindex");
	if (it == locConf.end())
		_autoindex = false; // default autoindex off
	else
		_autoindex = (it->second == "on");

	it = locConf.find("client_max_body_size");
	if (it == locConf.end())
		_clientMaxBodySize = 1000000; // default 1MB
	else
	{
		std::string sizeStr = it->second;
		char unit = sizeStr.empty() ? '\0' : sizeStr[sizeStr.size() - 1];
		long size = std::atol(sizeStr.c_str());
		if (unit == 'K')
			size *= 1024;
		else if (unit == 'M')
			size *= 1024 * 1024;
		else if (unit == 'G')
			size *= 1024 * 1024 * 1024;
		_clientMaxBodySize = size;
	}

	it = locConf.find("upload_store");
	if (it == locConf.end())
		_uploadStore = ""; // default no upload store
	else
		_uploadStore = it->second;
	it = locConf.find("cgi_path");
	if (it == locConf.end())
		_cgiPath = ""; // default no cgi path
	else
		_cgiPath = it->second;
	it = locConf.find("cgi_extension");
	if (it == locConf.end())
		_cgiExtension = ""; // default no cgi extension
	else
		_cgiExtension = it->second;
}

Location::~Location() {}

const std::string Config::getErrorPage(int code) const {
	std::map<int, std::string>::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end()) {
		return it->second;
	}
	return "";
}

void Location::printLocation() const {
	std::cout << "  Path: " << _path << std::endl;
	std::cout << "  Methods: ";
	for (size_t i = 0; i < _methods.size(); ++i) {
		std::cout << _methods[i];
		if (i < _methods.size() - 1)
			std::cout << ", ";
	}
	std::cout << std::endl;
	std::cout << "  Root: " << _root << std::endl;
	std::cout << "  Index: " << _index << std::endl;
	std::cout << "  Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
	std::cout << "  Client Max Body Size: " << _clientMaxBodySize << std::endl;
	std::cout << "  Upload Store: " << (_uploadStore.empty() ? "none" : _uploadStore) << std::endl;
}
