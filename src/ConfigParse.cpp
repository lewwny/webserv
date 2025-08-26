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
}

ConfigParse::~ConfigParse() {}

void ConfigParse::loadFile() {
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
		if (isalnum(_fileContent[pos])) {
			size_t start = pos;
			while (pos < _fileContent.size() && (isalnum(_fileContent[pos])
			|| _fileContent[pos] == '_' || _fileContent[pos] == '.'
			|| _fileContent[pos] == '-' || _fileContent[pos] == '/'))
				pos++;
			std::string ident = _fileContent.substr(start, pos - start);
			if (isIdentifier(ident))
				_tokens.push_back(Token(T_IDENT, ident, line));
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

std::string ConfigParse::getValue(const std::string &key) const {
	for (size_t s = 0; s < _config.size(); ++s) {
		std::map<std::string, std::string>::const_iterator it = _config[s].find(key);
		if (it != _config[s].end()) {
			return it->second;
		}
	}
	throw std::runtime_error("Key not found: " + key);
}

void ConfigParse::printTokens() {
	std::vector<std::string> tokenTypeNames;
	tokenTypeNames.resize(6);
	tokenTypeNames[T_EOF] = "T_EOF";
	tokenTypeNames[T_IDENT] = "T_IDENT";
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
	}
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
		if (i >= _tokens.size() || (_tokens[i].type != T_STRING && _tokens[i].type != T_IDENT))
			throw std::runtime_error("Expected value after identifier '" + key + "' at line " + to_string98(_tokens[i - 1].line));
		std::string value = _tokens[i].value;
		i++;
		if (i >= _tokens.size() || _tokens[i].type != T_SEMI)
			throw std::runtime_error("Expected ';' after value '" + value + "' at line " + to_string98(_tokens[i - 1].line));
		i++;
		if (serverCount >= _config.size())
			_config.push_back(std::map<std::string, std::string>());
		_config[serverCount][key] = value;
	}
	if (i >= _tokens.size() || _tokens[i].type != T_RBRACE)
		throw std::runtime_error("Expected '}' to close server block at line " + to_string98(_tokens[i - 1].line));
	i++;
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