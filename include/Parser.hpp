/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 11:42:38 by macauchy          #+#    #+#             */
/*   Updated: 2025/08/28 16:51:10 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <map>
# include <sstream>
# include <stdexcept>
# include <algorithm>
# include <cctype>
# include <iostream>
# include <vector>
# include "Request.hpp"
# include <exception>

/**
 * @class Parser
 * @brief HTTP request parser for the web server.
 *
 * The Parser class is responsible for incrementally parsing raw HTTP request data
 * received by the server. It processes the request line, headers, and body, and
 * manages the parsing state throughout the lifecycle of a request. The class supports
 * both standard Content-Length and chunked Transfer-Encoding bodies.
 *
 * @note This class is designed to be used internally by the server to convert incoming
 *       raw data streams into structured Request objects for further processing.
 *
 * @section Usage
 * Typical usage involves feeding raw data to the parser, which accumulates and parses
 * the data as it arrives. The parser maintains its state and can handle incomplete
 * requests, waiting for more data as needed.
 *
 * @section States
 * The parser transitions through several states:
 *   - REQ_LINE: Parsing the initial request line (method, URI, version).
 *   - HEADERS: Parsing HTTP headers.
 *   - BODY: Parsing the request body, if present.
 *   - COMPLETE: Parsing is finished and the request is ready.
 *   - ERROR: An error occurred during parsing.
 *
 * @section ThreadSafety
 * This class is not thread-safe. Each instance should be used by a single thread only.
 *
 * @author macauchy
 * @date 2025-08-26
 */
class Parser
{
	public:
		Parser( void );
		Parser( const Parser& other );
		Parser& operator=( const Parser& other );
		~Parser( void );

		bool	feed( const std::string &data );	// Feed raw data to the parser
		bool	isComplete( void ) const;			// Is the current request fully parsed
		Request	getRequest( void ) const;			// Get the parsed request
		void	reset( void );						// Reset parser state for a new request
		void	setLimits( size_t headerBytes, size_t bodyBytes, size_t lineBytes ); // Set size limits

	private:
		enum State {
			REQ_LINE,
			HEADERS,
			BODY,
			COMPLETE,
			ERROR
		};

		enum ChunkState {
			CHUNK_SIZE,
			CHUNK_DATA,
			CHUNK_CRLF,
			CHUNK_TRAILERS
		};

		State		_state;					// Current parsing state
		ChunkState	_chunkState;			// Current chunk parsing state
		std::string	_buffer;				// Accumulated raw data
		Request		_current;				// Current request being parsed
		size_t		_expectedBodyLength;	// Expected body length from Content-Length
		bool		_chunked;				// Is Transfer-Encoding: chunked
		size_t		_chunkBytesRemaining;	// Bytes remaining in the current chunk

		size_t		_limitHeaderBytes;		// Max bytes for headers to prevent abuse
		size_t		_limitBodyBytes;		// Max bytes for body to prevent abuse
		size_t		_limitLineBytes;		// Max bytes for a single line

		void	_parseRequestLine( const std::string &line );
		void	_parseHeaderBlock( const std::string &rawHeaders );
		bool	_parseBody( void );
		// bool	_parseChunkedBody( void );
		// size_t	_parseChunkedSizeLine( void );
		// bool	_parseChunkedData( void );
		// bool	_parseChunkedCRLF( void );
		// bool	_parseChunkedTrailers( void );

		std::string	_normalizePath( const std::string &rawPath ) const;
		void		_setError( int code, const std::string &message );
		bool		_isHex( char c ) const;
};

// http exception class
class HttpParseError : public std::runtime_error
{
	public:
		int code;

		HttpParseError( int c, const std::string &msg ) : std::runtime_error(msg), code(c) {}
};

#endif
