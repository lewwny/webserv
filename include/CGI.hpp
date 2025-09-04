#ifndef CGI_HPP
#define CGI_HPP

#include "Router.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "ConfigParse.hpp"
#include <sys/wait.h>

class CGI
{
public:
	static Response run(const Router::Decision &decision, const Request &req);
};

#endif