#include "RequestExtended.hpp"

// Basic constructor and destructor implementations for testing
Request::Request( void ) : _complete(false), _error(false), _errorCode(0),
    _hasProtocol(false), _hasServerName(false), _hasServerPort(false), _hasRemoteAddr(false) {}

Request::Request( const Request& other ) { *this = other; }

Request& Request::operator=( const Request& other ) {
    if (this != &other) {
        _method = other._method;
        _uri = other._uri;
        _path = other._path;
        _query = other._query;
        _version = other._version;
        _headers = other._headers;
        _body = other._body;
        _complete = other._complete;
        _error = other._error;
        _errorCode = other._errorCode;
        _errorMessage = other._errorMessage;
        _protocol = other._protocol;
        _serverName = other._serverName;
        _serverPort = other._serverPort;
        _remoteAddr = other._remoteAddr;
        _hasProtocol = other._hasProtocol;
        _hasServerName = other._hasServerName;
        _hasServerPort = other._hasServerPort;
        _hasRemoteAddr = other._hasRemoteAddr;
    }
    return *this;
}

Request::~Request( void ) {}

// Basic getters and setters
bool Request::hasHeader( const std::string& key ) const {
    return _headers.find(key) != _headers.end();
}

std::string Request::getHeader( const std::string& key ) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    return (it != _headers.end()) ? it->second : "";
}

void Request::setHeader( const std::string& key, const std::string& value ) {
    _headers[key] = value;
}

void Request::setMethod( const std::string& method ) { _method = method; }
void Request::setUri( const std::string& uri ) { _uri = uri; }
void Request::setVersion( const std::string& version ) { _version = version; }
void Request::setPath( const std::string& path ) { _path = path; }
void Request::setQuery( const std::string& query ) { _query = query; }
void Request::appendToBody( const std::string& data ) { _body += data; }
void Request::setError( void ) { _error = true; }
void Request::setErrorCode( int code ) { _errorCode = code; }
void Request::setErrorMessage( const std::string &message ) { _errorMessage = message; }
void Request::markComplete( void ) { _complete = true; }
bool Request::isComplete( void ) const { return _complete; }
bool Request::getError( void ) const { return _error; }
int Request::getErrorCode( void ) const { return _errorCode; }
void Request::reset( void ) {
    _method.clear();
    _uri.clear();
    _path.clear();
    _query.clear();
    _version.clear();
    _headers.clear();
    _body.clear();
    _complete = false;
    _error = false;
    _errorCode = 0;
    _errorMessage.clear();
    _protocol.clear();
    _serverName.clear();
    _serverPort.clear();
    _remoteAddr.clear();
    _hasProtocol = false;
    _hasServerName = false;
    _hasServerPort = false;
    _hasRemoteAddr = false;
}

const std::string &Request::getErrorMessage( void ) const { return _errorMessage; }
const std::string &Request::getMethod( void ) const { return _method; }
const std::string &Request::getUri( void ) const { return _uri; }
const std::string &Request::getPath( void ) const { return _path; }
const std::string &Request::getQuery( void ) const { return _query; }
const std::string &Request::getVersion( void ) const { return _version; }
const std::string &Request::getBody( void ) const { return _body; }
const std::map<std::string, std::string> &Request::getHeaders( void ) const { return _headers; }

// Additional methods for CGI
bool Request::hasProtocol() const { return _hasProtocol; }
std::string Request::getProtocol() const { return _protocol; }
void Request::setProtocol(const std::string& protocol) { 
    _protocol = protocol; 
    _hasProtocol = true; 
}

bool Request::hasServerName() const { return _hasServerName; }
std::string Request::getServerName() const { return _serverName; }
void Request::setServerName(const std::string& name) { 
    _serverName = name; 
    _hasServerName = true; 
}

bool Request::hasServerPort() const { return _hasServerPort; }
std::string Request::getServerPort() const { return _serverPort; }
void Request::setServerPort(const std::string& port) { 
    _serverPort = port; 
    _hasServerPort = true; 
}

bool Request::hasRemoteAddr() const { return _hasRemoteAddr; }
std::string Request::getRemoteAddr() const { return _remoteAddr; }
void Request::setRemoteAddr(const std::string& addr) { 
    _remoteAddr = addr; 
    _hasRemoteAddr = true; 
}
