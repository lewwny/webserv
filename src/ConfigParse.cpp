#include "../include/ConfigParse.hpp"

static std::string to_string98(long v) {
	std::ostringstream oss;
	oss << v;
	return oss.str();
}

ConfigParse::ConfigParse(const std::string &configFile) : _configFile(configFile) {
	_identifiers.push_back("server");
	_identifiers.push_back("listen");
	_identifiers.push_back("server_name");
	_identifiers.push_back("index");
	_identifiers.push_back("error_page");
	_identifiers.push_back("location");
	_identifiers.push_back("client_max_body_size");
	_locationIdentifiers.push_back("autoindex");
	_locationIdentifiers.push_back("allow_methods");
}

ConfigParse::~ConfigParse() {}

void ConfigParse::loadFile() {
	if (_configFile.empty() || _configFile.size() < 6 || _configFile.substr(_configFile.size() - 5) != ".conf") {
		throw std::runtime_error("Config file path is invalid");
	}
	std::ifstream file(_configFile.c_str());
	if (!file.is_open()) {
		throw std::runtime_error("Could not open config file: " + _configFile);
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	_fileContent = buffer.str();
	file.close();
}

bool ConfigParse::isIdentifier(const std::string &str) const {
	return std::find(_identifiers.begin(), _identifiers.end(), str) != _identifiers.end();
}

bool ConfigParse::isLocationIdentifier(const std::string &str) const {
	return std::find(_locationIdentifiers.begin(), _locationIdentifiers.end(), str) != _locationIdentifiers.end();
}

void ConfigParse::checkListenPort(const std::string &portStr) const {
	for (size_t i = 0; i < portStr.size(); ++i) {
		if (!isdigit(portStr[i])) {
			throw std::runtime_error("Invalid port number: " + portStr);
		}
	}
	int port = std::atoi(portStr.c_str());
	if (port < 1 || port > 65535) {
		throw std::runtime_error("Port number out of range: " + portStr);
	}
}

void ConfigParse::checkErrorCode(const std::string &codeStr) const {
	for (size_t i = 0; i < codeStr.size(); ++i) {
		if (!isdigit(codeStr[i])) {
			throw std::runtime_error("Invalid error code: " + codeStr);
		}
	}
	if (codeStr.size() != 3)
		throw std::runtime_error("Error code must be 3 digits: " + codeStr);
	int code = std::atoi(codeStr.c_str());
	if (code < 100 || code > 599)
		throw std::runtime_error("Error code out of range (100-599): " + codeStr);
}

void ConfigParse::tokenize() {
	size_t pos = 0;
	int line = 1;
	while (pos < _fileContent.size()) {
		if (isspace(_fileContent[pos])) {
			if (_fileContent[pos] == '\n') line++;
			pos++;
			continue;
		}
		if (_fileContent[pos] == '#') {
			while (pos < _fileContent.size() && _fileContent[pos] != '\n')
				pos++;
			continue;
		}
		if (isalnum(_fileContent[pos]) || _fileContent[pos] == '/' || _fileContent[pos] == '.') {
			size_t start = pos;
			while (pos < _fileContent.size() && (isalnum(_fileContent[pos])
			|| _fileContent[pos] == '_' || _fileContent[pos] == '.'
			|| _fileContent[pos] == '-' || _fileContent[pos] == '/'))
				pos++;
			std::string ident = _fileContent.substr(start, pos - start);
			if (isIdentifier(ident))
				_tokens.push_back(Token(T_IDENT, ident, line));
			else if (isLocationIdentifier(ident))
				_tokens.push_back(Token(T_LIDENT, ident, line));
			else
				_tokens.push_back(Token(T_STRING, ident, line));
			continue;
		}
		if (_fileContent[pos] == '{') {
			_tokens.push_back(Token(T_LBRACE, "{", line));
			pos++;
			continue;
		}
		if (_fileContent[pos] == '}') {
			_tokens.push_back(Token(T_RBRACE, "}", line));
			pos++;
			continue;
		}
		if (_fileContent[pos] == ';') {
			_tokens.push_back(Token(T_SEMI, ";", line));
			pos++;
			continue;
		}
		throw std::runtime_error(std::string("Unexpected character: ") + _fileContent[pos]);
	}
	_tokens.push_back(Token(T_EOF, "", line));
}

void ConfigParse::printTokens() {
	std::vector<std::string> tokenTypeNames;
	tokenTypeNames.resize(7);
	tokenTypeNames[T_EOF] = "T_EOF";
	tokenTypeNames[T_IDENT] = "T_IDENT";
	tokenTypeNames[T_LIDENT] = "T_LIDENT";
	tokenTypeNames[T_STRING] = "T_STRING";
	tokenTypeNames[T_LBRACE] = "T_LBRACE";
	tokenTypeNames[T_RBRACE] = "T_RBRACE";
	tokenTypeNames[T_SEMI] = "T_SEMI";
	for (size_t i = 0; i < _tokens.size(); i++) {
		std::cout << "Token " << i << ": Type = " << tokenTypeNames[_tokens[i].type] << ", Value = '" << _tokens[i].value << "', Line = " << _tokens[i].line << std::endl;
	}
}

void ConfigParse::printConfig() const {
	for (size_t s = 0; s < _config.size(); ++s) {
		std::cout << "Server " << s << " config:" << std::endl;
		for (std::map<std::string, std::string>::const_iterator it = _config[s].begin(); it != _config[s].end(); ++it) {
			std::cout << "  " << it->first << " = " << it->second << std::endl;
		}
		if (s < _locations.size() && !_locations[s].empty()) {
			for (size_t l = 0; l < _locations[s].size(); ++l) {
				std::cout << "  Location " << l << " config:" << std::endl;
				for (std::map<std::string, std::string>::const_iterator lit = _locations[s][l].begin(); lit != _locations[s][l].end(); ++lit) {
					std::cout << "    " << lit->first << " = " << lit->second << std::endl;
				}
			}
		}
	}
}

void ConfigParse::checkConfig(std::map<std::string, std::string> &config) {
	std::map<std::string, std::string>::const_iterator it;

	it = config.find("listen");
	if (it == config.end())
		throw std::runtime_error("Missing 'listen' directive in server block");
	checkListenPort(it->second);

	it = config.find("server_name");
	if (it == config.end())
		config["server_name"] = "localhost";

	it = config.find("index");
	if (it == config.end())
		throw std::runtime_error("Missing 'index' directive in server block");
	for (std::map<std::string, std::string>::const_iterator it = config.begin(); it != config.end(); ++it) {
		if (it->first.find("error_page") == 0) {
			std::string codeStr = it->first.substr(11); // after "error_page "
			checkErrorCode(codeStr);
		}
	}
}

static void CheckDuplicateLocation(const std::string &path, const std::vector<std::map<std::string, std::string> > &locations) {
	for (size_t i = 0; i < locations.size(); ++i) {
		std::map<std::string, std::string>::const_iterator it = locations[i].find("path");
		if (it != locations[i].end() && it->second == path) {
			throw std::runtime_error("Duplicate location path: " + path);
		}
	}
}

void ConfigParse::checkClientMaxBodySize(const std::string &sizeStr) const {
	for (size_t i = 0; i < sizeStr.size() - 1; ++i) {
		if (!isdigit(sizeStr[i])) {
			throw std::runtime_error("Invalid client_max_body_size: " + sizeStr);
		}
	}
	if (!isdigit(sizeStr[sizeStr.size() - 1]) && sizeStr[sizeStr.size() - 1] != 'K' && sizeStr[sizeStr.size() - 1] != 'M' && sizeStr[sizeStr.size() - 1] != 'G') {
		throw std::runtime_error("Invalid client_max_body_size: " + sizeStr);
	}
	long size = std::atol(sizeStr.c_str());
	if (size < 0)
		throw std::runtime_error("client_max_body_size must be non-negative: " + sizeStr);
}

void ConfigParse::parseLocationBlock(size_t &i, size_t &serverCount) {
	if (i >= _tokens.size() || _tokens[i].type != T_STRING)
		throw std::runtime_error("Expected path after 'location' at line " + to_string98(_tokens[i - 1].line));
	std::string path = _tokens[i].value;
	i++;
	if (i >= _tokens.size() || _tokens[i].type != T_LBRACE)
		throw std::runtime_error("Expected '{' after 'location' at line " + to_string98(_tokens[i - 1].line));
	i++;
	std::map<std::string, std::string> locationConfig;
	while (i < _tokens.size() && _tokens[i].type != T_RBRACE) {
		if (_tokens[i].type != T_LIDENT)
			throw std::runtime_error("Expected identifier in location block at line " + to_string98(_tokens[i].line));
		std::string key = _tokens[i].value;
		i++;
		if (i >= _tokens.size() || (_tokens[i].type != T_STRING && _tokens[i].type != T_LIDENT))
			throw std::runtime_error("Expected value after identifier '" + key + "' at line " + to_string98(_tokens[i - 1].line));
		std::string value = _tokens[i].value;
		if (key == "autoindex" && value != "on" && value != "off")
			throw std::runtime_error("Invalid value for autoindex: " + value + " at line " + to_string98(_tokens[i].line));
		i++;
		while (i < _tokens.size() && _tokens[i].type != T_SEMI)
		{
			if (key == "allow_methods" && _tokens[i].type == T_STRING) {
				value += " " + _tokens[i].value;
				i++;
			}
			else
				throw std::runtime_error("Expected ';' after value '" + value + "' at line " + to_string98(_tokens[i - 1].line));
		}
		i++;
		if (locationConfig.find(key) != locationConfig.end())
			throw std::runtime_error("Duplicate directive '" + key + "' in location block at line " + to_string98(_tokens[i - 1].line));
		locationConfig[key] = value;
	}
	if (i >= _tokens.size() || _tokens[i].type != T_RBRACE)
		throw std::runtime_error("Expected '}' to close location block at line " + to_string98(_tokens[i - 1].line));
	i++;
	if (serverCount >= _locations.size())
		_locations.push_back(std::vector<std::map<std::string, std::string> >());
	CheckDuplicateLocation(path, _locations[serverCount]);
	locationConfig["path"] = path;
	_locations[serverCount].push_back(locationConfig);
}

void ConfigParse::parseServerBlock(size_t &i, size_t &serverCount) {
	if (i >= _tokens.size() || _tokens[i].type != T_LBRACE)
		throw std::runtime_error("Expected '{' after 'server' at line " + to_string98(_tokens[i - 1].line));
	i++;
	while (i < _tokens.size() && _tokens[i].type != T_RBRACE) {
		if (_tokens[i].type != T_IDENT)
			throw std::runtime_error("Expected identifier in server block at line " + to_string98(_tokens[i].line));
		std::string key = _tokens[i].value;
		i++;
		if (key == "location") {
			parseLocationBlock(i, serverCount);
			continue;
		}
		if (i >= _tokens.size() || (_tokens[i].type != T_STRING && _tokens[i].type != T_IDENT))
			throw std::runtime_error("Expected value after identifier '" + key + "' at line " + to_string98(_tokens[i - 1].line));
		std::string value = _tokens[i].value;
		i++;
		if (_tokens[i].type == T_STRING && _tokens[i - 2].value == "error_page") {
			value = _tokens[i].value;
			key += " " + _tokens[i - 1].value;
			i++;
		}
		if (key == "client_max_body_size")
			checkClientMaxBodySize(value);
		else if (_tokens[i].type != T_STRING && _tokens[i - 2].value == "error_page") {
			throw std::runtime_error("Expected 2 string after 'error_page' at line " + to_string98(_tokens[i - 1].line));
		}
		if (i >= _tokens.size() || _tokens[i].type != T_SEMI)
			throw std::runtime_error("Expected ';' after value '" + value + "' at line " + to_string98(_tokens[i - 1].line));
		i++;
		if (serverCount >= _config.size())
			_config.push_back(std::map<std::string, std::string>());
		if (_config[serverCount].find(key) != _config[serverCount].end())
			throw std::runtime_error("Duplicate directive '" + key + "' in server block at line " + to_string98(_tokens[i - 1].line));
		_config[serverCount][key] = value;
	}
	if (i >= _tokens.size() || _tokens[i].type != T_RBRACE)
		throw std::runtime_error("Expected '}' to close server block at line " + to_string98(_tokens[i - 1].line));
	i++;
	checkConfig(_config[serverCount]);
}

void ConfigParse::parse() {
	loadFile();
	tokenize();
	if (_tokens.empty())
		throw std::runtime_error("No tokens to parse");
	size_t i = 0;
	size_t serverCount = 0;
	while (1)
	{
		if (_tokens[i].type != T_IDENT || _tokens[i].value != "server")
			throw std::runtime_error("Config must start with 'server' block");
		i++;
		parseServerBlock(i, serverCount);
		if (_tokens[i].type == T_EOF)
			break;
		serverCount++;
	}
}

int ConfigParse::getServerCount() const {
	return _config.size();
}

const std::map<std::string, std::string> &ConfigParse::getServerConfig(size_t index) const {
	if (index >= _config.size())
		throw std::runtime_error("Server index out of range");
	return _config[index];
}

const std::vector<std::map<std::string, std::string> > &ConfigParse::getLocationConfig(size_t index) const {
	if (index >= _locations.size())
		throw std::runtime_error("Location index out of range");
	return _locations[index];
}
