/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:18 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/05 19:35:24 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UPLOAD_HPP

#define UPLOAD_HPP

#include "Router.hpp"
#include "Response.hpp"
#include <sys/stat.h>
#include <ctime>
#include <cstdlib>

class Upload
{
	public :
		// Produce an upload response (blocking I/O):
		static Response save(const Router::Decision& d, const Request& req);
		
	private:
		static bool			isDir( const std::string& path );
		static bool			getUploadDir( const std::string& dir ); // crée le répertoire s'il n'existe pas
		static bool			invalidFilename( const std::string& name );
		static std::string	joinPath( const std::string& dir, const std::string& file );
		static std::string	generateUniqueFilename( void );
};

#endif
