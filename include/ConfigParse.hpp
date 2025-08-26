#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "Server.hpp"

enum TokenType {
	T_EOF,
	T_IDENT,
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
	~ConfigParse();
	void parse();
	std::string getValue(const std::string &key) const;
	void								printTokens();
	void								printConfig() const;
private:
	void												loadFile();
	void												tokenize();
	void												parseServerBlock(size_t &i, size_t &serverCount);
	bool												isIdentifier(const std::string &str) const;
	std::string											_configFile;
	std::string											_fileContent;
	std::vector<Token>									_tokens;
	std::vector<std::string>							_identifiers;
	std::vector<std::map<std::string, std::string> >	_config;
};

#endif