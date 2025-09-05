/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 11:43:17 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/05 15:19:37 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <map>
# include <algorithm>
# include <cctype>

/*
** A simple HTTP Request representation

	GET /index.html?user=42 HTTP/1.1
	Host: example.com
	User-Agent: curl/7.68.0
	Accept: text/html
	Body (for POST, etc.)
*/

class Request
{
	public:
		Request( void );
		Request( const Request& other );
		Request& operator=( const Request& other );
		~Request( void );

		bool	hasHeader( const std::string& key ) const;
		std::string	getHeader( const std::string& key ) const;
		void	setHeader( const std::string& key, const std::string& value );
		void	setMethod( const std::string& method );
		void	setUri( const std::string& uri );
		void	setVersion( const std::string& version );
		void	setPath( const std::string& path );
		void	setQuery( const std::string& query );
		void	setPort( int port );
		void	appendToBody( const std::string& data );
		void	setError( void );
		void	setErrorCode( int code );
		void	setErrorMessage( const std::string &message );
		void	markComplete( void );
		bool	isComplete( void ) const;
		bool	getError( void ) const;
		int		getErrorCode( void ) const;
		void	reset( void );

		const std::string	&getErrorMessage( void ) const;
		const std::string	&getMethod( void ) const;
		const std::string	&getUri( void ) const;
		const std::string	&getPath( void ) const;
		const std::string	&getQuery( void ) const;
		const std::string	&getVersion( void ) const;
		const std::string	&getBody( void ) const;
		const std::map<std::string, std::string> &getHeaders( void ) const;

		int			getPort( void ) const;

	private:
		std::string	_method;	// GET, POST, DELETE, etc.
		std::string	_uri;		// /index.html?user=42
		std::string	_path;		// /index.html split from _uri
		std::string	_query;		// user=42 split from _uri
		std::string	_version;	// HTTP/1.1
		int			_listenPort;// port number

		std::map<std::string, std::string>	_headers;	// Host, User-Agent, etc.
		std::string	_body;								// body of the request (for POST, etc.)

		// parser state
		bool		_complete;		// is the request fully received
		bool		_error;			// was there a parsing error
		int			_errorCode;		// HTTP error code if _error is true (e.g., 400, 404)
		std::string	_errorMessage;	// error message if _error is true (e.g., "Bad Request")
};

#endif
