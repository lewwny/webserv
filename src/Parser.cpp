/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:15:59 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/05 16:29:13 by lengarci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Parser.hpp"
#include <cstdlib>
#include <cerrno>

Parser::Parser( void ) : _state(REQ_LINE), _chunkState(CHUNK_SIZE)
{
	_chunked = false;
	_expectedBodyLength = 0;
	_chunkBytesRemaining = 0;
	_limitHeaderBytes = 8 * 1024; // 8 KB
	_limitBodyBytes = 1 * 1024 * 1024; // 1 MB
	_limitLineBytes = 4 * 1024; // 4 KB
}

Parser::Parser( const Parser& other )
{
	*this = other;
}

Parser	&Parser::operator=( const Parser &other )
{
	if (this != &other)
	{
		_state = other._state;
		_buffer = other._buffer;
		_current = other._current;
		_expectedBodyLength = other._expectedBodyLength;
		_chunked = other._chunked;
	}
	return *this;
}

Parser::~Parser( void )
{
}

bool	Parser::isComplete( void ) const
{
	return (_state == COMPLETE);
}

Request	Parser::getRequest( void ) const
{
	return (_current);
}

void	Parser::reset( void )
{
	_state = REQ_LINE;
	_chunkState = CHUNK_SIZE;
	_current.reset();
	_expectedBodyLength = 0;
	_chunkBytesRemaining = 0;
}

void	Parser::setLimits( size_t headerBytes, size_t bodyBytes, size_t lineBytes )
{
	_limitHeaderBytes = headerBytes;
	_limitBodyBytes = bodyBytes;
	_limitLineBytes = lineBytes;
}

void	Parser::_setError( int code, const std::string &message )
{
	_current.setError();
	_current.setErrorCode(code);
	_current.setErrorMessage(message);
	_current.markComplete();
	_state = COMPLETE; // Stop further parsing
}

bool	Parser::feed( const std::string &data )
{
	_buffer += data;

	try
	{
		// Request Line
		if (_state == REQ_LINE)
		{
			// Protect against overly long lines
			if (_buffer.size() > _limitLineBytes)
				throw HttpParseError(400, "Request line too long");

			size_t pos = _buffer.find("\r\n");
			if (pos == std::string::npos)
				return false;

			_parseRequestLine(_buffer.substr(0, pos));
			// Erase the request line including the terminating CRLF so headers start cleanly
			_buffer.erase(0, pos + 2);
			_state = HEADERS;
		}

		// Headers
		if (_state == HEADERS)
		{
			// Protect against overly large headers (slowloris attack)
			if (_buffer.size() > _limitHeaderBytes)
				throw HttpParseError(431, "Headers too large");

			size_t	end = _buffer.find("\r\n\r\n");
			if (end == std::string::npos)
			{
				// Special-case: empty header block where buffer starts with CRLF (i.e., "\r\n\r\n" was split so after removing request line we may have "\r\n")
				if (_buffer.size() >= 2 && _buffer.substr(0, 2) == "\r\n")
					end = 0; // empty headers
				else
					return false;
			}

			_parseHeaderBlock(_buffer.substr(0, end));
			// If end == 0 we need to erase the terminating sequence length (4) to consume the CRLFCRLF
			_buffer.erase(0, end + 4);
			
			// Check for body indicators
			if (_chunked || _expectedBodyLength > 0)
			{
				_state = BODY;
				if (_chunked)
				{
					_chunkState = CHUNK_SIZE;
					_chunkBytesRemaining = 0;
				}
			}
			else
			{
				// No body, request is complete
				_current.markComplete();
				_state = COMPLETE;
				return true;
			}
		}

		// Body
		if (_state == BODY)
		{
			if (_parseBody())
			{
				_state = COMPLETE;
				_current.markComplete();
				return true;
			}
		}
	}
	catch (const HttpParseError &e)
	{
		_setError(e.code, e.what());
		_state = ERROR;
		return false;
	}
	catch (const std::exception &e)
	{
		_setError(400, "Bad Request");
		_state = ERROR;
		return false;
	}
	return false;
}

void	Parser::_parseRequestLine( const std::string &line )
{
	// Format: METHOD SP URI SP VERSION
	// Example: GET /index.html HTTP/1.1

	// enforce line length limit
	if (line.length() > _limitLineBytes)
		throw HttpParseError(400, "Request line too long");

	std::istringstream	ss(line);
	std::string method, uri, version;
	ss >> method >> uri >> version;
	_current.setMethod(method);
	_current.setUri(uri);
	_current.setVersion(version);

	// Basic validation
	if (method.empty() || uri.empty() || version.empty())
		throw HttpParseError(400, "Malformed request line");

	// Allowed methods
	if (method != "GET" && method != "POST" && method != "DELETE")
		throw HttpParseError(405, "Method Not Allowed");

	// Validate HTTP version
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		throw HttpParseError(505, "HTTP Version Not Supported");

	// Normalize path and extract query
	size_t	qpos = _current.getUri().find('?');
	if (qpos != std::string::npos)
	{
		// Query string present (e.g., /index.html?user=42)
		_current.setPath(_normalizePath(_current.getUri().substr(0, qpos)));
		_current.setQuery(_current.getUri().substr(qpos + 1));
	}
	else
	{
		// No query string present
		_current.setPath(_normalizePath(_current.getUri()));
		_current.setQuery("");
	}

	// Reject weird characters in path
	for (size_t i = 0; i < _current.getPath().length(); i++)
	{
		unsigned char c = static_cast<unsigned char>(_current.getPath()[i]);
		if (c < 32 && c != '\t') // Allow tab
			throw HttpParseError(400, "Invalid character in path");
	}
}

