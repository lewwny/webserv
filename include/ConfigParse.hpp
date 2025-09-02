#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

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

enum TokenType {
	T_EOF,
	T_IDENT,
	T_LIDENT,
	T_STRING,
	T_LBRACE,
	T_RBRACE,
	T_SEMI,
};

struct Token {
	TokenType type;
	std::string value;
	int line;

	Token(TokenType t, const std::string &v, int l) : type(t), value(v), line(l) {}
};

class ConfigParse {
public:
	ConfigParse(const std::string &configFile);
	ConfigParse( void );
	~ConfigParse();
	void parse();
	int getServerCount() const;
	const std::map<std::string, std::string> &getServerConfig(size_t index) const;
	const std::vector<std::map<std::string, std::string> > &getLocationConfig(size_t index) const;
	void printTokens();
	void printConfig() const;
	const std::string& getErrorPagePath(const int code, size_t serverIndex) const;
private:
	void						loadFile();
	void						tokenize();
	void						parseServerBlock(size_t &i, size_t &serverCount);
	void						parseLocationBlock(size_t &i, size_t &serverCount);
	bool						isIdentifier(const std::string &str) const;
	bool						isLocationIdentifier(const std::string &str) const;
	void						checkListenPort(const std::string &portStr) const;
	void						checkErrorCode(const std::string &codeStr) const;
	void						checkConfig(std::map<std::string, std::string> &config);
	void						checkClientMaxBodySize(const std::string &sizeStr) const;
	std::string					_configFile;
	std::string					_fileContent;
	std::vector<Token>			_tokens;
	std::vector<std::string>	_identifiers;
	std::vector<std::string>	_locationIdentifiers;
	std::vector<std::map<std::string, std::string> >	_config;
	std::vector<std::vector<std::map<std::string, std::string> > > _locations;
};

#endif
