/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:16:16 by macauchy          #+#    #+#             */
/*   Updated: 2025/08/26 14:17:54 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Request.hpp"

Request::Request( void ) : _complete(false), _error(false), _errorCode(0)
{
}

Request::Request( const Request& other )
{
	*this = other;
}

Request	&Request::operator=( const Request &other )
{
	if (this != &other)
	{
		_complete = other._complete;
		_error = other._error;
		_errorCode = other._errorCode;
		_errorMessage = other._errorMessage;
		_method = other._method;
		_uri = other._uri;
		_path = other._path;
		_query = other._query;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

Request::~Request( void )
{
}

static std::string	normalizeKey( const std::string &key )
{
	std::string	lowerKey = key;

	std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
	return (lowerKey);
}

bool	Request::hasHeader( const std::string &key ) const
{
	return (_headers.find(normalizeKey(key)) != _headers.end());
}

std::string	Request::getHeader( const std::string &key ) const
{
	std::map<std::string, std::string>::const_iterator	it;

	it = _headers.find(normalizeKey(key));
	if (it != _headers.end())
		return (it->second);
	return ("");
}

void	Request::setHeader( const std::string &key, const std::string &value )
{
	_headers[normalizeKey(key)] = value;
}

void	Request::setMethod( const std::string &method )
{
	_method = method;
}

void	Request::setUri( const std::string &uri )
{
	_uri = uri;
}

void	Request::setVersion( const std::string &version )
{
	_version = version;
}

void	Request::appendToBody( const std::string &data )
{
	_body += data;
}

void	Request::markComplete( void )
{
	_complete = true;
}

bool	Request::isComplete( void ) const
{
	return (_complete);
}

bool	Request::getError( void ) const
{
	return (_error);
}

int	Request::getErrorCode( void ) const
{
	return (_errorCode);
}

std::string	Request::getErrorMessage( void ) const
{
	return (_errorMessage);
}

std::string	Request::getMethod( void ) const
{
	return (_method);
}

std::string	Request::getUri( void ) const
{
	return (_uri);
}

std::string	Request::getPath( void ) const
{
	return (_path);
}

std::string	Request::getQuery( void ) const
{
	return (_query);
}

std::string	Request::getVersion( void ) const
{
	return (_version);
}

std::string	Request::getBody( void ) const
{
	return (_body);
}

void	Request::reset( void )
{
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
}
