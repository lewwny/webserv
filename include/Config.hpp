#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ConfigParse.hpp"

class Location {
private:
	std::string _path;
	std::string _cgiPath;
	std::string _cgiExtension;
	std::vector<std::string> _methods;
	std::string _root;
	std::string _index;
	std::string _cgiPath;
	std::string _cgiExtension;
	bool _autoindex;
	long _clientMaxBodySize;
	std::string _uploadStore;
public:
	Location(const std::map<std::string, std::string>& locConf);
	~Location();
	// getters
	const std::string& getPath() const { return _path; }
	const std::vector<std::string>& getMethods() const { return _methods; }
	const std::string& getRoot() const { return _root; }
	const std::string& getIndex() const { return _index; }
	const std::string &getCgiPath() const { return _cgiPath; }
	const std::string &getCgiExtension() const { return _cgiExtension; }
	bool isAutoindex() const { return _autoindex; }
	long getClientMaxBodySize() const { return _clientMaxBodySize; }
	const std::string& getUploadStore() const { return _uploadStore; }
	void printLocation() const;
};

class Config {
private:
	std::string _host;
	int _port;
	std::vector<std::string> _serverNames;
	std::string _index;
	std::string _root;
	long _clientMaxBodySize;
	std::map<int, std::string> _errorPages;
	std::vector<Location> _locations;
	// std::map<std::string, std::string> _serverConfig;
	// std::vector<std::map<std::string, std::string> > _locationConfig;
public:
	Config(const ConfigParse& configParse, size_t index);
	~Config();
	// getters
	const std::string& getHost() const { return _host; }
	int getPort() const { return _port; }
	const std::vector<std::string>& getServerNames() const { return _serverNames; }
	const std::string& getIndex() const { return _index; }
	const std::string& getRoot() const { return _root; }
	int getClientMaxBodySize() const { return _clientMaxBodySize; }
	const std::string getErrorPage(int code) const;
	const std::vector<Location>& getLocations() const { return _locations; }
	const Location* getLocationByPath(const std::string& path) const;
	void printConfig() const;
	// const std::map<std::string, std::string>& getServerConfig() const;
	// const std::vector<std::map<std::string, std::string> >& getLocationConfig() const;
	// void printConfig() const;
	// const std::string& getValue(const std::string& key) const;
	// const std::map<std::string, std::string>& getLocation(const std::string& path) const;
	// size_t getLocationCount() const;
	// const std::string& getErrorPage(int code) const;
	// const std::string& getLocationValue(const std::string& path, const std::string& key) const;
};

#endif
