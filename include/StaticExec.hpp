#ifndef STATIC_EXEC_HPP

#define STATIC_EXEC_HPP

#include "Router.hpp"
#include "Response.hpp"
#include "ConfigParse.hpp"


// Static file execution (in-Router, no blocking I/O):
class StaticExec 
{
					public:
						// Produce a static file response (no blocking I/O):
						static Response makeRedirect(const Router::Decision& d);
						static Response makeError(const Router::Decision& d, const ConfigParse &cfg);
						static Response serveStatic(const Router::Decision& d, const ConfigParse &cfg);
};

#endif