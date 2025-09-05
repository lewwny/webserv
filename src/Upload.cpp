/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcauchy- <mcauchy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:04 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/05 12:12:50 by mcauchy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Upload.hpp"
#include "StaticExec.hpp"

static bool isDir( const std::string& path ) // vérifie si le path est un répertoire, but de stat : vérifier le type de fichier
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return ( false );
	return ( S_ISDIR(info.st_mode) );
}

static bool getUploadDir( const std::string& dir ) // crée le répertoire s'il n'existe pas
{
	if (dir.empty())
		return ( false );
	if (isDir(dir))
		return ( true );
	// Crée le répertoire avec les permissions rwxr-xr-x (755)
	if (mkdir(dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
		return ( false );
	return ( true );
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

	if (uploadDir.empty() || !isDir(uploadDir)) // condition si le répertoire n'existe pas ou n'est pas configuré
	{
		res.setStatus(403, "Forbidden");
		const std::string body = " Upload directory is not configured or does not exist.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.addSecurityHeaders();
		return ( res );
	}
	if (!getUploadDir(uploadDir)) // condition pour créer le répertoire s'il n'existe pas
	{
		res.setStatus(500, "Internal Server Error");
		const std::string body = " Failed to create upload directory.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8"); // pas sur si utile
		res.addSecurityHeaders();
		return ( res );
	}
	const std::string &data = req.getBody(); // recupere le contenu de la requête POST ( Si un client envoie une image, data contiendra tous les bytes de cette image
	if (data.empty())
	{
		res.setStatus(400, "Bad Request");
		const std::string body = " Request body is empty.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.addSecurityHeaders();
		return ( res );
	}
	std::string filename = generateUniqueFilename(); // génère un nom de fichier unique
	std::string filePath = joinPath(uploadDir, filename); // crée le chemin complet du fichier en joignant le répertoire et le nom du fichier
	if (invalidFilename(filename)) // vérifie si le nom de fichier est invalide
	{
		res.setStatus(500, "Internal Server Error");
		const std::string body = " Generated filename is invalid.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.addSecurityHeaders();
		return ( res );
	}
	std::ofstream ofs(filePath.c_str(), std::ios::binary); // Ouvre le fichier en mode binaire (important pour les fichiers non-texte : PDF , images, etc.)
	if (!ofs)
	{
		res.setStatus(500, "Internal Server Error");
		const std::string body = " Failed to open file for writing.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.addSecurityHeaders();
		return ( res );
	}
	// Recevoir un corps de requête (POST) et l’enregistrer tel quel sur disque.
	ofs.write(data.data(), static_cast<std::streamsize>(data.size())); // Écrit toutes les donnees brutes, même les null bytes (\0) + convertit en type attendu par write
	if (ofs.fail())
	{
		res.setStatus(500, "Internal Server Error");
		const std::string body = " Failed to write data to file.\n";
		res.setBody(body);
		res.setHeader("Content-Length", std::to_string(body.size()));
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.addSecurityHeaders();
		return ( res );
	}
	ofs.close();
	res.setStatus(201, "Created");
	std::ostringstream body;
	body << "<!DOCTYPE html>"
		<< "<html><head><title>"
		<< "201 Created"
		<< "</title></head><body>"
		<< "<h1>File Uploaded Successfully</h1>"
		<< "<p>File has been uploaded to: " << filePath << "</p>"
		<< "</body></html>";

	res.setBody(body.str());
	res.setHeader("Content-Length", std::to_string(body.str().size()));
	res.setHeader("Content-Type", "text/html; charset=utf-8");
	res.addSecurityHeaders();
	return ( res );
}