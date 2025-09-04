/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/02 15:29:43 by macauchy          #+#    #+#             */
/*   Updated: 2025/09/02 17:24:24 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Router.hpp"

// No CTOR/DTOR needed, as Router is a static utility class.

const std::map<std::string, std::string> Router::selectServerByHost( const Request &req, const ConfigParse &cfg )
{
	if (!req.hasHeader("Host"))
	{
		return (cfg.getServerConfig(0)); // Return first server block if no Host header
	}

	// Extract hostname from Host header (ignore port if present)
	std::string	hostName = req.getHeader("Host");
	size_t		colonPos = hostName.find(':');
	if (colonPos != std::string::npos)
		hostName = hostName.substr(0, colonPos);
	// Search for server block with matching server_name
	int	serverCount = cfg.getServerCount();
	for (int i = 0; i < serverCount; ++i)
	{
		if (cfg.getServerConfig(i).find("server_name") != cfg.getServerConfig(i).end())
		{
			if (cfg.getServerConfig(i).at("server_name") == hostName)
				return (cfg.getServerConfig(i));
		}
	}
	
}

const std::map<std::string, std::string> Router::longestPrefix( const std::string &uri, const std::vector<std::map<std::string, std::string> > loc, std::string &mountOut, std::string &relOut )
{
	if (loc.empty())
		return (std::map<std::string, std::string>());
	
	const std::map<std::string, std::string>	*bestMatch = NULL;
	size_t										bestLen = 0;

	// Clear output variables
	mountOut.clear();
	relOut.clear();

	// Find the longest matching prefix
	std::vector<std::string> paths;
	for (size_t i = 0; i < loc.size(); ++i)
	{
		if (loc[i].find("path") != loc[i].end())
			paths.push_back(loc[i].at("path"));
	}
	
}

Router::Decision	Router::decide( const Request &req, const ConfigParse &cfg )
{
	Decision	decision;

	if (req.getError())
	{
		// If the request has a parsing error, return an error decision immediately.
		decision.type = ACTION_ERROR;
		decision.status = req.getErrorCode();
		decision.reason = req.getErrorMessage();
		return (decision);
	}

	std::map<std::string, std::string> server = selectServerByHost(req, cfg);
	decision.server = &server;
	if (!decision.server)
	{
		decision.type = ACTION_ERROR;
		decision.status = 500;
		decision.reason = "No server configured";
		return (decision);
	}
}
