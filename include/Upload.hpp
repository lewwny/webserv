/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcauchy- <mcauchy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:18 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/05 09:35:31 by mcauchy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UPLOAD_HPP

#define UPLOAD_HPP

#include "Router.hpp"
#include "Response.hpp"
#include <sys/stat.h>
#include <chrono>

class Upload : public StaticExec
{
	public :
		// Produce an upload response (blocking I/O):
		static Response save(const Router::Decision& d, const Request& req);
};
	
	static bool			isDir( const std::string& path );
	static bool			getUploadDir( const std::string& dir ); // crée le répertoire s'il n'existe pas
	static bool			invalidFilename( const std::string& name );
	static std::string	joinPath( const std::string& dir, const std::string& file );
	static std::string	generateUniqueFilename( void );

#endif
