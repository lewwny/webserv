#include "StaticExec.hpp"


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
	body << "<html><head><title>"
		<< d.status << " " << d.reason
		<< "</title></head><body>"
		<< "<h1>Redirection</h1>"
		<< "<p>You are being redirected to <a href=\"" << d.redirectURL << "\">" << d.redirectURL << "</a></p>"
		<< "</body></html>";
	res.setHeader("Content-Type", "text/html; charset=utf-8");
	res.setHeader("Content-Length", std::to_string(body.str().size()));
	res.setBody(body.str());
	res.addSecurityHeaders(); // Protège contre certains types d’attaques
	return ( res );
}

Response	StaticExec::makeError(const Router::Decision& d, const ConfigParse &cfg) 
{
	Response 			res;
	std::ostringstream	body;
	std::string			errorPage;

	res.setStatus(d.status, d.reason);
	errorPage = cfg.getErrorPagePath(d.status);
	std::ifstream	file(errorPage.c_str(), std::ios::binary); // Ouvre le fichier en mode binaire
	if (file)
	{
		body << file.rdbuf();
		res.setHeader("Content-Type", getContentType(errorPage)); // car la page d'erreur peut etre html, png, etc.
	}
	else
	{
		body << "<html><head><title>"
			<< d.status << " " << d.reason
			<< "</title></head><body>"
			<< "<h1>Error " << d.status << ": " << d.reason << "</h1>"
			<< "<p> The requested URL was not found on this server.</p>"
			<< "</body></html>";
		res.setHeader("Content-Type", "text/html; charset=utf-8");
	}
	res.setHeader("Content-Length", std::to_string(body.str().size()));
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
Response	StaticExec::serveStatic(const Router::Decision& d, const ConfigParse &cfg)
{
	Response			res;
	std::ifstream		file(d.fsPath.c_str(), std::ios::binary); // Sans ios::in/binary, les fichiers non-texte risquent d’être corrompus

	if (!file)
	{
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
	res.setHeader("Content-Length", std::to_string(body.str().size()));
	res.setBody(body.str());
	res.addSecurityHeaders();
	return ( res );
}