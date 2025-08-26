/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:45:22 by lengarci          #+#    #+#             */
/*   Updated: 2025/08/26 11:24:10 by lengarci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "include/Server.hpp"
#include "include/ConfigParse.hpp"

int main(int argc, char **argv) {
	(void)argv;
	if (argc != 1 && argc != 2) {
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}
	try {
		ConfigParse parser(argc == 2 ? argv[1] : "conf/default.conf");
		parser.loadFile();
		parser.tokenize();
		parser.printTokens();
	}
	catch (const std::exception &e) {
		std::cerr << "Config Error: " << e.what() << std::endl;
		return 1;
	}
	// try {
	// 	Server server;
	// 	server.init("0.0.0.0", 8080);
	// 	server.run();
	// }
	// catch (const std::exception &e) {
	// 	std::cerr << "Error: " << e.what() << std::endl;
	// }
	return 0;
}
