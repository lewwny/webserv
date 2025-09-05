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
        const Config*    server;
        const Location*    location;
        Decision() : type(ACTION_ERROR), status(500), autoindex(false),
                     keepAlive(true), uploadEnabled(false),
                     server(0), location(0) {}
    };