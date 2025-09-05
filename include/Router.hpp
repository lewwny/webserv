#ifndef ROUTER_HPP
#define ROUTER_HPP

// Ownership: Phase 2 "HTTP/Router" slice.
// Primary owner(s): the team members responsible for routing logic.
// Dependencies: Parser (Request), Response, Config (Route/ServerBlock), Cgi/Upload (only for types).

#include <string>
#include <map>
#include <sys/stat.h> // stat
#include "Parser.hpp"   // Request
#include "Response.hpp" // Response
#include "Config.hpp"   // Route, ServerBlock, Config

// Forward declarations to break circular dependencies
class Server;
class ServerManager;

// The Router *does not* execute CGI or write uploads itself. It *decides* the action.
// The event-loop (Server) will execute non-blocking actions (CGI pipes, disk streaming).

class Router {
public:
	// What action should be performed for this request?
	enum ActionType {
		ACTION_STATIC,
		ACTION_AUTOINDEX,
		ACTION_REDIRECT,
		ACTION_CGI,
		ACTION_UPLOAD,
		ACTION_ERROR
	};

	struct Decision {
		ActionType type;
		int        status;            // for errors (and allowed to prefill 3xx/4xx/5xx)
		std::string reason;           // short message (e.g., "Not Found")
		std::string redirectURL;      // for ACTION_REDIRECT
		std::string fsPath;           // resolved filesystem path for static/CGI/upload
		std::string mountUri;         // the matched location prefix (e.g., "/cgi-bin")
		std::string relPath;          // URI relative to location root
		std::string root;             // route root
		std::string index;            // route index (if any)
		bool        autoindex;        // route autoindex
		bool        keepAlive;        // whether to keep connection open
		// CGI
		std::string cgiInterpreter;   // e.g. "/usr/bin/php-cgi"
		std::string cgiExt;           // e.g. ".php"
		// Upload
		bool        uploadEnabled;
		std::string uploadStore;      // target dir for uploads
		// Which server block was selected (optional, for further use)
		const Config*	server;
		const Location*	location;
		Decision() : type(ACTION_ERROR), status(500), autoindex(false),
					 keepAlive(true), uploadEnabled(false),
					 server(0), location(0) {}
	};

	// Decide the action to perform. (No blocking I/O here.)
	static Decision decide(const Request& req, const ServerManager &sm);

	// Immediate producers (in-Router), no blocking on pipes:
	static Response makeRedirect(const Decision& d);
	static Response makeError(const Decision& d,
							  const Server* sb,
							  int code, const std::string& msg);

	// Static file/dir: build a complete Response (reads file; small blocking read).
	// In your final version, you should stream via the event loop; for the first milestone this is fine.
	static Response serveStatic(const Decision& d);

	// Cookie/session utility (bonus-friendly):
	// - If request has no "sid" cookie, create one and attach Set-Cookie.
	// - Optionally update a tiny in-memory session counter.
	static void attachSessionCookie(const Request& req, Response& res);

private:
	// Helpers
	static const Server			*selectServer( const Request &req, const ServerManager &sm );
	static const Location		*matchRoute(const std::string& uri, const Server* sb,
										std::string& mountUriOut, std::string& relPathOut);
	static bool               isMethodAllowed( const std::string &method, const Location *loc,
										std::string &allowOut );
	static bool               hasCgi(const Location* r, const std::string& relPath,
										std::string& matchedExt, std::string& interp);
	static bool               isDir(const std::string& p);
	static bool               isFile(const std::string& p);
	static bool               pathJoin(const std::string& base, const std::string& rel, std::string& out);
	static bool               normalizePath( const std::string &in, std::string &out);
	static std::string        guessMime(const std::string& path);
	static std::string        autoindexHtml(const std::string& dir, const std::string& reqUri);
	static std::string        getHeader(const Request& req, const std::string& key);
};

#endif // ROUTER_HPP
