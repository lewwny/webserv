/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 15:29:43 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/05 15:58:49 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Router.hpp"
#include "../include/Server.hpp"
#include "../include/ServerManager.hpp"

// No CTOR/DTOR needed, as Router is a static utility class.

bool	Router::isMethodAllowed( const std::string &method, const Location *loc, std::string &allowHeader )
{
	std::vector<std::string>	allowed;

	if (loc && !loc->getMethods().empty())
		allowed = loc->getMethods();
	else
	{
		// Default allowed methods if none specified
		allowed.push_back("GET");
		allowed.push_back("POST");
		allowed.push_back("DELETE");
	}

	// Build Allow header
	std::ostringstream	oss;
	for (size_t i = 0; i < allowed.size(); ++i)
	{
		if (i > 0)
			oss << ", ";
		oss << allowed[i];
	}
	allowHeader = oss.str();

	// Check if method is in allowed list
	for (size_t i = 0; i < allowed.size(); ++i)
	{
		if (allowed[i] == method)
			return (true);
	}
	return (false);
}

bool	Router::normalizePath( const std::string &in, std::string &out )
{
	std::vector<std::string>	parts;	// to hold path segments
	std::istringstream			iss(in);
	std::string					token;

	// Split path by '/'
	while (std::getline(iss, token, '/'))
	{
		if (token.empty() || token == ".")
		{
			// skip empty or current dir
			continue ;
		}
		else if (token == "..")
		{
			// Backtrack one segment if possible
			if (!parts.empty())
				parts.pop_back();
			else
				return (false); // Attempt to go above root
		}
		else
		{
			parts.push_back(token);
		}
	}
	
	// Reconstruct normalized path
	out = "/";
	for (size_t i = 0; i < parts.size(); ++i)
	{
		out += parts[i];
		if (i + 1 < parts.size())
			out += "/";
	}
	return (true);
}

Router::Decision	Router::decide( const Request &req, const Config &cfg, const ServerManager &sm )
{
	Decision	d;

	d.server = &selectServer(req, sm)->getConfig();

	// Match location
	const Location	*loc = d.server->getLocationByPath(req.getUri());
	if (!loc)
	{
		d.type = ACTION_ERROR;
		d.status = 404;
		d.reason = "Not Found";
		return (d);
	}
	d.location = loc;
	d.mountUri = loc->getPath();
	
	// Relative path within location
	std::string	mount, rel;
	d.location = matchRoute(req.getUri(), selectServer(req, sm), mount, rel);
	d.mountUri = mount;
	d.relPath = rel;

	// Check method
	std::string	allowHeader;
	if (!isMethodAllowed(req.getMethod(), loc, allowHeader))
	{
		d.type = ACTION_ERROR;
		d.status = 405;
		d.reason = "Method Not Allowed";
		return (d);
	}

	// Body size check in location then server
	long	maxBody = loc->getClientMaxBodySize() > 0 
					? loc->getClientMaxBodySize() 
					: d.server->getClientMaxBodySize();
	if (req.getBody().size() > static_cast<size_t>(maxBody))
	{
		d.type = ACTION_ERROR;
		d.status = 413;
		d.reason = "Payload Too Large";
		return (d);
	}

	// Determine filesystem path
	std::string	path;
	if (normalizePath(cfg.getRoot() + req.getUri(), path))
	{
		d.fsPath = path;
	}
	else
	{
		d.type = ACTION_ERROR;
		d.status = 400;
		d.reason = "Bad Path";
		return (d);
	}

	// CGI check

	std::string	ext, interp;
	if (hasCgi(d.location, d.relPath, ext, interp))
	{
		d.type = ACTION_CGI;
		d.cgiExt = ext;
		d.cgiInterpreter = interp;
		return (d);
	}

	// Autoindex or static
	struct stat st; // to check file/dir
	if (::stat(d.fsPath.c_str(), &st) == 0)
	{
		// Path exists
		if (S_ISDIR(st.st_mode))
		{
			// Directory
			if (!loc->getIndex().empty())
			{
				d.fsPath += "/" + loc->getIndex();
				d.type = ACTION_STATIC;
			}
			else if (loc->isAutoindex())
			{
				d.type = ACTION_AUTOINDEX;
				d.autoindex = true;
			}
			else
			{
				d.type = ACTION_ERROR;
				d.status = 403;
				d.reason = "Forbidden";
			}
		}
		else
			d.type = ACTION_STATIC; // Regular file
	}
	else
	{
		// Path does not exist
		d.type = ACTION_ERROR;
		d.status = 404;
		d.reason = "Not Found";
	}

	// Upload
	if (!loc->getUploadStore().empty() && req.getMethod() == "POST")
	{
		d.type = ACTION_UPLOAD;
		d.uploadEnabled = true;
		d.uploadStore = loc->getUploadStore();
	}

	return (d);
}

const Server	*Router::selectServer( const Request &req, const ServerManager &sm )
{
	const std::vector<Server *>	&servers = sm.getServers();

	if (servers.empty())
		return (0);

	// Match by Host header
	std::string	hostHeader = req.getHeader("Host");
	std::string	hostOnly = hostHeader;
	std::string::size_type colon = hostHeader.find(':');
	if (colon != std::string::npos)
		hostOnly = hostHeader.substr(0, colon);

	// First try exact match of host:port
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const std::vector<std::string> &names = servers[i]->getConfig().getServerNames();
		for (size_t j = 0; j < names.size(); ++j)
		{
			if (names[j] == hostOnly) // Exact match
				return (servers[i]);
		}
	}

	// Fallback to first server block listening on the same port
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (servers[i]->getPort() == req.getPort())
			return (servers[i]);
	}

	// Fallback to first server block
	return (servers[0]);
}

const Location	*Router::matchRoute( const std::string &uri, const Server *sb,
									std::string &mountUriOut, std::string &relPathOut )
{
	const std::vector<Location>	&locs = sb->getConfig().getLocations();
	const Location				*bestMatch = 0;
	size_t						bestLen = 0;

	// Find longest matching location prefix
	for (size_t i = 0; i < locs.size(); ++i)
	{
		const std::string	&locPath = locs[i].getPath();
		if (uri.compare(0, locPath.size(), locPath) == 0)
		{
			// Prefix matches location path
			if (locPath.size() > bestLen)
			{
				// Longer match found
				bestLen = locPath.size();
				bestMatch = &locs[i];
			}
		}
	}

	// Set outputs
	if (bestMatch)
	{
		mountUriOut = bestMatch->getPath();
		relPathOut = uri.substr(mountUriOut.size());
		return (bestMatch);
	}
	// No match, use root
	mountUriOut = "/";
	relPathOut = uri;
	return (0);
}

bool	Router::hasCgi( const Location *loc, const std::string &relPath,
					std::string &matchedExt, std::string &interp )
{
	if (!loc)
		return (false);

	// Example: Location configured for ".py" with "/usr/bin/python"
	const std::vector<std::string>	&methods = loc->getMethods();
	if (std::find(methods.begin(), methods.end(), "POST") == methods.end())
		return (false); // CGI usually requires POST
	
	std::string::size_type dot = relPath.rfind('.');
	if (dot == std::string::npos)
		return (false); // No extension
	
	std::string	ext = relPath.substr(dot); // includes the dot

	// Check if this extension is configured for CGI
	if (!loc->getCgiExtension().empty() && ext == loc->getCgiExtension())
	{
		matchedExt = ext;
		interp = loc->getCgiPath();
		return (true);
	}
	return (false);
}
