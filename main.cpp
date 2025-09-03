/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lengarci <lengarci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 11:45:22 by lengarci          #+#    #+#             */
/*   Updated: 2025/09/03 15:51:47 by lengarci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "include/Server.hpp"
#include "include/ConfigParse.hpp"
#include "include/Parser.hpp"
#include "include/Request.hpp"
#include "include/Response.hpp"
#include "include/Config.hpp"
#include "include/ServerManager.hpp"


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
		// parser.printConfig();
		ServerManager serverManager(parser);
		// for (int i = 0; i < parser.getServerCount(); ++i) {
		// 	std::cout << "=== Server " << i + 1 << " Configuration ===" << std::endl;
		// 	Config config(parser, i);
		// 	config.printConfig();
		// 	std::cout << "==============================" << std::endl;
		// }
		ServerManager::setInstance(&serverManager);
		signal(SIGINT, &ServerManager::handleSignal);
		serverManager.run();
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
