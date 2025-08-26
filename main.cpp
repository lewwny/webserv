/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macauchy <macauchy@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:45:22 by lengarci          #+#    #+#             */
/*   Updated: 2025/08/25 17:24:32 by macauchy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "include/Server.hpp"

int main() {
	std::cout << "Hello, Zehma Webserv!" << std::endl;
	try {
		Server server;
		server.init("127.0.0.1", 8080);
		server.run();
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
