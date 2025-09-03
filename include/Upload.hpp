/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mcauchy- <mcauchy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 14:28:18 by mcauchy-          #+#    #+#             */
/*   Updated: 2025/09/03 14:28:19 by mcauchy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UPLOAD_HPP

#define UPLOAD_HPP

#include "Router.hpp"
#include "Response.hpp"

class Upload
{
	public :
		// Produce an upload response (blocking I/O):
		static Response save(const Router::Decision& d, const Request& req);
};

#endif
