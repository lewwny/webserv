/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 17:57:56 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/05 19:42:02 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <vector>
# include <map>
# include <sstream>

class Response
{
	public:
		Response( void );
		Response( const Response& other );
		Response& operator=( const Response& other );
		~Response( void );

		void	setStatus( int code, const std::string &message );
		void	setStatusCode( int code );
		void	setStatusMessage( const std::string &message );

		void	setHeader( const std::string &key, const std::string &value );
		void	addHeader( const std::string &key, const std::string &value );

		// cookies : Add a full cookie string (e.g. "sessionId=abc123; Path=/; HttpOnly")
		void	addSetCookie( const std::string &cookie ); // Add a Set-Cookie header
		
		// Build cookie string from name, value and attributes (e.g. "Path=/; HttpOnly")
		void	addCookie( const std::string &name, const std::string &value, const std::string &attributes );

		// Body management
		void	setBody( const std::string &body );
		void	appendBody( const std::string &data );
		
		
		// Serialize the response to a raw HTTP response string
		// serializing is a process of converting the response object into
		// a string format that can be sent over the network as an HTTP response.
		// if _chunked true, serialize will not add Content-Length header
		void	setChunked( bool chunked );

		// Connection management
		void	setConnectionClose( bool close );
		void	setConnectionKeepAlive( bool keepAlive );

		// Serialize the response to a raw HTTP response string
		std::string	serialize( void ) const;
		std::string	headerOnly( void ) const;

		// Security headers
		void	addSecurityHeaders( void );

		// Getters
		int			getStatusCode( void ) const;
		std::string	getStatusMessage( void ) const;
		std::string	getBody( void ) const;
		bool		isChunked( void ) const;
		bool		isConnectionClose( void ) const;
		bool		isConnectionKeepAlive( void ) const;
		std::map<std::string, std::string>	getHeaders( void ) const;

	private:
		int			_statusCode;
		std::string	_statusMessage;
		std::map<std::string, std::string> _headers;
		std::vector<std::string> _cookies; // Each full cookie line (i.e., "sessionId=abc123; Path=/; HttpOnly")
		std::string	_body;
		bool		_chunked;
		bool		_connectionClose;
		bool		_connectionKeepAlive;

		static std::string	_canonicalHeaderKey( const std::string &key ); // to prevent duplication of headers with different cases (i.e., "Content-Type" vs "content-type")

		static std::string	_itos( long v );
};

#endif
