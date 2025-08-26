/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:45:22 by lengarci          #+#    #+#             */
/*   Updated: 2025/08/26 15:05:10 by lengarci         ###   ########.fr       */
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
		parser.parse();
		// parser.printTokens();
		parser.printConfig();
		Server server;
		server.init("0.0.0.0", std::atoi(parser.getValue("listen").c_str()));
		server.run();
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