void	Parser::_parseHeaderBlock( const std::string &rawHeaders )
{
	std::istringstream	ss(rawHeaders);
	std::string 		line;

	bool	sawContentLength = false; 	// track if Content-Length was seen and
	bool	sawTransferEncoding = false;// track if Transfer-Encoding was seen to prevent http smuggling

	while (std::getline(ss, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1); // Remove trailing \r (C++98 safe)
		if (line.empty())
			break ; // Skip empty lines (shouldn't happen here)

		// Check line length
		if (line.size() > _limitLineBytes)
			throw HttpParseError(400, "Header line too long");

		// Split at first colon
		size_t	colon = line.find(':');
		if (colon == std::string::npos)
			throw HttpParseError(400, "Malformed header line");

		std::string	key, value;
		key = line.substr(0, colon);
		value = line.substr(colon + 1);

		// Trim value (i.g., "  value  " -> "value")
		size_t start = value.find_first_not_of(" \t");
		size_t end = value.find_last_not_of(" \t");
		
		// If value is all spaces/tabs, set to empty string
		if (start == std::string::npos)
			value.clear();
		else
			value = value.substr(start, end - start + 1); // Trim spaces/tabs

		_current.setHeader(key, value);

		// Special handling for Content-Length and Transfer-Encoding (Bookkeeping)
		std::string	k = key;
		std::transform(k.begin(), k.end(), k.begin(), ::tolower);
		if (k == "content-length")
		{
			if (sawContentLength)
			{
				_setError(400, "Multiple Content-Length headers");
				return;
			}
			sawContentLength = true;
			
			// Parse Content-Length value
			const std::string	&v = _current.getHeader("content-length");
			if (v.empty())
			{
				_setError(400, "Empty Content-Length header");
				return;
			}

			// Ensure Content-Length is numeric
			for (size_t i = 0; i < v.length(); ++i)
			{
				if (!std::isdigit(static_cast<unsigned char>(v[i])))
				{
					_setError(400, "Non-numeric Content-Length");
					return;
				}
			}

			// Convert to long long using strtoll (C++98 safe) with errno checks
			errno = 0;
			const char *cstr = v.c_str();
			char *endptr = NULL;
			long long tmp = std::strtoll(cstr, &endptr, 10);
			if (errno == ERANGE)
			{
				_setError(400, "Content-Length out of range");
				return;
			}
			if (endptr == cstr || *endptr != '\0')
			{
				_setError(400, "Invalid Content-Length");
				return;
			}
			if (tmp < 0 || static_cast<size_t>(tmp) > _limitBodyBytes)
			{
				_setError(413, "Content-Length too large");
				return;
			}

			_expectedBodyLength = static_cast<size_t>(tmp); // Safe cast after checks
			if (_expectedBodyLength > _limitBodyBytes)
			{
				_setError(413, "Content-Length exceeds limit");
				return;
			}
		}
		else if (k == "transfer-encoding")
		{
			if (sawTransferEncoding)
			{
				_setError(400, "Multiple Transfer-Encoding headers");
				return;
			}
			sawTransferEncoding = true;
			if (_current.getHeader("transfer-encoding") != "chunked")
			{
				_setError(501, "Unsupported Transfer-Encoding");
				return;
			}
			_chunked = true;
		}
		else if (k == "cookie")
		{
			_current.setHeader("cookie", value);
		}

	}
	
	// HTTP/1.1 requires Host header
	if (_current.getVersion() == "HTTP/1.1" && !_current.hasHeader("host"))
		_setError(400, "Missing Host header");

	// Prevent HTTP request smuggling
	if (sawContentLength && sawTransferEncoding)
		_setError(400, "Both Content-Length and Transfer-Encoding headers present");
}

bool	Parser::_parseBody( void )
{
	if (_chunked)
		return (true); // TODO: implement chunked body parsing
	else
	{
		// Content-Length based body
		if (_buffer.size() < _expectedBodyLength)
			return (false); // Need more data
		
		// Append body data
		_current.appendToBody(_buffer.substr(0, _expectedBodyLength));
		_buffer.erase(0, _expectedBodyLength);
		return (true); // Body complete
	}
}

std::string	Parser::_normalizePath( const std::string &rawPath ) const
{
	// Always treat as absolute path for routing
	std::vector<std::string>	parts;	// to hold path segments
	std::string			seg;    // current segment

	// Fast path: empty -> root
	if (rawPath.empty())
		return std::string("/");

	// Split path by '/' — iterate one past the end to capture trailing segment
	size_t len = rawPath.size();
	size_t j = 0; // segment start
	for (size_t i = 0; i <= len; ++i)
	{
		if (i == len || rawPath[i] == '/')
		{
			if (i > j) // non-empty segment
			{
				seg = rawPath.substr(j, i - j);
				if (seg == ".")
				{
					// skip current dir
				}
				else if (seg == "..")
				{
					if (!parts.empty())
						parts.erase(parts.end() - 1);
					// if parts empty, stay at root (do not add empty)
				}
				else
				{
					parts.push_back(seg);
				}
			}
			// move to next segment start (skip the '/')
			j = i + 1;
		}
	}

	// Reconstruct normalized path
	std::string	normalized = "/";
	for (size_t i = 0; i < parts.size(); ++i)
	{
		normalized += parts[i];
		if (i + 1 < parts.size())
			normalized += "/";
	}
	return normalized;
}

bool	Parser::_isHex( char c ) const
{
	return ( (c >= '0' && c <= '9') ||
			 (c >= 'a' && c <= 'f') ||
			 (c >= 'A' && c <= 'F') );
}

// ----------- TODO: Implement chunked body parsing methods ------------
