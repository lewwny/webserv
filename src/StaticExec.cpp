/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticExec.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lenygarcia <lenygarcia@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:11 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/06 13:03:57 by lenygarcia       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/StaticExec.hpp"
#include <sstream>
#include <fstream>

// C++98 compatible toString helper
template<typename T>
static std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Simple MIME type detection based on file extension
static std::string getContentType(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(pos);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".txt") return "text/plain";
    if (ext == ".pdf") return "application/pdf";
    return "application/octet-stream";
}


/**
 * @brief Construire une réponse HTTP 3xx qui indique au client qu’il doit aller ailleurs.
 * @param d La décision de redirection contenant les informations nécessaires.
 * @return Une instance de Response configurée pour la redirection.
 * Créer une Response avec le code de statut et le message de la décision.
*/
Response	StaticExec::makeRedirect(const Router::Decision& d) 
{
	Response 			res;
	std::ostringstream body;

	res.setStatus(d.status, d.reason); // recuperer le code et le message de redirection. ex : 301 Moved Permanently
	res.setHeader("Location", d.redirectURL); // ajouter le header Location avec l'URL de redirection
	body << "<!DOCTYPE html>"
		<< "<html><head><title>"
		<< d.status << " " << d.reason
		<< "</title></head><body>"
		<< "<h1>Redirection</h1>"
		<< "<p>You are being redirected to <a href=\"" << d.redirectURL << "\">" << d.redirectURL << "</a></p>"
		<< "</body></html>";
	res.setHeader("Content-Type", "text/html; charset=utf-8");
	res.setHeader("Content-Length", toString(body.str().size()));
	res.setBody(body.str());
	res.addSecurityHeaders(); // Protège contre certains types d’attaques
	return ( res );
}

Response	StaticExec::makeError(const Router::Decision& d, const Config &cfg) 
{
	Response 			res;
	std::ostringstream	body;
	std::string			errorPage;

	res.setStatus(d.status, d.reason);
	// For now, use server index 0 - this should be passed from the router decision in the future
	errorPage = cfg.getErrorPage(d.status);
	std::ifstream	file(errorPage.c_str(), std::ios::binary); // Ouvre le fichier en mode binaire
	if (file)
	{
		body << file.rdbuf();
		res.setHeader("Content-Type", getContentType(errorPage)); // car la page d'erreur peut etre html, png, etc.
	}
	else
	{
		body << "<!DOCTYPE html>"
			<< "<html><head><title>"
			<< d.status << " " << d.reason
			<< "</title></head><body>"
			<< "<h1>Error " << d.status << ": " << d.reason << "</h1>"
			<< "<p> The requested URL was not found on this server.</p>"
			<< "</body></html>";
		res.setHeader("Content-Type", "text/html; charset=utf-8");
	}
	res.setHeader("Content-Length", toString(body.str().size()));
	res.setBody(body.str());
	res.addSecurityHeaders();
	return ( res );
}

/**
 * @brief Ouvre et lit le fichier.
 * Définit les headers appropriés.
 * @return Error 404 si le fichier n’existe pas.
 *
 */
//  getContentType(const std::string& path) : une fonction qui retourne le type MIME
// (ex : "text/html" pour .html, "image/png" pour .png).
Response	StaticExec::serveStatic(const Router::Decision& d, const Config &cfg)
{
	Response			res;
	std::ifstream		file(d.fsPath.c_str(), std::ios::binary); // Sans ios::in/binary, les fichiers non-texte risquent d’être corrompus

	if (!file)
	{
		std::cerr << "[StaticExec] File not found: " << d.fsPath << std::endl;
		Router::Decision errorDecision;
		errorDecision.status = 404;
		errorDecision.reason = "Not Found";
		errorDecision.redirectURL = "";
		errorDecision.fsPath = "";
		return (makeError(errorDecision, cfg));
	}
	std::ostringstream	body;
	body << file.rdbuf();
	res.setStatus(200, "OK");
	res.setHeader("Content-Type", getContentType(d.fsPath));
	res.setHeader("Content-Length", toString(body.str().size()));
	res.setBody(body.str());
	// std::cout << "[StaticExec] Serving static file: " << body.str() << std::endl;
	res.addSecurityHeaders();
	return ( res );
}
