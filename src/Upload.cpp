/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcauchy- <mcauchy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:04 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/04 12:01:10 by mcauchy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Upload.hpp"
#include "StaticExec.hpp"

static bool isDir( const std::string& path )
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return ( false );
	return ( S_ISDIR(info.st_mode) );
}

static bool invalidFilename( const std::string& name )
{
	// Vérifie si le nom de fichier est invalide (!= de find(".") et "..", car si ex.txt -> valide)
	if (name.empty() || name == "." || name == "..")
		return ( true );
	if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos)
		return ( true );
	std::string invalidChars = "<>:\"|?*"; // Caractères interdits dans les noms de fichiers
	for (size_t i = 0; i < name.size(); ++i)
	{
		if (invalidChars.find(name[i]) != std::string::npos)
			return ( true );
	}
	return ( false );
}

// cree le path complet du fichier en joignant le répertoire et le nom du fichier
static std::string joinPath( const std::string& dir, const std::string& name )
{
	if (dir.empty())
		return ( name );
	if (dir.back() == '/')
		return ( dir + name );
	return ( dir + "/" + name );
}

/*
	utile quand le serveur reçoit plusieurs fichiers et que le serveur ne fournit pas de nom de fichier (ex: d.)
	ex : upload_1693651234567890.bin
*/
static std::string generateUniqueFilename( void )
{
	auto now = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
	std::ostringstream oss;
	oss << "_upload" << now << ".bin";
	return ( oss.str() );
}

// but de cette fonction : sauvegarder le corps de la requête (POST) dans un fichier dans le répertoire spécifié par d.uploadStore
// Le nom du fichier peut être généré de manière unique (par exemple, en utilisant un horodatage ou un UUID).
// Si la sauvegarde réussit, retourner une réponse 201 Created.
// Si une erreur survient (par exemple, le répertoire n'existe pas), retourner une réponse 500 Internal Server Error.
//         std::string uploadStore;      // target dir for uploads
static Response save(const Router::Decision& d, const Request& req)
{
	Response			res;
	std::string		uploadDir = d.uploadStore; // Contient le répertoire de stockage des uploads

	if (uploadDir.empty() || !isDir(uploadDir))
	{
		res.setStatus(403, "Forbidden");
		const std::string body = " Upload directory is not configured or does not exist.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		return ( res );
	}
	if (req.getBody().empty())
	{
		res.setStatus(400, "Bad Request");
		const std::string body = " Request body is empty.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		return ( res );
	}
	std::string filename;
	if ( d.filename.empty() )
		filename = generateUniqueFilename();
	else
		filename = d.filename;
	if (invalidFilename(filename))
	{
		res.setStatus(500, "Internal Server Error");
		const std::string body = " Generated filename is invalid.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		return ( res );
	}
}