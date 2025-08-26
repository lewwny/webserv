#include "../include/ConfigParse.hpp"

ConfigParse::ConfigParse(const std::string &configFile) : _configFile(configFile) {}

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
			while (pos < _fileContent.size() && (isalnum(_fileContent[pos]) || _fileContent[pos] == '_'))
				pos++;
			_tokens.push_back(Token(T_IDENT, _fileContent.substr(start, pos - start), line));
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