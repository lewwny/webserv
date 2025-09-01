/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 17:57:36 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/01 15:37:54 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Response.hpp"

Response::Response( void )
{
	_statusCode = 200;
	_statusMessage = "OK";
	_chunked = false;
	_connectionClose = false;
	_connectionKeepAlive = false;
	setHeader("Server", "webserv/1.0");
	setHeader("Date", ""); // Server should set the actual date when sending the response
	addSecurityHeaders();
}

Response::Response( const Response& other )
{
	*this = other;
}

Response& Response::operator=( const Response& other )
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_statusMessage = other._statusMessage;
		_headers = other._headers;
		_cookies = other._cookies;
		_body = other._body;
		_chunked = other._chunked;
		_connectionClose = other._connectionClose;
		_connectionKeepAlive = other._connectionKeepAlive;
	}
	return *this;
}

Response::~Response( void )
{
}

void	Response::setStatus( int code, const std::string &message )
{
	_statusCode = code;
	_statusMessage = message;
}

void	Response::setHeader( const std::string &key, const std::string &value )
{
	std::string	lower = key;

	// Convert key to lowercase for case-insensitive comparison
	for (size_t i = 0; i < lower.size(); ++i)
		lower[i] = static_cast<char>(tolower(lower[i]));

	// Special cookies handling
	if (lower == "set-cookie")
	{
		addSetCookie(value);
		return ;
	}
	_headers[_canonicalHeaderKey(lower)] = value;
}

void	Response::addHeader( const std::string &key, const std::string &value )
{
	std::string	lower = key;

	// Convert key to lowercase for case-insensitive comparison
	for (size_t i = 0; i < lower.size(); ++i)
		lower[i] = static_cast<char>(tolower(lower[i]));
		
	// Special cookies handling
	if (lower == "set-cookie")
	{
		addSetCookie(value);
		return ;
	}

	// If header already exists, append the new value separated by a comma
	std::map<std::string, std::string>::iterator	it = _headers.find(_canonicalHeaderKey(lower));
	if (it == _headers.end())
	{
		// Header does not exist, insert new entry
		_headers.insert(std::make_pair(_canonicalHeaderKey(lower), value));
	}
	else
	{
		// Append new value with comma separation
		std::string newValue = it->second + ", " + value;
		it->second = newValue;
	}
}

void	Response::addSetCookie( const std::string &cookie )
{
	_cookies.push_back(cookie);
}

void	Response::addCookie( const std::string &name, const std::string &value, const std::string &attributes )
{
	std::ostringstream	ss;
	ss << name << "=" << value;
	if (!attributes.empty())
		ss << "; " << attributes;
	_cookies.push_back(ss.str());
}

void	Response::setBody( const std::string &body )
{
	_body = body;
	if (!_chunked)
	{
		// Update Content-Length header
		setHeader("Content-Length", _itos(_body.size()));
	}
}

void	Response::appendBody( const std::string &data )
{
	_body.append(data);
	if (!_chunked)
	{
		// Update Content-Length header
		setHeader("Content-Length", _itos(_body.size()));
	}
}

void	Response::setChunked( bool chunked )
{
	_chunked = chunked;
	if (_chunked)
	{
		// Remove Content-Length header if present
		std::map<std::string, std::string>::iterator it = _headers.find("Content-Length");
		if (it != _headers.end())
			_headers.erase(it);
		setHeader("Transfer-Encoding", "chunked");
	}
	else
	{
		// Remove Transfer-Encoding header if present
		std::map<std::string, std::string>::iterator it = _headers.find("Transfer-Encoding");
		if (it != _headers.end())
			_headers.erase(it);
		// Update Content-Length header
		setHeader("Content-Length", _itos(_body.size()));
	}
}

void	Response::setConnectionClose( bool close )
{
	_connectionClose = close;
	if (_connectionClose)
	{
		_connectionKeepAlive = false; // mutually exclusive
		setHeader("Connection", "close");
	}
	else
	{
		// Remove Connection header if present
		std::map<std::string, std::string>::iterator it = _headers.find("Connection");
		if (it != _headers.end() && it->second == "close")
			_headers.erase(it);
	}
}

