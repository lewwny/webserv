#ifndef ROUTER_HPP
#define ROUTER_HPP

// Ownership: Phase 2 "HTTP/Router" slice.
// Primary owner(s): the team members responsible for routing logic.
// Dependencies: Parser (Request), Response, Config (Route/ServerBlock), Cgi/Upload (only for types).

#include <string>
#include <map>
#include "Parser.hpp"   // Request
#include "Response.hpp" // Response
#include "ConfigParse.hpp" // Route, ServerBlock

class Router
{
	public:
		// No CTOR/DTOR needed, as Router is a static utility class.

		enum ActionType
		{
			ACTION_STATIC,		// Serve static files
			ACTION_REDIRECT,	// HTTP redirect
			ACTION_CGI,			// Execute CGI scripts
			ACTION_UPLOAD,		// Handle file uploads
			ACTION_ERROR,		// Return error response
			ACTION_AUTOINDEX	// Generate directory listing
		};

		// Structure `Decision` encapsulates the routing decision details.
		struct Decision
		{
			ActionType	type;			// Type of action to perform
			int			status;			// HTTP status code (for redirects/errors)
			std::string	reason;			// Reason phrase (for error responses)
			std::string	redirectURL;	// URL to redirect to (for redirects)
			std::string	fsPath;			// Absolute filesystem path (for static files/CGI/uploads)
			std::string	mountUri;		// URI prefix for static files (i.g. "/static", "/cgi-bin")
			std::string	relPath;		// Relative path within the mount point (root)
			std::string	root;			// Effective root directory
			bool		autoindex;		// If dir and autoindex enabled
			bool		keepAlive;		// Connection keep-alive

			// CGI-specific
			std::string	cgiExt;			// CGI file extension (e.g. ".php", ".py")
			std::string	cgiInterpreter;	// CGI interpreter path (i.g. "/usr/bin/php", "/usr/bin/python")
	
			// Upload-specific
			bool		uploadEnabled;	// Is upload enabled for this route
			std::string	uploadStore;	// Directory to store uploaded files

			// Config reference (for logging/debugging)
			const std::map<std::string, std::string>	*server;
			const std::map<std::string, std::string>	*route;

			Decision( void ) :
				type(ACTION_ERROR), status(500), reason("Internal Server Error"),
				redirectURL(""), fsPath(""), mountUri(""), relPath(""), root(""),
				autoindex(false), keepAlive(true),
				cgiExt(""), cgiInterpreter(""),
				uploadEnabled(false), uploadStore(""),
				server(NULL), route(NULL)
			{}
		};

		// Core entry point for routing decisions.
		static Decision	decide( const Request &req, const ConfigParse &cfg );

		// Small helpers
		static bool	normalizePath( std::string &in, std::string &out ); // Normalize path (remove .., ., duplicate slashes)
		static bool	isMethodAllowed( const std::string &method, const std::map<std::string, std::string> *loc,
									 std::string &allowHeaderOut ); // Check if method is allowed, populate Allow header if not
		static const std::map<std::string, std::string>	selectServerByHost( const Request &req, const ConfigParse &cfg ); // Select server block by Host header
		// Find the best matching location block for the request URI
		static const std::map<std::string, std::string>	longestPrefix( const std::string &uri, const std::map<std::string, std::string> srv, std::string &mountOut, std::string &relOut );
};

#endif // ROUTER_HPP
