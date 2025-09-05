#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>

// Temporary Router class for testing CGI functionality
class Router {
public:
    enum ActionType {
        ACTION_CGI,
        ACTION_STATIC,
        ACTION_REDIRECT,
        ACTION_DELETE,
        ACTION_ERROR
    };

    struct Decision {
        ActionType type;
        std::string fsPath;         // File system path to the script
        std::string relPath;        // Relative path from mount point
        std::string mountUri;       // Mount URI (e.g., /cgi-bin/)
        std::string root;           // Document root
        std::string cgiExt;         // CGI extension (e.g., .py, .php)
        std::string cgiInterpreter; // CGI interpreter path (e.g., /usr/bin/python3)
        
        Decision() : type(ACTION_ERROR) {}
        
        Decision(ActionType t, const std::string& fs, const std::string& rel, 
                const std::string& mount, const std::string& rt, 
                const std::string& ext, const std::string& interp)
            : type(t), fsPath(fs), relPath(rel), mountUri(mount), 
              root(rt), cgiExt(ext), cgiInterpreter(interp) {}
    };
};

#endif