void	Response::setConnectionKeepAlive( bool keepAlive )
{
	_connectionKeepAlive = keepAlive;
	if (_connectionKeepAlive)
	{
		_connectionClose = false; // mutually exclusive
		setHeader("Connection", "keep-alive");
	}
	else
	{
		// Remove Connection header if present
		std::map<std::string, std::string>::iterator it = _headers.find("Connection");
		if (it != _headers.end() && it->second == "keep-alive")
			_headers.erase(it);
	}
}

std::string	Response::headerOnly( void ) const
{
	std::ostringstream	ss;
	
	// Status line (HTTP/1.1 200 OK)
	ss << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";

	// Headers
	std::map<std::string, std::string>::const_iterator	it;
	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		// Skip Date header if empty (it should be set by server when sending)
		if (it->first == "Date" && it->second.empty())
			continue ;
		ss << _canonicalHeaderKey(it->first) << ": " << it->second << "\r\n";
	}

	// Cookies
	for (size_t i = 0; i < _cookies.size(); ++i)
	{
		ss << "Set-Cookie: " << _cookies[i] << "\r\n";
	}

	// If chunked, ensure Transfer-Encoding header is present
	if (_chunked)
	{
		if (_headers.find("Transfer-Encoding") == _headers.end())
			ss << "Transfer-Encoding: chunked\r\n";
	}
	else
	{
		// If not chunked, ensure Content-Length header is present
		if (_headers.find("Content-Length") == _headers.end())
			ss << "Content-Length: " << _body.size() << "\r\n";
	}

	ss << "\r\n"; // End of headers
	return ss.str();
}

std::string	Response::serialize( void ) const
{
	std::string headers = headerOnly();
	
	if (_chunked)
	{
		std::ostringstream	ss;
		ss << std::hex << _body.size() << "\r\n"; // Chunk size in hex
		ss << _body << "\r\n"; // Chunk data
		ss << "0\r\n\r\n"; // Final chunk
		headers += ss.str();
	}
	else
	{
		headers += _body;
	}
	return headers;
}

void	Response::addSecurityHeaders( void )
{
	// Common security headers if not already present
	if (_headers.find("X-Content-Type-Options") == _headers.end())
		setHeader("X-Content-Type-Options", "nosniff");
	if (_headers.find("X-Frame-Options") == _headers.end())
		setHeader("X-Frame-Options", "DENY");
	if (_headers.find("X-XSS-Protection") == _headers.end())
		setHeader("X-XSS-Protection", "1; mode=block");
	if (_headers.find("Content-Security-Policy") == _headers.end())
		setHeader("Content-Security-Policy", "default-src 'self'");
}

std::string	Response::_canonicalHeaderKey( const std::string &key )
{
	std::string	res = key;
	bool		cap = true;

	// Capitalize first letter and letters after hyphens (e.g., "content-type" -> "Content-Type")
	for (size_t i = 0; i < res.size(); ++i)
	{
		if (res[i] == '-')
		{
			cap = true;
			continue ;
		}
		if (cap)
		{
			res[i] = static_cast<char>(toupper(res[i]));
			cap = false;
		}
		else
		{
			res[i] = static_cast<char>(tolower(res[i]));
		}
	}
	return res;
}

std::string	Response::_itos( long v )
{
	std::ostringstream	ss;
	ss << v;
	return ss.str();
}

// Getters
int			Response::getStatusCode( void ) const
{
	return _statusCode;
}

std::string	Response::getBody( void ) const
{
	return _body;
}

std::string	Response::getStatusMessage( void ) const
{
	return _statusMessage;
}

bool		Response::isChunked( void ) const
{
	return _chunked;
}

bool		Response::isConnectionClose( void ) const
{
	return _connectionClose;
}

bool		Response::isConnectionKeepAlive( void ) const
{
	return _connectionKeepAlive;
}

std::map<std::string, std::string>	Response::getHeaders( void ) const
{
	return _headers;
}

