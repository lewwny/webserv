#include "StaticExec.hpp"


/**
 * @brief Construire une réponse HTTP 3xx qui indique au client qu’il doit aller ailleurs.
 * @param d La décision de redirection contenant les informations nécessaires.
 * @return Une instance de Response configurée pour la redirection.
 * Créer une Response

Avec le bon code et message
Ajouter le header Location
Construire un petit body HTML

Pas strictement nécessaire, mais recommandé pour navigateurs.

Ajouter les headers de sécurité
Empêche le navigateur d’interpréter le contenu comme un autre type que celui déclaré.
Empêche l’affichage du site dans une iframe (protection contre le clickjacking).
Active la protection XSS du navigateur.
 */
Response	StaticExec::makeRedirect(const Router::Decision& d) 
{
	Response 			res;
	std::ostringstream body;

	res.setStatus(d.status, d.reason); // recuperer le code et le message de redirection. ex : 301 Moved Permanently
	res.setHeader("Location", d.redirectURL); // ajouter le header Location avec l'URL de redirection
	std::ifstream file(d.fsPath.c_str());
	if (file) {
		body << "<html><head><title>"
			<< d.status << " " << d.reason
			<< "</title></head><body>"
			<< "<h1>Redirection</h1>"
			<< "<p>You are being redirected to <a href=\"" << d.redirectURL << "\">" << d.redirectURL << "</a></p>"
			<< "</body></html>";
	}
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
	std::ifstream	file(errorPage.c_str());
	if (file)
		body << file.rdbuf();
	else
	{
		body << "<html><head><title>"
			<< d.status << " " << d.reason
			<< "</title></head><body>"
			<< "<h1>Redirection</h1>"
			<< "<p>Error Page <a href=\"" << d.redirectURL << "\">" << d.redirectURL << "</a></p>"
			<< "</body></html>";
	}

/* 
	res.setHeader("Content-Type", "text/html; charset=UTF-8");
	res.setHeader("Content-Length", std::to_string(body.str().size()));
	
	À quoi ça sert ?

Content-Type indique au client que le corps de la réponse est du HTML encodé en UTF-8.
Content-Length précise la taille du corps (en octets), ce qui permet au client de savoir quand la réponse est terminée.
Pourquoi dans makeError ?

Les pages d’erreur sont toujours envoyées avec un corps HTML : il faut donc indiquer le type et la taille.
Pour une redirection (makeRedirect), le corps HTML est souvent facultatif : le navigateur se base surtout sur le header Location pour agir. Mais si tu ajoutes un body HTML dans la redirection (pour compatibilité navigateur), il est aussi recommandé d’ajouter ces headers.
En résumé :
Ces headers sont nécessaires pour que le client interprète correctement la réponse et sachent sa taille.
Ils sont obligatoires pour les pages d’erreur, et recommandés pour les redirections si tu fournis un body HTML.
*/
	res.
	res.setBody(body.str());
	res.addSecurityHeaders();
}