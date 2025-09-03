/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcauchy- <mcauchy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:04 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/03 15:00:16 by mcauchy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Upload.hpp"

static bool isDir( const std::string& path )
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}

// but de cette fonction : sauvegarder le corps de la requête (POST) dans un fichier dans le répertoire spécifié par d.uploadStore
// Le nom du fichier peut être généré de manière unique (par exemple, en utilisant un horodatage ou un UUID).
// Si la sauvegarde réussit, retourner une réponse 201 Created.
// Si une erreur survient (par exemple, le répertoire n'existe pas), retourner une réponse 500 Internal Server Error.
//         std::string uploadStore;      // target dir for uploads
static Response save(const Router::Decision& d, const Request& req)
{
	
}